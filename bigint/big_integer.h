#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <vector>
#include <string>

struct big_integer {
	big_integer();
	big_integer(const big_integer &other);
	big_integer(int a);
	explicit big_integer(const std::string &str);
	big_integer(uint32_t a);
	~big_integer() = default;

	big_integer &operator=(big_integer const &other);

	big_integer &operator+=(big_integer const &rhs);
	big_integer &operator-=(big_integer const &rhs);
	big_integer &operator*=(big_integer const &rhs);
	big_integer &operator/=(big_integer const &rhs);
	big_integer &operator%=(big_integer const &rhs);

	big_integer &operator&=(big_integer const &rhs);
	big_integer &operator|=(big_integer const &rhs);
	big_integer &operator^=(big_integer const &rhs);

	big_integer &operator<<=(uint32_t rhs);
	big_integer &operator>>=(uint32_t rhs);

	big_integer operator+() const;
	big_integer operator-() const;
	big_integer operator~() const;

	big_integer &operator++();
	big_integer operator++(int);

	big_integer &operator--();
	big_integer operator--(int);

	friend bool operator==(const big_integer &a, const big_integer &b);
	friend bool operator!=(const big_integer &a, const big_integer &b);
	friend bool operator<(const big_integer &a, const big_integer &b);
	friend bool operator>(const big_integer &a, const big_integer &b);
	friend bool operator<=(const big_integer &a, const big_integer &b);
	friend bool operator>=(const big_integer &a, const big_integer &b);

	friend big_integer operator+(big_integer a, const big_integer &b);
	friend big_integer operator-(big_integer a, const big_integer &b);
	friend big_integer operator*(big_integer a, const big_integer &b);
	friend big_integer operator/(big_integer a, const big_integer &b);
	friend big_integer operator%(big_integer a, const big_integer &b);

	friend big_integer operator&(big_integer a, const big_integer &b);
	friend big_integer operator|(big_integer a, const big_integer &b);
	friend big_integer operator^(big_integer a, const big_integer &b);

	friend big_integer operator<<(big_integer a, uint32_t b);
	friend big_integer operator>>(big_integer a, uint32_t b);

	friend std::string to_string(big_integer a);

	bool positive() const;
	bool is_zero() const;

 private:
	big_integer(bool sign, std::vector<uint32_t> digits);
	friend int compare(const big_integer &a, const big_integer &b);

	bool is_smaller(const big_integer &other, size_t other_size) {
		for (size_t i = 1; i <= dig.size(); i++) {
			uint32_t other_dig = other_size - i < other.dig.size() ? other.dig[other_size - i] : 0u;

			if (dig[dig.size() - i] != other_dig) {
				return dig[dig.size() - i] >= other_dig;
			}
		}

		return true;
	}

	void difference(const big_integer &other, size_t shift) {
		bool take = false;

		for (size_t k = 0; k + shift < size(); k++) {
			int64_t diff = static_cast<int64_t>(dig[shift + k]);
			diff -= static_cast<int64_t>(k < other.size() ? other[k] : 0);
			diff -= static_cast<int64_t>(take);

			take = diff < 0;
			dig[shift + k] = static_cast<uint32_t>(diff);
		}

		if (!dig.back()) {
			dig.pop_back();
		}
	}

	big_integer bit_shift(ptrdiff_t shift) {
		big_integer shifted(sign, dig);
		sign = true;

		if (shift > 0) {
			shifted.dig.resize(shifted.size() + shift / (32 - 1) + 2);
		}

		transform_to_compl2(*this, size());

		for (size_t i = 0; i < shifted.size(); i++) {
			ptrdiff_t start_bit = i * 32 - shift;
			ptrdiff_t end_bit = (i + 1) * 32 - 1 - shift;
			ptrdiff_t start_block = start_bit / 32u;
			ptrdiff_t end_block = end_bit / 32u;

			uint32_t cnt = (32 - shift % 32) % 32;
			uint32_t left_block, right_block;

			if ((start_bit < 0 && shift >= 0) || start_block >= static_cast<ptrdiff_t>(dig.size())) {
				left_block = 0;
			} else {
				left_block = dig[start_block];
			}

			if ((end_bit < 0 && shift >= 0) || end_block >= static_cast<ptrdiff_t>(dig.size())) {
				right_block = 0;
			} else {
				right_block = dig[end_block];
			}

			left_block >>= cnt;
			right_block &= (1U << cnt) - 1U;

			shifted[i] = (cnt ? (right_block << (32u - cnt)) : 0) + left_block;
		}

		if (shift < 0 && !shifted.sign) {
			--shifted;
		}

		shifted.normalize();
		return shifted;
	}

	uint32_t operator[](size_t index) const {
		return dig[index];
	}

	uint32_t &operator[](size_t index) {
		return dig[index];
	}

	size_t size() const {
		return dig.size();
	}

	void normalize();

	static void transform_to_compl2(big_integer &a, size_t new_size) {
		if (!a.sign) {
			++a;
			a.dig.resize(new_size, 0u);

			for (uint32_t &x : a.dig) {
				x = ~x;
			}
		} else {
			a.dig.resize(new_size, 0u);
		}
	}

	template<class BitFunction>
	friend big_integer bit_function_applier
			(big_integer lhs,
			 big_integer rhs,
			 BitFunction const &bit_function) {
		size_t result_len = std::max(lhs.size(), rhs.size()) + 1;

		transform_to_compl2(lhs, result_len);
		transform_to_compl2(rhs, result_len);

		std::vector<uint32_t> result_num(result_len);
		bool result_sign = !bit_function(!lhs.sign, !rhs.sign);

		for (size_t i = 0; i < result_len; i++) {
			uint32_t a = i < lhs.size() ? lhs[i] : 0;
			uint32_t b = i < rhs.size() ? rhs[i] : 0;
			uint32_t c = bit_function(a, b);

			if (!result_sign) {
				c = ~c;
			}

			result_num[i] = c;
		}

		big_integer result = big_integer(result_sign, result_num);

		return result_sign ? result : --result;
	}

	std::pair<big_integer, uint32_t> div_mod_short(uint32_t rhs);
	std::pair<big_integer, big_integer> div_mod_long(big_integer const &rhs);

	bool sign;
	std::vector<uint32_t> dig;
};

std::ostream &operator<<(std::ostream &s, const big_integer &a);

#endif // BIG_INTEGER_H