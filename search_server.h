#pragma once
#include <string>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "document.h"
#include "string_processing.h"
 
const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double MAX_DIFFERENCE = 1e-6;
 
enum class DocumentStatus {
	ACTUAL, IRRELEVANT, BANNED, REMOVED,
};
 
class SearchServer {
public:
	inline static constexpr int INVALID_DOCUMENT_ID = -1;
 
	template<typename StringCollection>
	explicit SearchServer(const StringCollection &stop_words) {
		for (const std::string &item : stop_words) {
			if (!IsValidWord(item))
				throw std::invalid_argument("Wrong symbol in a stop word");
			if (item != "")
				stop_words_.insert(item);
		}
	}
 
	explicit SearchServer(const std::string &stop_words_text) :
			SearchServer(SplitIntoWords(stop_words_text)) {
	}
 
	void AddDocument(int document_id, const std::string &document,
			DocumentStatus status, const std::vector<int> &ratings);
 
	template<typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const std::string &raw_query,
			DocumentPredicate document_predicate) const {
		const auto query = ParseQuery(raw_query);
 
		auto matched_documents = FindAllDocuments(query, document_predicate);
 
		sort(matched_documents.begin(), matched_documents.end(),
				[](const Document &lhs, const Document &rhs) {
					if (std::abs(lhs.relevance - rhs.relevance) < MAX_DIFFERENCE) {
						return lhs.rating > rhs.rating;
					} else {
						return lhs.relevance > rhs.relevance;
					}
				});
		if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
		}
 
		return matched_documents;
	}
 
	std::vector<Document> FindTopDocuments(const std::string &raw_query,
			DocumentStatus status) const {
		return FindTopDocuments(raw_query,
				[status](int document_id, DocumentStatus document_status,
						int rating) {
					return document_status == status;
				});
	}
 
	std::vector<Document> FindTopDocuments(const std::string &raw_query) const {
		return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
	}
 
	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(
			const std::string &raw_query, int document_id) const;
 
	int GetDocumentCount() const;
 
	int GetDocumentId(int index) const;
    
    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;
    
    void RemoveDocument(int document_id);
    
    auto begin() {
        return document_ids_.begin();
    }
    
    auto end() {
        return document_ids_.end();
    }
 
private:
	std::set<std::string> stop_words_;
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	struct DocumentData {
		int rating;
		DocumentStatus status;
	};
	std::map<int, DocumentData> documents_;
	std::vector<int> document_ids_;
    std::map<int, std::map<std::string, double>> ids_to_words_;
 
	bool IsStopWord(const std::string &word) const;
 
	void SetStopWords(const std::string &text);
 
	std::vector<std::string> SplitIntoWordsNoStop(
			const std::string &text) const;
 
	static int ComputeAverageRating(const std::vector<int> &ratings);
 
	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};
 
	QueryWord ParseQueryWord(std::string text) const;
 
	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};
 
	Query ParseQuery(const std::string &text) const;
 
	double ComputeWordInverseDocumentFreq(const std::string &word) const;
 
	template<typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const Query &query,
			DocumentPredicate document_predicate) const {
		std::map<int, double> document_to_relevance;
		for (const std::string &word : query.plus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			const double inverse_document_freq = ComputeWordInverseDocumentFreq(
					word);
			for (const auto [document_id, term_freq] : word_to_document_freqs_.at(
					word)) {
				const auto &document_data = documents_.at(document_id);
				if (document_predicate(document_id, document_data.status,
						document_data.rating)) {
					document_to_relevance[document_id] += term_freq
							* inverse_document_freq;
				}
			}
		}
 
		for (const std::string &word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}
 
		std::vector<Document> matched_documents;
		for (const auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back( { document_id, relevance,
					documents_.at(document_id).rating });
		}
		return matched_documents;
	}
 
	static bool IsValidWord(const std::string &word);
 
};
 
void PrintDocument(const Document &document);
 
void PrintMatchDocumentResult(int document_id,
		const std::vector<std::string> &words, DocumentStatus status);
 
void AddDocument(SearchServer &search_server, int document_id,
		const std::string &document, DocumentStatus status,
		const std::vector<int> &ratings);
 
void FindTopDocuments(const SearchServer &search_server, const std::string &raw_query);
 
void MatchDocuments(const SearchServer &search_server, const std::string &query);
