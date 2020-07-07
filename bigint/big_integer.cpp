#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>
#define u32 uint32_t

static const big_integer ZERO = 0;

big_integer::big_integer() : positive(true) {
    data_.push_back(0);
}

big_integer::big_integer(big_integer const &other) = default;

big_integer::big_integer(int a) {
    if (a == std::numeric_limits<int>::min()) {
        data_.push_back(static_cast<u32>(a));
    } else {
        data_.push_back(abs(a));
    }
    positive = a >= 0;
}

big_integer::big_integer(std::string const &str) : big_integer() {
    big_integer base = 1;
    positive = true;
    for (u32 i = str.size(); i != (u32)(str[0] == '-'); i--) {
        *this += base * (str[i - 1] - '0');
        base *= 10;
    }
    if (str[0] == '-' && *this != ZERO) {
        positive = false;
    }
}

big_integer::~big_integer() = default;

big_integer &big_integer::operator=(big_integer const &other) = default;

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (positive && !rhs.positive) {
        *this -= -rhs;
    } else if (!positive && rhs.positive) {
        *this = rhs - (-*this);
    } else {
        sum_abs(rhs);
    }
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (positive && !rhs.positive) {
        sum_abs(rhs);
    } else if (!positive && rhs.positive) {
        sum_abs(rhs);
    } else if (positive && rhs.positive) {
        bool tp = *this >= rhs;
        sub_abs(rhs);
        positive = tp;
    } else if (!positive && !rhs.positive) {
        bool tp = *this >= rhs;
        sub_abs(rhs);
        positive = tp;
    }
    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    big_integer result;
    result.data_.resize(data_.size() + rhs.data_.size());
    for (size_t i = 0; i < data_.size(); i++) {
        u32 over = 0;
        for (size_t j = 0; j < rhs.data_.size(); j++) {
            auto p = split64(static_cast<uint64_t>(data_[i]) * rhs.data_[j] + result.data_[i + j] + over);
            result.data_[i + j] = p.first;
            over = p.second;
        }
        if (over) {
            result.data_[i + rhs.data_.size()] = over;
        }
    }
    to_fit(result.data_);
    result.positive = positive == rhs.positive;
    *this = result;
    return *this;

}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool sign = positive == rhs.positive;
    positive = true;
    data_ = div(*this, rhs).first.data_;
    positive = sign;
    if (*this == ZERO) {
        positive = true;
    }
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    *this -= (*this / rhs) * rhs;
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    auto d1 = addition_to_2(data_);
    auto d2 = rhs.addition_to_2(rhs.data_);
    if (d1.size() < d2.size()) {
        d1.resize(d2.size(), highBit(d1) ? MAX_DIGIT : 0);
    }
    if (d2.size() < d1.size()) {
        d2.resize(d1.size(), highBit(d2) ? MAX_DIGIT : 0);
    }
    for (size_t i = 0; i < d1.size(); i++) {
        d1[i] &= d2[i];
    }
    bool high = highBit(d1);
    data_ = addition_to_2(d1, true);
    positive = !high;
    to_fit(data_);
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    auto d1 = addition_to_2(data_);
    auto d2 = rhs.addition_to_2(rhs.data_);
    if (d1.size() < d2.size()) {
        d1.resize(d2.size(), highBit(d1) ? MAX_DIGIT : 0);
    }
    if (d2.size() < d1.size()) {
        d2.resize(d1.size(), highBit(d2) ? MAX_DIGIT : 0);
    }
    for (size_t i = 0; i < d1.size(); i++) {
        d1[i] |= d2[i];
    }
    bool high = highBit(d1);
    data_ = addition_to_2(d1, true);
    positive = !high;
    to_fit(data_);
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    auto d1 = addition_to_2(data_);
    auto d2 = rhs.addition_to_2(rhs.data_);
    if (d1.size() < d2.size()) {
        d1.resize(d2.size(), highBit(d1) ? MAX_DIGIT : 0);
    }
    if (d2.size() < d1.size()) {
        d2.resize(d1.size(), highBit(d2) ? MAX_DIGIT : 0);
    }
    for (size_t i = 0; i < d1.size(); i++) {
        d1[i] ^= d2[i];
    }
    bool high = highBit(d1);
    data_ = addition_to_2(d1, true);
    positive = !high;
    to_fit(data_);
    return *this;
}

big_integer &big_integer::operator<<=(int rhs) {
    int in = rhs % BASE;
    int out = rhs / BASE;
    cont res((size_t) (data_.size() + out + 1));
    for (size_t i = data_.size(); i != 0; i--) {
        auto p = split64(static_cast<uint64_t>(data_[i - 1]) << in);
        res[i + out - 1] |= p.first;
        res[i + out] |= p.second;
    }
    data_ = res;
    to_fit(data_);
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    int in = rhs % BASE;
    int out = rhs / BASE;
    auto d = addition_to_2(data_);
    cont res(d.size());
    for (int i = 0; i != (int) d.size(); i++) {
        auto p = split64(static_cast<uint64_t>(d[i]) << (BASE - in));
        if (i - out >= 0) {
            res[i - out] |= p.second;
        }
        if (i - out - 1 >= 0) {
            res[i - out - 1] |= p.first;
        }
    }
    if (!positive) {
        size_t i = res.size();
        for (; i != 0; i--) {
            if (res[i - 1] == 0) {
                res[i - 1] = MAX_DIGIT;
            } else {
                break;
            }
        }
        if (i) {
            for (u32 j = 1 << 31; j != 0; j >>= 1) {
                if ((res[i - 1] & j) != 0) {
                    break;
                }
                res[i - 1] |= j;
            }
        }
    }
    data_ = addition_to_2(res, true);
    to_fit(data_);
    if (*this == ZERO) {
        positive = true;
    }
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer r(*this);
    if (r != 0) {
        r.positive ^= true;
    }
    return r;
}

big_integer big_integer::operator~() const {
    return -*this - 1;
}

big_integer &big_integer::operator++() {
    return *this += 1;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer &big_integer::operator--() {
    return *this -= 1;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

bool operator==(big_integer const &a, big_integer const &b) {
    return a.data_ == b.data_ && a.positive == b.positive;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a.positive != b.positive) {
        return !a.positive && b.positive;
    }
    if (a.data_.size() != b.data_.size()) {
        return (a.data_.size() < b.data_.size()) ^ !a.positive;
    }
    for (size_t i = a.data_.size(); i > 0; --i) {
        if (a.data_[i - 1] != b.data_[i - 1]) {
            return (a.data_[i - 1] < b.data_[i - 1]) ^ !a.positive;
        }
    }
    return false;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return !(a <= b);
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return a < b || a == b;
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

std::string to_string(big_integer const &a) {
    string res = "";
    big_integer temp = a;
    bool neg = false;
    if (!temp.positive) {
        neg = true;
        temp.positive = true;
    }
    const u32 billion = 1000000000;
    while (temp != 0) {
        string ad = to_string((temp % billion).data_[0]);
        std::reverse(ad.begin(), ad.end());
        res += ad;
        res.insert(res.end(), 9 - ad.size(), '0');
        temp /= billion;
    }
    for (; res.size() > 1 && res.back() == '0'; res.pop_back());
    if (neg && !res.empty()) {
        res += "-";
    }
    std::reverse(res.begin(), res.end());
    if (res.empty()) {
        return "0";
    }
    return res;
}
pair<uint32_t, uint32_t> big_integer::split64(uint64_t n) {
    return {n & 0xFFFFFFFF, n >> 32};
}

void big_integer::sub_abs(big_integer const &b) {
    bool loan = false;
    cont t1;
    cont t2;
    if ((*this < b) ^ !positive) {
        t1 = b.data_;
        t2 = (*this).data_;
    } else {
        t1 = (*this).data_;
        t2 = b.data_;
    }
    if (t2.size() < t1.size()) {
        t2.resize(t1.size());
    }
    for (size_t i = 0; i < t1.size(); i++) {
        bool nLoan = t1[i] < t2[i];
        t1[i] -= (t2[i] + loan);
        loan = nLoan;
    }
    (*this).data_ = t1;
    to_fit(data_);
}
void big_integer::sum_abs(big_integer const &b) {
    if (data_.size() < b.data_.size())
        data_.resize(b.data_.size());
    bool over = false;
    for (size_t i = 0; i < b.data_.size(); i++) {
        auto p = split64(static_cast<uint64_t>(data_[i]) + b.data_[i] + over);
        data_[i] = p.first;
        over = p.second;
    }
    for (size_t i = b.data_.size(); i < data_.size(); i++) {
        auto p = split64(static_cast<uint64_t>(data_[i]) + over);
        data_[i] = p.first;
        over = p.second;
    }
    if (over) {
        data_.push_back(over);
    }
}
void big_integer::to_fit(cont &v) {
    while (v.size() != 1 && v[v.size() - 1] == 0) {
        v.pop_back();
    }
}
big_integer::cont big_integer::addition_to_2(cont const &v, bool is2) const {
    big_integer temp;
    temp.data_ = v;
    bool high = highBit(temp.data_);
    if (!is2 && high) {
        temp.data_.push_back(0);
    }
    if (!is2 && positive) {
        return temp.data_;
    }
    if (is2 && !high) {
        return v;
    }
    for (unsigned int &i : temp.data_) {
        i = ~i;
    }
    temp++;
    return temp.data_;
}
bool big_integer::highBit(cont &v) {
    return (v.back() & (static_cast<u32>(1) << (BASE - 1)));
}
pair<big_integer, big_integer> big_integer::div(big_integer &v, big_integer const &d) {
    if (v.data_.size() < d.data_.size()) {
        return {0, v};
    } else if (v == d) {
        return {1, 0};
    } else if (d.data_.size() == 1) {
        return div_N_1(v, d);
    } else if (v.data_.size() <= 4 && d.data_.size() <= 4) {
        return div_primal(v, d);
    } else {
        return div_M_N(v, d);
    }
}

pair<big_integer, big_integer> big_integer::div_M_N(big_integer &v, big_integer const &d_) {
    u32 i = 0;
    auto d = d_;
    d.positive = true;
    u32 mask = 1 << (BASE - 1);
    while ((d.data_.back() & mask) == 0) {
        mask >>= 1;
        i++;
    }
    v <<= i;
    d <<= i;
    int k = v.data_.size() - d.data_.size();
    cont res;
    big_integer dk = d << (int) (BASE * k);
    if (v >= dk) {
        res.push_back(1);
        v -= dk;
    } else {
        res.push_back(0);
    }
    while (k != 0) {
        k--;
        dk = d << (int) (k * BASE);
        if (v[-1] == d[-1] && v[-2] == d[-2]) {
            res.push_back(0xFFFFFFFF);
        } else {
            auto p = div_3_2_primal(v[-1],
                                    v[-2],
                                    v[-3],
                                    d[-1],
                                    d[-2]);
            res.push_back(p.first);
        }
        big_integer temp;
        temp.data_[0] = res.back();
        v -= temp * dk;
        if (v < 0) {
            v += d;
            --res.back();
        }
    }
    std::reverse(res.begin(), res.end());
    //res.reverse();
    big_integer result;
    result.data_ = res;
    to_fit(result.data_);
    return {result, v >> i};
}

pair<big_integer, big_integer> big_integer::div_N_1(big_integer &v, big_integer const &d) {
    uint64_t carry = 0;
    u32 di = d.data_[0];
    for (size_t i = v.data_.size(); i != 0; --i) {
        auto p = static_cast<uint64_t>(v.data_[i - 1]) + carry * BASE_DIGIT;
        v.data_[i - 1] = split64(p / di).first;
        carry = p % di;
    }
    to_fit(v.data_);
    return {v, carry};
}
pair<big_integer, big_integer> big_integer::div_primal(big_integer &v, big_integer const &d) {
    uint128_t t1 = 0;
    uint128_t t2 = 0;
    for (size_t i = 0; i < v.data_.size(); i++) {
        t1 += static_cast<uint128_t>(v.data_[i]) << (BASE * i);
    }
    for (size_t i = 0; i < d.data_.size(); i++) {
        t2 += static_cast<uint128_t>(d.data_[i]) << (BASE * i);
    }
    uint128_t r = t1 / t2;
    uint128_t m = t1 - r * t2;
    big_integer res;
    big_integer mod;
    res.data_.resize(4);
    mod.data_.resize(4);
    for (u32 i = 0; i < 4; i++) {
        res.data_[i] = (r & (static_cast<uint128_t>(MAX_DIGIT) << (BASE * i))) >> (BASE * i);
        mod.data_[i] = (m & (static_cast<uint128_t>(MAX_DIGIT) << (BASE * i))) >> (BASE * i);
    }
    to_fit(res.data_);
    to_fit(mod.data_);
    return {res, mod};
}
pair<uint32_t, uint32_t> big_integer::div_3_2_primal(uint32_t u1, uint32_t u2, uint32_t u3, uint32_t d1, uint32_t d2) {
    uint128_t t1 = 0;
    uint128_t t2 = 0;
    t1 += u3;
    t1 += static_cast<uint128_t>(u2) << BASE;
    t1 += static_cast<uint128_t>(u1) << (BASE * 2);
    t2 += d2;
    t2 += static_cast<uint128_t>(d1) << BASE;
    uint128_t r = t1 / t2;
    uint128_t m = t1 - r * t2;
    u32 res = static_cast<u32>(r & MAX_DIGIT);
    u32 mod = static_cast<u32>(m & MAX_DIGIT);
    return {res, mod};
}


uint32_t big_integer::operator[](int i) {
    if (i < 0) {
        i = data_.size() + i;
    }
    if (i >= data_.size()) {
        return 0;
    } else {
        return data_[i];
    }
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}
