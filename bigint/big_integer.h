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

	template<class BitFunction>
	friend big_integer bit_function_applier(big_integer lhs,
	                                        big_integer rhs,
	                                        BitFunction const &bit_function) {
		size_t result_len = std::max(lhs.size(), rhs.size()) + 1;

		if (!lhs.sign) {
			++lhs;
			lhs.dig.resize(result_len, 0u);

			for (uint32_t &x : lhs.dig) {
				x = ~x;
			}
		} else {
			lhs.dig.resize(result_len, 0u);
		}

		if (!rhs.sign) {
			++rhs;
			rhs.dig.resize(result_len, 0u);

			for (uint32_t &x : rhs.dig) {
				x = ~x;
			}
		} else {
			rhs.dig.resize(result_len, 0u);
		}

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