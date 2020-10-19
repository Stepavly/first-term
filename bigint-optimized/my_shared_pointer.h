#ifndef BIGINT__MY_SHARED_POINTER_H_
#define BIGINT__MY_SHARED_POINTER_H_

#include <cstdint>
#include <vector>

struct my_shared_pointer {
	my_shared_pointer()
			: ref_counter(1) {}

	my_shared_pointer(std::vector<uint32_t> const &other)
			: ref_counter(1)
			, data(other) {}

	bool unique() const {
		return ref_counter == 1;
	}

	size_t ref_counter;
	std::vector<uint32_t> data;
};

#endif //BIGINT__MY_SHARED_POINTER_H_
