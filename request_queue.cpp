#include "request_queue.h"
 
std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query,
		DocumentStatus status) {
	std::vector<Document> out = search_server_.FindTopDocuments(raw_query,
			status);
	AddRequest(out);
	return out;
}
 
std::vector<Document> RequestQueue::AddFindRequest(
		const std::string &raw_query) {
	std::vector<Document> out = search_server_.FindTopDocuments(raw_query);
	AddRequest(out);
	return out;
}
 
int RequestQueue::GetNoResultRequests() const {
	return empty_results_;
}
 
void RequestQueue::AddRequest(const std::vector<Document> &out) {
	bool empty = out.empty();
	requests_.push_back( { out, empty });
	if (empty) {
		empty_results_++;
	}
 
	if (requests_.size() > (sec_in_day_)) {
		const QueryResult &start = requests_.front();
		if (start.is_empty) {
			empty_results_--;
		}
		requests_.pop_front();
	}
}
