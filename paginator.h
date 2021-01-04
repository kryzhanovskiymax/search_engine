#pragma once
#include <vector>
#include <iterator>
 
template<typename Iterator>
class IteratorRange {
public:
	IteratorRange(Iterator _begin, Iterator _end) :
			begin_(_begin), end_(_end), size_(distance(begin_, end_)) {
	}
 
	auto begin() const {
		return begin_;
	}
 
	auto end() const {
		return end_;
	}
 
	auto size() const {
		return size_;
	}
 
private:
	Iterator begin_;
	Iterator end_;
	size_t size_;
};
 
template<typename Iterator>
class Paginator {
public:
	explicit Paginator(Iterator a, Iterator b, long long size) {
		auto i = a;
		while (a != b){
			while(distance(a,i) != size){
				if (i==b) break;
				advance(i,1);
			}
			IteratorRange<Iterator> begin = { a, i };
			pages_.push_back(begin);
			a = i;
		}
	}
 
	auto begin() const {
		return pages_.begin();
	}
 
	auto end() const {
		return pages_.end();
	}
 
private:
	std::vector<IteratorRange<Iterator>> pages_;
};
 
template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
 
template<typename Iterator>
std::ostream& operator<<(std::ostream &out, const IteratorRange<Iterator> &range) {
	for (Iterator it = range.begin(); it != range.end(); ++it) {
		out << *it;
	}
	return out;
}
