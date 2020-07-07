#ifndef BIGINT__OPTIMIZED_STORAGE_H_
#define BIGINT__OPTIMIZED_STORAGE_H_

#include "my_shared_pointer.h"
#include <vector>

struct uint32_storage {
	uint32_storage();
	uint32_storage(std::vector<uint32_t> const &other);
	uint32_storage(uint32_storage const &other);
	uint32_storage &operator=(uint32_storage const &other);
	~uint32_storage();

	uint32_t operator[](size_t index) const;
	uint32_t &operator[](size_t index);

	void resize(size_t new_size);
	void resize(size_t new_size, uint32_t value = 0u);
	void push_back(uint32_t value);
	void pop_back();
	uint32_t back() const;
	size_t size() const;
	void swap(uint32_storage &other);

 private:
	constexpr static size_t SMALL_CAPACITY = sizeof(my_shared_pointer *) / sizeof(uint32_t);

	bool is_small;
	size_t small_size;

	union {
		uint32_t small[SMALL_CAPACITY];
		my_shared_pointer *big;
	} so;
};

bool operator==(uint32_storage const &lhs, __uint32_t const &rhs);

#endif //BIGINT__OPTIMIZED_STORAGE_H_
