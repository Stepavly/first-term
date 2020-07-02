#include "big_integer.h"
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <utility>
#include <iostream>

typedef std::pair<uint32_t, uint32_t> arithmetic_type;

bool big_integer::positive() const {
	return sign;
}
big_integer abs(const big_integer &a) {
	return a < 0 ? -a : a;
}

arithmetic_type add(uint32_t a, uint32_t b) {
	arithmetic_type result(0u, a + b);

	if (0xFFFFFFFFu - a < b) {
		result.first = 1;
	}

	return result;
}
arithmetic_type mult(uint32_t a, uint32_t b) {
	arithmetic_type result(0u, 0u);

	if (b & 1u) {
		result.second = a;
	}

	for (uint32_t bit = 1, high_mask = 1u << 31u; bit < 32u; high_mask |= 1u << (31 - bit), bit++) {
		if (b >> bit & 1u) {
			uint32_t high = (a & high_mask) >> (32 - bit);
			uint32_t low = (a & (~high_mask)) << bit;

			arithmetic_type sum = add(result.second, low);
			result.first += high + sum.first;
			result.second = sum.second;
		}
	}

	return result;
}

big_integer::big_integer() : sign(true), dig(1, 0u) {}
big_integer::big_integer(const big_integer &other) : sign(other.sign), dig(other.dig) {}
big_integer::big_integer(int a) {
	sign = a >= 0;
	uint32_t b = (a == std::numeric_limits<int>::min() ?
								(uint32_t) std::numeric_limits<int>::max() + 1u :
								(uint32_t) (a < 0 ? -a : a));
	dig = std::vector<uint32_t>();
	dig.push_back(b);
}
big_integer::big_integer(uint32_t a) : sign(true) {
	dig = std::vector<uint32_t>();
	dig.push_back(a);
}
big_integer::big_integer(const std::string &str) {
	if (str.empty()) {
		throw std::length_error("can not create big_int from empty string");
	}
	sign = str[0] != '-';
	big_integer a;

	for (size_t i = isdigit(str[0]) ? 0 : 1; i < str.size(); i++) {
		if (!isdigit(str[i])) {
			throw std::logic_error("string must contain only digits");
		}

		a *= 10;
		a += str[i] - '0';
	}

	if (a == 0) {
		sign = true;
	}

	*this = sign ? a : -a;
}
big_integer::big_integer(bool sign, std::vector<uint32_t> digits) : sign(sign), dig(std::move(digits)) {}
big_integer::~big_integer() {}

big_integer &big_integer::operator=(big_integer const &other) {
	if (this == &other) {
		return *this;
	}
	this->sign = other.sign;
	this->dig = std::vector<uint32_t>(other.dig);

	return *this;
}

int compare(const big_integer &a, const big_integer &b) {
	if (a.dig.size() > b.dig.size()) {
		return a.positive() ? +1 : -1;
	} else if (a.dig.size() < b.dig.size()) {
		return b.positive() ? -1 : +1;
	} else {
		if (a.positive() != b.positive()) {
			return a.positive() ? +1 : -1;
		}

		for (size_t i = a.dig.size() - 1;; i--) {
			if (a.dig[i] < b.dig[i]) {
				return a.positive() ? -1 : +1;
			} else if (a.dig[i] > b.dig[i]) {
				return a.positive() ? +1 : -1;
			}
			if (i == 0) {
				break;
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

	if (dig.size() == 1 && dig[0] == 0) {
		sign = true;
	}
}

big_integer big_integer::operator+() const {
	return *this;
}
big_integer big_integer::operator-() const {
	big_integer negative(*this);
	if (negative != 0) {
		negative.sign = !negative.sign;
	}
	return negative;
}
big_integer big_integer::operator~() const {
	return -(*this) - 1;
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
	if (rhs == 0) {
		return *this;
	} else if (sign != rhs.sign) {
		return *this -= -rhs;
	}

	uint32_t carry = 0;

	for (size_t i = 0; i < rhs.dig.size() || carry > 0; i++) {
		arithmetic_type curDig(0, carry);

		if (i == dig.size()) {
			dig.push_back(0);
		}

		curDig = add(carry, dig[i]);

		if (i < rhs.dig.size()) {
			auto temp = add(curDig.second, rhs.dig[i]);
			curDig = std::make_pair(curDig.first + temp.first, temp.second);
		}

		carry = curDig.first;
		dig[i] = curDig.second;
	}

	normalize();

	return *this;
}
big_integer &big_integer::operator-=(big_integer const &rhs) {
	if (rhs == 0) {
		return *this;
	} else if (sign != rhs.sign) {
		return *this += -rhs;
	} else if ((positive() && *this < rhs) || (!positive() && *this > rhs)) {
		big_integer temp = rhs;
		return *this = -(temp -= *this);
	}

	uint32_t take = 0;

	for (size_t i = 0; i < dig.size(); i++) {
		bool high_bits_used = false;
		uint32_t sub = 0;

		if (i < rhs.dig.size()) {
			sub = rhs.dig[i];
		}

		if (dig[i] < take || dig[i] - take < sub) {
			high_bits_used = true;
		}

		dig[i] = dig[i] - sub - take;
		take = high_bits_used;
	}

	normalize();

	return *this;
}
big_integer &big_integer::operator*=(big_integer const &rhs) {
	big_integer c;
	c.sign = sign == rhs.sign;
	c.dig = std::vector<uint32_t>(dig.size() + rhs.dig.size());

	for (size_t i = 0; i < dig.size(); i++) {
		uint32_t carry = 0;

		for (size_t j = 0; j < rhs.dig.size() || carry; j++) {
			auto aMulB = mult(dig[i], j < rhs.dig.size() ? rhs.dig[j] : 0); // a[i]*b[j]
			auto temp = add(aMulB.second, carry);
			aMulB = std::pair<uint32_t, uint32_t>(aMulB.first + temp.first, temp.second); // a[i]*b[j]+carry
			temp = add(aMulB.second, c.dig[i + j]);
			aMulB = std::pair<uint32_t, uint32_t>(aMulB.first + temp.first, temp.second); // a[i]*b[j]+carry+c[i+j]

			c.dig[i + j] = aMulB.second;
			carry = aMulB.first;
		}
	}

	c.normalize();

	return *this = c;
}
big_integer &big_integer::operator/=(big_integer const &rhs) {
	if (rhs == 0) {
		throw std::range_error("division by zero");
	} else if (abs(*this) < abs(rhs)) {
		return *this = big_integer(0);
	}

	big_integer c;
	c.sign = sign == rhs.sign;
	c.dig = std::vector<uint32_t>(dig.size());
	big_integer cur;
	cur.dig.clear();

	for (size_t i = dig.size() - 1;; i--) {
		cur.dig.insert(cur.dig.begin(), 0);
		cur += dig[i];

		uint32_t l = 0, r = 0xFFFFFFFFu;

		while (r - l > 1) {
			uint32_t m = l + (r - l) / 2;
			big_integer temp = abs(rhs * m);

			if (cur >= abs(rhs * m)) {
				l = m;
			} else {
				r = m;
			}
		}

		if (cur >= abs(rhs * r)) {
			c.dig[i] = r;
		} else {
			c.dig[i] = l;
		}

		cur = cur - abs(rhs * c.dig[i]);

		if (i == 0) {
			break;
		}
	}

	c.normalize();

	return *this = c;
}
big_integer &big_integer::operator%=(big_integer const &rhs) {
	return *this = *this - *this / rhs * rhs;
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

template<class BitFunction>
std::vector<uint32_t> bit_function_applier
		(std::vector<uint32_t> const &lhs,
		 bool lhs_sign,
		 std::vector<uint32_t> const &rhs,
		 bool rhs_sign,
		 bool need_inv,
		 BitFunction bit_function) {
	std::vector<uint32_t> result(std::max(lhs.size(), rhs.size()));

	for (size_t i = 0; i < lhs.size(); i++) {
		uint32_t a = (i < lhs.size() ? (lhs_sign ? lhs[i] : -lhs[i]) : 0);
		uint32_t b = (i < rhs.size() ? (rhs_sign ? rhs[i] : -rhs[i]) : 0);
		uint32_t c = bit_function(a, b);

		if (need_inv) {
			c = -c;
		}

		result[i] = c;
	}

	return result;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
	*this = big_integer
			(sign || rhs.sign,
			 bit_function_applier(dig, sign, rhs.dig, rhs.sign, false, std::bit_and<uint32_t>()));

	normalize();
	return *this;
}
big_integer &big_integer::operator|=(big_integer const &rhs) {
	*this = big_integer
			(sign && rhs.sign,
			 bit_function_applier(dig, sign, rhs.dig, rhs.sign, !(sign && rhs.sign), std::bit_or<uint32_t>()));

	normalize();
	return *this;
}
big_integer &big_integer::operator^=(big_integer const &rhs) {
	*this = big_integer
			(!(sign ^ rhs.sign),
			 bit_function_applier(dig, sign, rhs.dig, rhs.sign, sign ^ rhs.sign, std::bit_xor<uint32_t>()));

	normalize();
	return *this;
}

big_integer pow(big_integer a, size_t power) {
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
	while (rhs > 0) {
		if (sign) {
			*this /= 2;
		} else {
			*this = (*this - 1) / 2;
		}
		rhs--;
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
	return *this = *this + 1;
}
big_integer big_integer::operator++(int) {
	big_integer copy(*this);
	++*this;
	return copy;
}
big_integer &big_integer::operator--() {
	return *this = *this + 1;
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

	do {
		std::string d = std::to_string((a % 1000000000).dig[0]);
		std::string t = std::string(needZero, '0');
		needZero = 9u - d.size();
		d += t;
		std::reverse(d.begin(), d.end());
		result += d;
		a /= 1000000000;
	} while (a != 0);

	result += sign;

	std::reverse(result.begin(), result.end());
	return result;
}

std::ostream &operator<<(std::ostream &s, const big_integer &a) {
	s << to_string(a);
	return s;
}
