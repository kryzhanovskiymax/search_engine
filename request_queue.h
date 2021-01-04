#pragma once
#include <vector>
#include <string>
#include <deque>
#include "search_server.h"
 
class RequestQueue {
public:
	explicit RequestQueue(SearchServer &search_server) :
			search_server_(search_server) {
	}
 
	// сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
	template<typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string &raw_query,
			DocumentPredicate document_predicate) {
		std::vector<Document> out = search_server_.FindTopDocuments(raw_query,
				document_predicate);
		AddRequest(out);
		return out;
	}
 
	std::vector<Document> AddFindRequest(const std::string &raw_query,
			DocumentStatus status);
 
	std::vector<Document> AddFindRequest(const std::string &raw_query);
 
	int GetNoResultRequests() const;
 
private:
 
	struct QueryResult {
		std::vector<Document> doc;
		bool is_empty;
	};
	std::deque<QueryResult> requests_;
	const static int sec_in_day_ = 1440;
 
	int empty_results_ = 0;
	SearchServer &search_server_;
 
	void AddRequest(const std::vector<Document> &out);
};
