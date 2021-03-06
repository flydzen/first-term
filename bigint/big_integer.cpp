#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>
#define u32 uint32_t

big_integer::big_integer() : positive(true) {
    data_.push_back(0);
}

big_integer::big_integer(big_integer const &other) = default;

big_integer::big_integer(int a) {
    if (a == std::numeric_limits<int>::min())
        data_.push_back(static_cast<u32>(a));
    else
        data_.push_back(abs(a));
    positive = a >= 0;
}

big_integer::big_integer(std::string const &str) {
    big_integer base = 1;
    positive = true;
    u32 j = 0;
    if (str[0] == '-')
        j = 1;
    for (u32 i = str.size(); i != j; i--) {
        *this += base * (str[i - 1] - '0');
        base *= 10;
    }
    if (j == 1 && !(data_.size() == 1 && data_[0] == 0))
        positive = false;
}

big_integer::~big_integer() = default;

big_integer &big_integer::operator=(big_integer const &other) = default;

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (positive && !rhs.positive)
        *this -= -rhs;
    else if (!positive && rhs.positive)
        *this = rhs - (-*this);
    else
        sumABS(rhs);
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (positive && !rhs.positive)
        sumABS(rhs);
    else if (!positive && rhs.positive)
        sumABS(rhs);
    else if (positive && rhs.positive) {
        bool tp = *this >= rhs;
        subABS(rhs);
        positive = tp;
    } else if (!positive && !rhs.positive) {
        bool tp = *this >= rhs;
        subABS(rhs);
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
        if (over)
            result.data_[i + rhs.data_.size()] = over;
    }
    toFit(result.data_);
    result.positive = positive == rhs.positive;
    *this = result;
    return *this;

}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool sign = positive == rhs.positive;
    positive = true;
    data_ = div(*this, rhs).first.data_;
    positive = sign;
    if (data_.size() == 1 && data_[0] == 0) {
        positive = true;
    }
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    *this -= (*this / rhs) * rhs;
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    auto d1 = additionTo2(data_);
    auto d2 = rhs.additionTo2(rhs.data_);
    if (d1.size() < d2.size())
        d1.resize(d2.size(), highBit(d1) ? MAX_DIGIT : 0);
    if (d2.size() < d1.size())
        d2.resize(d1.size(), highBit(d2) ? MAX_DIGIT : 0);
    for (size_t i = 0; i < d1.size(); i++)
        d1[i] &= d2[i];
    bool high = highBit(d1);
    data_ = additionTo2(d1, true);
    positive = !high;
    toFit(data_);
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    auto d1 = additionTo2(data_);
    auto d2 = rhs.additionTo2(rhs.data_);
    if (d1.size() < d2.size())
        d1.resize(d2.size(), highBit(d1) ? MAX_DIGIT : 0);
    if (d2.size() < d1.size())
        d2.resize(d1.size(), highBit(d2) ? MAX_DIGIT : 0);
    for (size_t i = 0; i < d1.size(); i++)
        d1[i] |= d2[i];
    bool high = highBit(d1);
    data_ = additionTo2(d1, true);
    positive = !high;
    toFit(data_);
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    auto d1 = additionTo2(data_);
    auto d2 = rhs.additionTo2(rhs.data_);
    if (d1.size() < d2.size())
        d1.resize(d2.size(), highBit(d1) ? MAX_DIGIT : 0);
    if (d2.size() < d1.size())
        d2.resize(d1.size(), highBit(d2) ? MAX_DIGIT : 0);
    for (size_t i = 0; i < d1.size(); i++)
        d1[i] ^= d2[i];
    bool high = highBit(d1);
    data_ = additionTo2(d1, true);
    positive = !high;
    toFit(data_);
    return *this;
}

big_integer &big_integer::operator<<=(int rhs) {
    int in = rhs % BASE;
    int out = rhs / BASE;
    vector<u32> res(data_.size() + out + 1);
    for (size_t i = data_.size(); i != 0; i--) {
        auto p = split64(static_cast<uint64_t>(data_[i - 1]) << in);
        res[i + out - 1] |= p.first;
        res[i + out] |= p.second;
    }
    data_ = res;
    toFit(data_);
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    int in = rhs % BASE;
    int out = rhs / BASE;
    auto d = additionTo2(data_);
    vector<u32> res(d.size());
    for (int i = 0; i != (int) d.size(); i++) {
        auto p = split64(static_cast<uint64_t>(d[i]) << (BASE - in));
        if (i - out >= 0)
            res[i - out] |= p.second;
        if (i - out - 1 >= 0)
            res[i - out - 1] |= p.first;
    }
    if (!positive) {
        size_t i = res.size();
        for (; i != 0; i--) {
            if (res[i - 1] == 0)
                res[i - 1] = MAX_DIGIT;
            else
                break;
        }
        if (i) {
            for (u32 j = 1 << 31; j != 0; j >>= 1) {
                if ((res[i - 1] & j) != 0)
                    break;
                res[i - 1] |= j;
            }
        }
    }
    data_ = additionTo2(res, true);
    toFit(data_);
    if (data_.size() == 1 && data_[0] == 0)
        positive = true;
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer r(*this);
    if (r != 0)
        r.positive ^= true;
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
    if (a.positive != b.positive)
        return !a.positive && b.positive;
    if (a.data_.size() != b.data_.size())
        return (a.data_.size() < b.data_.size()) ^ !a.positive;
    for (size_t i = a.data_.size(); i > 0; --i)
        if (a.data_[i - 1] != b.data_[i - 1])
            return (a.data_[i - 1] < b.data_[i - 1]) ^ !a.positive;
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
    while (temp != 0) {
        string ad = to_string((temp % 1000000000).data_[0]);
        std::reverse(ad.begin(), ad.end());
        res += ad;
        res.insert(res.end(), 9 - ad.size(), '0');
        temp /= 1000000000;
    }
    for (; res.size() > 1 && res.back() == '0'; res.pop_back());
    if (neg)
        res += "-";
    std::reverse(res.begin(), res.end());
    if (res.empty())
        return "0";
    return res;
}
pair<uint32_t, uint32_t> big_integer::split64(uint64_t n) {
    return {n & 0xFFFFFFFF, n >> 32};
}

void big_integer::subABS(big_integer const &b) {
    bool loan = false;
    vector<u32> t1;
    vector<u32> t2;
    if ((*this < b) ^ !positive) {
        t1 = b.data_;
        t2 = (*this).data_;
    } else {
        t1 = (*this).data_;
        t2 = b.data_;
    }
    if (t2.size() < t1.size())
        t2.resize(t1.size());
    for (size_t i = 0; i < t1.size(); i++) {
        bool nLoan = t1[i] < t2[i];
        t1[i] -= (t2[i] + loan);
        loan = nLoan;
    }
    (*this).data_ = t1;
    toFit(data_);
}
void big_integer::sumABS(big_integer const &b) {
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
    if (over)
        data_.push_back(over);
}
void big_integer::toFit(vector<uint32_t> &v) {
    while (v.size() != 1 && v[v.size() - 1] == 0)
        v.pop_back();
}
vector<uint32_t> big_integer::additionTo2(vector<uint32_t> const &v, bool is2) const {
    big_integer temp;
    temp.data_ = v;
    bool high = highBit(temp.data_);
    if (!is2 && high)
        temp.data_.push_back(0);
    if (!is2 && positive)
        return temp.data_;
    if (is2 && !high)
        return v;
    for (size_t i = 0; i < temp.data_.size(); i++)
        temp.data_[i] = ~temp.data_[i];
    temp++;
    return temp.data_;
}
bool big_integer::highBit(vector<uint32_t> &v) {
    return (v.back() & (static_cast<u32>(1) << (BASE - 1)));
}
pair<big_integer, big_integer> big_integer::div(big_integer &v, big_integer const &d) {
    if (v.data_.size() < d.data_.size())
        return {0, v};
    else if (v == d)
        return {1, 0};
    else if (d.data_.size() == 1)
        return divN_1(v, d);
    else if (v.data_.size() <= 4 && d.data_.size() <= 4)
        return div_primal(v, d);
    else
        return divM_N(v, d);
}

pair<big_integer, big_integer> big_integer::divM_N(big_integer &v, big_integer const &d_) {
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
    vector<u32> res;
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
        if (v.get(-1) == d.get(-1) && v.get(-2) == d.get(- 2)) {
            res.push_back(0xFFFFFFFF);
        } else {
            auto p = div3_2_primal(v.get(-1),
                                   v.get(-2),
                                   v.get(-3),
                                   d.get(-1),
                                   d.get(-2));
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
    big_integer result;
    result.data_ = res;
    toFit(result.data_);
    return {result, v >> i};
}

pair<big_integer, big_integer> big_integer::divN_1(big_integer &v, big_integer const &d) {
    uint64_t carry = 0;
    u32 di = d.data_[0];
    for (ptrdiff_t i = v.data_.size() - 1; i >= 0; --i) {
        const auto dividend = static_cast<uint64_t>(v.data_[i]) + carry * BASE_DIGIT;
        v.data_[i] = split64(dividend / di).first;
        carry = dividend % di;
    }
    toFit(v.data_);
    return {v, carry};
}
pair<big_integer, big_integer> big_integer::div_primal(big_integer &v, big_integer const &d) {
    uint128_t t1 = 0;
    uint128_t t2 = 0;
    for (size_t i = 0; i < v.data_.size(); i++)
        t1 += static_cast<uint128_t>(v.data_[i]) << (BASE * i);
    for (size_t i = 0; i < d.data_.size(); i++)
        t2 += static_cast<uint128_t>(d.data_[i]) << (BASE * i);
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
    toFit(res.data_);
    toFit(mod.data_);
    return {res, mod};
}
pair<uint32_t, uint32_t> big_integer::div3_2_primal(uint32_t u1, uint32_t u2, uint32_t u3, uint32_t d1, uint32_t d2) {
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
uint32_t big_integer::get(int i) {
    if (i < 0)
        i = data_.size() + i;
    if (i >= data_.size())
        return 0;
    else
        return data_[i];
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}
