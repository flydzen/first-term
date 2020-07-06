#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <iosfwd>
#include <vector>
#include <cstdint>

using namespace std;

struct big_integer {
     typedef unsigned __int128 uint128_t;
     big_integer();
     big_integer(big_integer const &other);
     big_integer(int a);
     explicit big_integer(std::string const &str);
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

     big_integer &operator<<=(int rhs);
     big_integer &operator>>=(int rhs);

     big_integer operator+() const;
     big_integer operator-() const;
     big_integer operator~() const;

     big_integer &operator++();
     big_integer operator++(int);

     big_integer &operator--();
     big_integer operator--(int);

     friend bool operator==(big_integer const &a, big_integer const &b);
     friend bool operator!=(big_integer const &a, big_integer const &b);
     friend bool operator<(big_integer const &a, big_integer const &b);
     friend bool operator>(big_integer const &a, big_integer const &b);
     friend bool operator<=(big_integer const &a, big_integer const &b);
     friend bool operator>=(big_integer const &a, big_integer const &b);

     friend std::string to_string(big_integer const &a);

 private:
     vector<uint32_t> data_;
     bool positive;
     static const uint32_t BASE = 32;
     static const uint32_t MAX_DIGIT = (((uint64_t) 1) << BASE) - 1;
     static const uint64_t BASE_DIGIT = ((uint64_t) 1) << BASE;
     static pair<uint32_t, uint32_t> split64(uint64_t n);
     void sumABS(big_integer const &b);
     void subABS(big_integer const &b);
     static void toFit(vector<uint32_t> &v);
     vector<uint32_t> additionTo2(vector<uint32_t> const &v, bool is2 = false) const;
     static bool highBit(vector<uint32_t> &v);
     static pair<big_integer, big_integer> div(big_integer &v, big_integer const &d);
     static pair<big_integer, big_integer> divM_N(big_integer &v, big_integer const &d);
     static pair<big_integer, big_integer> div_primal(big_integer &v, big_integer const &d);
     static pair<uint32_t, uint32_t> div3_2_primal(uint32_t u1, uint32_t u2, uint32_t u3, uint32_t d1, uint32_t d2);
     static pair<big_integer, big_integer> divN_1(big_integer &v, big_integer const &d);
};

big_integer operator+(big_integer a, big_integer const &b);
big_integer operator-(big_integer a, big_integer const &b);
big_integer operator*(big_integer a, big_integer const &b);
big_integer operator/(big_integer a, big_integer const &b);
big_integer operator%(big_integer a, big_integer const &b);

big_integer operator&(big_integer a, big_integer const &b);
big_integer operator|(big_integer a, big_integer const &b);
big_integer operator^(big_integer a, big_integer const &b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const &a, big_integer const &b);
bool operator!=(big_integer const &a, big_integer const &b);
bool operator<(big_integer const &a, big_integer const &b);
bool operator>(big_integer const &a, big_integer const &b);
bool operator<=(big_integer const &a, big_integer const &b);
bool operator>=(big_integer const &a, big_integer const &b);

std::string to_string(big_integer const &a);
std::ostream &operator<<(std::ostream &s, big_integer const &a);

#endif // BIG_INTEGER_H
