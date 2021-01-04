#include "search_server.h"
 
void SearchServer::AddDocument(int document_id, const std::string &document,
		DocumentStatus status, const std::vector<int> &ratings) {
	if (document_id < 0)
		throw std::invalid_argument("Negative ID");
	if (documents_.count(document_id) > 0) 
		throw std::invalid_argument("This ID already exists");		
	const std::vector<std::string> words = SplitIntoWordsNoStop(document);
	const double inv_word_count = 1.0 / words.size();
	for (const std::string &word : words) {
		word_to_document_freqs_[word][document_id] += inv_word_count;
        ids_to_words_[document_id][word] += inv_word_count;
	}
	//Записываем рейтинг и статус
	documents_[document_id] = { ComputeAverageRating(ratings), status };
	document_ids_.push_back(document_id);
}
 
 
 
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(
		const std::string &raw_query, int document_id) const {
 
	const Query &query = ParseQuery(raw_query);
 
	std::vector<std::string> matched_words;
 
	const DocumentStatus status = documents_.at(document_id).status;
 
	for (const std::string &word : query.minus_words) {
		// Проверим наличие ключа в словаре
		if (word_to_document_freqs_.count(word)) {
			if (word_to_document_freqs_.at(word).count(document_id)) {
				return make_tuple(matched_words, status);
			}
		}
	}
 
	for (const std::string &word : query.plus_words) {
		if (word_to_document_freqs_.count(word)) {
			if (word_to_document_freqs_.at(word).count(document_id) == 1) {
				matched_words.push_back(word);
			}
		}
 
	}
	return make_tuple(matched_words, status);
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const{
    return ids_to_words_.find(document_id)->second;
}


void SearchServer::RemoveDocument(int document_id){
    auto iter = documents_.find(document_id);
    documents_.erase(iter);
    document_ids_.erase(std::remove(document_ids_.begin(), document_ids_.end(), document_id), document_ids_.end());
    for(auto element : word_to_document_freqs_){
        auto it = element.second.find(document_id);
        if(it != element.second.end()){
            element.second.erase(it);
        }
    }
    auto it = ids_to_words_.find(document_id);
    ids_to_words_.erase(it);
 }
 
int SearchServer::GetDocumentCount() const {
	return documents_.size();
}
 
int SearchServer::GetDocumentId(int index) const {
	int size = document_ids_.size();
	if (index < 0 || index >= size)
		throw std::out_of_range("Invalid document id");
	return document_ids_[index];
}
 
//private section methods
bool SearchServer::IsStopWord(const std::string &word) const {
	return stop_words_.count(word) > 0;
}
 
void SearchServer::SetStopWords(const std::string &text) {
	for (const std::string &word : SplitIntoWords(text)) {
		stop_words_.insert(word);
	}
}
 
std::vector<std::string> SearchServer::SplitIntoWordsNoStop(
		const std::string &text) const {
	std::vector<std::string> words;
	for (const std::string &word : SplitIntoWords(text)) {
		if (!IsValidWord(word))
			throw std::invalid_argument("Invalid symbol in a word");
 
		if (!IsStopWord(word)) {
			words.push_back(word);
		}
	}
	return words;
}
 
int SearchServer::ComputeAverageRating(const std::vector<int> &ratings) {
	int rating_sum = 0;
	for (const int rating : ratings) {
		rating_sum += rating;
	}
	return rating_sum / static_cast<int>(ratings.size());
}
 
SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
	bool is_minus = false;
 
	if (text.empty()) {
		throw std::invalid_argument("empty word");
	}
 
	if (text.size() > 1) {
		if (text[0] == '-' && text[1] == '-') {
			throw std::invalid_argument("More than one minus before a word");
		}
	}
 
	if (text.size() == 1 && text[0] == '-') {
		throw std::invalid_argument("empty word after a minus");
	}
 
	if (!IsValidWord(text)) {
		throw std::invalid_argument("invalid symbol in a word");
	}
 
	if (text[0] == '-') {
		is_minus = true;
		text = text.substr(1);
	}
	return {
		text,
		is_minus,
		IsStopWord(text)
	};
}
 
SearchServer::Query SearchServer::ParseQuery(const std::string &text) const {
		Query query;
		for (const std::string &word : SplitIntoWords(text)) {
			const QueryWord query_word = ParseQueryWord(word);
			if (!query_word.is_stop) {
				if (query_word.is_minus) {
					query.minus_words.insert(query_word.data);
				} else {
					query.plus_words.insert(query_word.data);
				}
			}
		}
		return query;
	}
 
double SearchServer::ComputeWordInverseDocumentFreq(const std::string &word) const {
		return std::log(
				documents_.size() * 1.0
						/ word_to_document_freqs_.at(word).size());
	}
 
 
 
bool SearchServer::IsValidWord(const std::string &word) {
		// A valid word must not contain special characters
		return none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
		});
	}
 
void PrintDocument(const Document &document) {
	std::cout << "{ "<< "document_id = "<< document.id << ", "
			<< "relevance = "<< document.relevance << ", "<< "rating = "
			<< document.rating << " }"<< std::endl;
}
 
void PrintMatchDocumentResult(int document_id, const std::vector<std::string> &words,
		DocumentStatus status) {
	std::cout << "{ "<< "document_id = "<< document_id << ", "<< "status = "
			<< static_cast<int>(status) << ", "<< "words =";
	for (const std::string &word : words) {
		std::cout << ' ' << word;
	}
	std::cout << "}"<< std::endl;
}
 
void AddDocument(SearchServer &search_server, int document_id,
		const std::string &document, DocumentStatus status,
		const std::vector<int> &ratings) {
	try {
		search_server.AddDocument(document_id, document, status, ratings);
	} catch (const std::invalid_argument &e) {
		std::cout << "Ошибка добавления документа "<< document_id << ": "
				<< e.what() << std::endl;
	}
}
 
void FindTopDocuments(const SearchServer &search_server,
		const std::string &raw_query) {
	std::cout << "Результаты поиска по запросу: "<< raw_query << std::endl;
	try {
		for (const Document &document : search_server.FindTopDocuments(
				raw_query)) {
			PrintDocument(document);
		}
	} catch (const std::invalid_argument &e) {
		std::cout << "Ошибка поиска: "<< e.what() << std::endl;
	}
}
 
void MatchDocuments(const SearchServer &search_server, const std::string &query) {
	try {
		std::cout << "Матчинг документов по запросу: "<< query << std::endl;
		const int document_count = search_server.GetDocumentCount();
		for (int index = 0; index < document_count; ++index) {
			const int document_id = search_server.GetDocumentId(index);
			const auto [words, status] = search_server.MatchDocument(query,
					document_id);
			PrintMatchDocumentResult(document_id, words, status);
		}
	} catch (const std::invalid_argument &e) {
		std::cout << "Ошибка матчинга документов на запрос "<< query << ": "
				<< e.what() << std::endl;
	}
}
