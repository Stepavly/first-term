#include "big_integer.h"
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <functional>
#include <utility>
#include <iostream>

typedef std::pair<uint32_t, uint32_t> arithmetic_t;
typedef unsigned int uint128_t __attribute__((mode(TI)));

bool big_integer::positive() const {
	return sign;
}

bool big_integer::is_zero() const {
	return dig.size() == 1 && dig[0] == 0;
}

big_integer abs(const big_integer &a) {
	return a < 0 ? -a : a;
}

static arithmetic_t add(uint32_t a, uint32_t b) {
	arithmetic_t result(0u, a + b);

	if (UINT32_MAX - a < b) {
		result.first = 1;
	}

	return result;
}

static arithmetic_t mult(uint32_t a, uint32_t b) {
	uint64_t c = static_cast<uint64_t>(a) * static_cast<uint64_t>(b);

	return std::make_pair(static_cast<uint32_t>(c >> 32u), static_cast<uint32_t>(c & UINT32_MAX));
}

big_integer::big_integer() : sign(true), dig({0u}) {}

big_integer::big_integer(const big_integer &other) : sign(other.sign), dig(other.dig) {}

big_integer::big_integer(int a)
		: sign(a >= 0)
		, dig({a == INT32_MIN ? static_cast<uint32_t>(INT32_MAX) + 1 : static_cast<uint32_t>(abs(a))}) {
}

big_integer::big_integer(uint32_t a)
		: sign(true)
		, dig({a}) {
}

big_integer::big_integer(const std::string &str) : big_integer() {
	if (str.empty()) {
		throw std::length_error("can not create big_int from empty string");
	}

	for (size_t i = isdigit(str[0]) ? 0 : 1; i < str.size();) {
		uint32_t to_mult = 1;
		uint32_t to_add = 0;
		size_t j = i;

		for (; j < str.size() && j - i < 9; j++) {
			if (!isdigit(str[j])) {
				throw std::runtime_error(std::string("digit expected, ") + str[j] + " found");
			}

			to_mult *= 10;
			to_add = 10 * to_add + str[j] - '0';
		}

		*this *= to_mult;
		*this += to_add;
		i = j;
	}

	sign = str[0] != '-';

	if (is_zero()) {
		sign = true;
	}
}

big_integer::big_integer(bool sign, std::vector<uint32_t> const &digits) : sign(sign), dig(digits) {
	normalize();
}

big_integer &big_integer::operator=(big_integer const &other) {
	if (this == &other) {
		return *this;
	}
	this->sign = other.sign;
	this->dig = other.dig;

	return *this;
}

int compare(const big_integer &a, const big_integer &b) {
	if (a.size() > b.size()) {
		return a.positive() ? +1 : -1;
	} else if (a.size() < b.size()) {
		return b.positive() ? -1 : +1;
	} else {
		if (a.positive() != b.positive()) {
			return a.positive() ? +1 : -1;
		}

		for (size_t i = a.size(); i > 0; i--) {
			if (a[i - 1] < b[i - 1]) {
				return a.positive() ? -1 : +1;
			} else if (a[i - 1] > b[i - 1]) {
				return a.positive() ? +1 : -1;
			}
		}

		return 0;
	}
}

bool operator==(const big_integer &a, const big_integer &b) {
	return compare(a, b) == 0;
}

bool operator!=(const big_integer &a, const big_integer &b) {
	return compare(a, b) != 0;
}

bool operator<(const big_integer &a, const big_integer &b) {
	return compare(a, b) < 0;
}

bool operator>(const big_integer &a, const big_integer &b) {
	return compare(a, b) > 0;
}

bool operator<=(const big_integer &a, const big_integer &b) {
	return compare(a, b) <= 0;
}

bool operator>=(const big_integer &a, const big_integer &b) {
	return compare(a, b) >= 0;
}

void big_integer::normalize() {
	while (dig.size() > 1u && dig.back() == 0u) {
		dig.pop_back();
	}

	if (is_zero()) {
		sign = true;
	}
}

big_integer big_integer::operator+() const {
	return *this;
}

big_integer big_integer::operator-() const {
	big_integer negative(*this);

	if (!negative.is_zero()) {
		negative.sign = !negative.sign;
	}

	return negative;
}

big_integer big_integer::operator~() const {
	return -(*this) - 1;
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
	if (rhs.is_zero()) {
		return *this;
	} else if (sign != rhs.sign) {
		return *this -= -rhs;
	}

	dig.increase_size(std::max(size(), rhs.size()) + 1, 0u);

	uint32_t carry = 0;

	for (size_t i = 0; i < rhs.size() || carry > 0; i++) {
		arithmetic_t curDig = add(carry, dig[i]);

		if (i < rhs.size()) {
			auto temp = add(curDig.second, rhs[i]);
			curDig = std::make_pair(curDig.first + temp.first, temp.second);
		}

		carry = curDig.first;
		dig[i] = curDig.second;
	}

	normalize();

	return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
	if (rhs.is_zero()) {
		return *this;
	} else if (sign != rhs.sign) {
		return *this += -rhs;
	} else if ((positive() && *this < rhs) || (!positive() && *this > rhs)) {
		big_integer temp = rhs;
		return *this = -(temp -= *this);
	}

	difference(rhs, 0);
	normalize();

	return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
	dig.increase_size(dig.size() + rhs.dig.size());
	sign = sign == rhs.sign;

	for (size_t k = dig.size(); k > 0; k--) {
		uint32_t carry = 0;
		uint32_t cur_value = 0;

		for (size_t i = k - 1, j = 0; j < std::min(k, rhs.dig.size()); i--, j++) {
			auto a_mul_b = mult(dig[i], rhs.dig[j]); // a[i]*b[j]
			auto temp = add(a_mul_b.second, carry);
			a_mul_b = std::make_pair(a_mul_b.first + temp.first, temp.second); // a[i]*b[j]+carry
			temp = add(a_mul_b.second, cur_value);
			a_mul_b = std::make_pair(a_mul_b.first + temp.first, temp.second); // a[i]*b[j]+carry+c[i+j]

			cur_value = a_mul_b.second;
			carry = a_mul_b.first;

			for (size_t carry_pos = k; carry_pos < dig.size() && carry != 0; carry_pos++) {
				auto dig_with_carry = add(dig[carry_pos], carry);
				dig[carry_pos] = dig_with_carry.second;
				carry = dig_with_carry.first;
			}
		}

		dig[k - 1] = cur_value;
	}

	normalize();
	return *this;
}

std::pair<big_integer, uint32_t> big_integer::div_mod_short(uint32_t rhs) {
	big_integer quotient;
	quotient.dig.increase_size(size(), 0u);
	uint64_t remainder = 0;

	for (size_t i = dig.size(); i > 0; i--) {
		remainder <<= 32u;
		remainder += dig[i - 1];
		uint32_t ratio = remainder / rhs;
		remainder -= ratio * rhs;
		quotient.dig[i - 1] = ratio;
	}

	quotient.normalize();
	return std::make_pair(quotient, static_cast<uint32_t>(remainder));
}

std::pair<big_integer, big_integer> big_integer::div_mod_long(big_integer const &rhs) {
	big_integer quotient(sign == rhs.sign, {});
	quotient.dig.increase_size(dig.size() - rhs.dig.size() + 1);
	big_integer rhs_abs = abs(rhs);

	big_integer dividend(*this);
	dig.push_back(0u);
	size_t n = dig.size(), m = rhs.dig.size() + 1;

	uint64_t denominator = static_cast<uint64_t>(rhs_abs.dig.back()) << 32u;
	denominator |= static_cast<uint64_t>(rhs_abs.dig[rhs_abs.dig.size() - 2]);

	for (size_t i = m - 1, j = quotient.dig.size() - 1; i != n; i++, j--) {
		uint128_t numerator = static_cast<uint128_t>(dig.back()) << 64u;
		numerator |= static_cast<uint128_t>(dig[dig.size() - 2]) << 32u;
		numerator |= static_cast<uint128_t>(dig[dig.size() - 3]);

		uint32_t ratio = static_cast<uint32_t>(numerator / denominator);
		big_integer to_sub = rhs_abs * ratio;

		if (!(*this).is_smaller(to_sub, m)) {
			ratio--;
			to_sub -= rhs_abs;
		}

		quotient.dig[j] = ratio;
		(*this).difference(to_sub, dig.size() - m);
	}

	normalize();
	quotient.normalize();

	return std::make_pair(quotient, *this);
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
	if (rhs.is_zero()) {
		throw std::range_error("division by zero");
	} else if (abs(*this) < abs(rhs)) {
		return *this = big_integer(0);
	} else if (rhs.size() == 1) {
		return *this = sign == rhs.sign ? div_mod_short(rhs[0]).first : -div_mod_short(rhs[0]).first;
	} else {
		return *this = div_mod_long(rhs).first;
	}
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
	if (rhs.is_zero()) {
		throw std::range_error("division by zero");
	} else if (rhs.size() == 1) {
		return *this = sign ? big_integer(div_mod_short(rhs[0]).second) : -big_integer(div_mod_short(rhs[0]).second);
	} else {
		return *this = *this - *this / rhs * rhs;
	}
}

big_integer operator+(big_integer a, const big_integer &b) {
	return a += b;
}

big_integer operator-(big_integer a, const big_integer &b) {
	return a -= b;
}

big_integer operator*(big_integer a, const big_integer &b) {
	return a *= b;
}

big_integer operator/(big_integer a, const big_integer &b) {
	return a /= b;
}

big_integer operator%(big_integer a, const big_integer &b) {
	return a %= b;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
	return *this = bit_function_applier(*this, rhs, std::bit_and<uint32_t>());
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
	return *this = bit_function_applier(*this, rhs, std::bit_or<uint32_t>());
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
	return *this = bit_function_applier(*this, rhs, std::bit_xor<uint32_t>());
}

static big_integer pow(big_integer a, size_t power) {
	big_integer result(1);

	while (power > 0) {
		if (power & 1u) {
			result *= a;
		}

		power >>= 1u;

		if (power > 0) {
			a *= a;
		}
	}

	return result;
}

big_integer &big_integer::operator<<=(uint32_t rhs) {
	return *this *= pow(2, rhs);
}

big_integer &big_integer::operator>>=(uint32_t rhs) {
	while (rhs-- > 0) {
		if (positive()) {
			*this /= 2;
		} else {
			*this = (*this - 1) / 2;
		}
	}

	return *this;
}

big_integer operator&(big_integer a, const big_integer &b) {
	return a &= b;
}

big_integer operator|(big_integer a, const big_integer &b) {
	return a |= b;
}

big_integer operator^(big_integer a, const big_integer &b) {
	return a ^= b;
}

big_integer operator<<(big_integer a, uint32_t b) {
	return a <<= b;
}

big_integer operator>>(big_integer a, uint32_t b) {
	return a >>= b;
}

big_integer &big_integer::operator++() {
	return *this += 1;
}

big_integer big_integer::operator++(int) {
	big_integer copy(*this);
	++*this;
	return copy;
}

big_integer &big_integer::operator--() {
	return *this -= 1;
}

big_integer big_integer::operator--(int) {
	big_integer copy(*this);
	--*this;
	return copy;
}

std::string to_string(big_integer a) {
	std::string result;
	std::string sign = a.positive() ? "" : "-";

	a = abs(a);
	size_t needZero = 0;

	const uint32_t dig_remainder = 1000000000;
	const uint32_t dig_count = 9;

	do {
		auto div_and_mod = a.div_mod_short(dig_remainder);
		std::string d = std::to_string(div_and_mod.second);
		std::string t = std::string(needZero, '0');
		needZero = dig_count - d.size();
		d += t;
		std::reverse(d.begin(), d.end());
		result += d;
		a = div_and_mod.first;
	} while (a != 0);

	result += sign;

	std::reverse(result.begin(), result.end());
	return result;
}

std::ostream &operator<<(std::ostream &s, const big_integer &a) {
	s << to_string(a);
	return s;
}