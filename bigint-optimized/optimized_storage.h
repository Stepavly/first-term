#ifndef BIGINT__OPTIMIZED_STORAGE_H_
#define BIGINT__OPTIMIZED_STORAGE_H_

#include "my_shared_pointer.h"
#include <vector>

struct uint32_storage {
	uint32_storage();
	uint32_storage(std::vector<uint32_t> const &other);
	uint32_storage(uint32_t);
	uint32_storage(uint32_storage const &other);
	uint32_storage &operator=(uint32_storage const &other);
	~uint32_storage();

	uint32_t operator[](size_t index) const;
	uint32_t &operator[](size_t index);

	void increase_size(size_t new_size);
	void increase_size(size_t new_size, uint32_t value);
	void push_back(uint32_t value);
	void pop_back();
	uint32_t &back();
	size_t size() const;
	void swap(uint32_storage &other);

	friend bool operator==(uint32_storage const &lhs, uint32_storage const &rhs);

 private:
	constexpr static size_t SMALL_CAPACITY = sizeof(my_shared_pointer *) / sizeof(uint32_t);

	bool is_small;
	size_t small_size;

	union {
		uint32_t small[SMALL_CAPACITY];
		my_shared_pointer *big;
	} so;
};

#endif //BIGINT__OPTIMIZED_STORAGE_H_
