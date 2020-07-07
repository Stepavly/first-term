#include "optimized_storage.h"

uint32_storage::uint32_storage() :
	is_small(true),
	small_size(1) {
	std::fill(so.small, so.small + SMALL_CAPACITY, 0u);
}

uint32_storage::uint32_storage(std::vector<uint32_t> const& other) :
	is_small(false) {
	so.big = new my_shared_pointer(other);
}

uint32_storage::uint32_storage(uint32_storage const& other) {
	uint32_storage temp = *this;
	swap()
}

uint32_storage& uint32_storage::operator=(uint32_storage const& other) {
	if (this == &other) {
		return *this;
	}


}

uint32_storage::~uint32_storage() {
	if (!is_small) {
		delete so.big;
	}
}

uint32_t uint32_storage::operator[](size_t index) const {

}

uint32_t& uint32_storage::operator[](size_t index) {

}

void uint32_storage::resize(size_t new_size) {
	resize(new_size, 0u);
}

void uint32_storage::resize(size_t new_size, uint32_t value) {

}

void uint32_storage::push_back(uint32_t value) {

}

void uint32_storage::pop_back() {

}

uint32_t uint32_storage::back() const {

}

size_t uint32_storage::size() const {

}