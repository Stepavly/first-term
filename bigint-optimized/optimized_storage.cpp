#include "optimized_storage.h"
#include <utility>

uint32_storage::uint32_storage(uint32_t value)
		: is_small(true)
		, small_size(1) {
	so.small[0] = value;
}

uint32_storage::uint32_storage(std::vector<uint32_t> const &other) : is_small(false) {
	so.big = new my_shared_pointer(other);
}

uint32_storage::uint32_storage(uint32_storage const &other) {
	is_small = other.is_small;
	small_size = other.small_size;

	if (is_small) {
		for (size_t i = 0; i < other.small_size; i++) {
			so.small[i] = other.so.small[i];
		}
	} else {
		so.big = other.so.big;
		so.big->ref_counter++;
	}
}

uint32_storage &uint32_storage::operator=(uint32_storage const &other) {
	if (this == &other) {
		return *this;
	}

	uint32_storage temp(other);
	swap(temp);
	return *this;
}

uint32_storage::~uint32_storage() {
	if (!is_small) {
		if (so.big->unique()) {
			delete so.big;
		} else {
			so.big->ref_counter--;
		}
	}
}

uint32_t uint32_storage::operator[](size_t index) const {
	return is_small ? so.small[index] : so.big->data[index];
}

uint32_t &uint32_storage::operator[](size_t index) {
	if (is_small) {
		return so.small[index];
	} else if (so.big->unique()) {
		return so.big->data[index];
	} else {
		so.big->ref_counter--;
		so.big = new my_shared_pointer(so.big->data);
		return so.big->data[index];
	}
}

void uint32_storage::increase_size(size_t new_size) {
	increase_size(new_size, 0u);
}

void uint32_storage::increase_size(size_t new_size, uint32_t value) {
	if (is_small) {
		if (new_size > SMALL_CAPACITY) {
			is_small = false;
			std::vector<uint32_t> temp(new_size, value);

			for (size_t i = 0; i < small_size; i++) {
				temp[i] = so.small[i];
			}

			so.big = new my_shared_pointer(temp);
		} else {
			std::fill(so.small + small_size, so.small + new_size, value);
			small_size = new_size;
		}
	} else {
		if (so.big->unique()) {
			so.big->data.resize(new_size, value);
		} else {
			so.big->ref_counter--;
			so.big = new my_shared_pointer(so.big->data);
			so.big->data.resize(new_size, value);
		}
	}
}

void uint32_storage::push_back(uint32_t value) {
	increase_size(size() + 1, value);
}

void uint32_storage::pop_back() {
	if (is_small) {
		small_size--;
	} else {
		if (so.big->unique()) {
			so.big->data.pop_back();
		} else {
			so.big->ref_counter--;
			so.big = new my_shared_pointer(so.big->data);
			so.big->data.pop_back();
		}
	}
}

uint32_t &uint32_storage::back() {
	if (is_small) {
		return so.small[small_size - 1];
	} else if (so.big->unique()) {
		return so.big->data.back();
	} else {
		so.big->ref_counter--;
		so.big = new my_shared_pointer(so.big->data);
		return so.big->data.back();
	}
}

size_t uint32_storage::size() const {
	return is_small ? small_size : so.big->data.size();
}

void uint32_storage::swap(uint32_storage &other) {
	if (is_small && other.is_small) {
		for (size_t i = 0; i < SMALL_CAPACITY; i++) {
			std::swap(so.small[i], other.so.small[i]);
		}

		std::swap(small_size, other.small_size);
	} else if (is_small && !other.is_small) {
		my_shared_pointer *copy = other.so.big;

		for (size_t i = 0; i < small_size; i++) {
			other.so.small[i] = so.small[i];
		}

		other.small_size = small_size;
		other.is_small = true;

		is_small = false;
		so.big = copy;
	} else if (!is_small && other.is_small) {
		my_shared_pointer *copy = so.big;

		for (size_t i = 0; i < other.small_size; i++) {
			so.small[i] = other.so.small[i];
		}

		small_size = other.small_size;
		is_small = true;

		other.is_small = false;
		other.so.big = copy;
	} else {
		std::swap(so.big, other.so.big);
	}
}

bool operator==(uint32_storage const &lhs, uint32_storage const &rhs) {
	if (lhs.size() != rhs.size()) {
		return false;
	}
	for (size_t i = 0; i < lhs.size(); i++) {
		if (lhs[i] != rhs[i]) {
			return false;
		}
	}

	return true;
}