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
	~big_integer();

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

 private:
	big_integer(bool sign, std::vector<uint32_t> digits);
	friend int compare(const big_integer &a, const big_integer &b);
	bool positive() const;
	void normalize();

	bool sign;
	std::vector<uint32_t> dig;
};

std::ostream &operator<<(std::ostream &s, const big_integer &a);

#endif // BIG_INTEGER_H