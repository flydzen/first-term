//
// Created by flydzen on 06.07.2020.
//

#include <algorithm>
#include "container.h"
container::container() : is_small(true), empty(true) {
    data.small = 0;
}
container::~container() {
    if (!is_small)
        delete data.big;
}
container::container(container const &other) : is_small(other.is_small), empty(other.empty) {
    if (is_small)
        data.small = other.data.small;
    else {
        data.big = new wrapper;
        data.big->v = other.data.big->v;
    }
}
container::container(size_t i) : container() {
    resize(i);
}
void container::push_back(u32 v) {
    if (empty) {
        data.small = v;
        empty = false;
    } else if (is_small) {
        auto first = data.small;
        data.big = new wrapper;
        data.big->v.push_back(first);
        data.big->v.push_back(v);
        is_small = false;
    } else {
        data.big->v.push_back(v);
    }
}
void container::pop_back() {
    ////////////////////////////////////////////// добавь для is_small
    data.big->v.pop_back();
    if (data.big->v.size() == 1) {
        u32 value = data.big->v[0];
        delete data.big;
        data.small = value;
        is_small = true;
    }
}
u32 &container::operator[](size_t ind) {
    if (is_small)
        return data.small;
    else
        return data.big->v[ind];
}
u32 const &container::operator[](size_t ind) const {
    if (is_small)
        return data.small;
    else
        return data.big->v[ind];
}
u32 &container::back() {
    if (is_small)
        return data.small;
    else
        return data.big->v.back();
}
size_t container::size() const {
    if (is_small)
        return empty ? 0 : 1;
    else
        return data.big->v.size();
}
void container::resize(size_t sz) {
    resize(sz, 0);
}
void container::resize(size_t sz, u32 v) {
    if (sz == 1){
        data.small = v;
        is_small = true;
    }else {
        if (is_small) {
            u32 val = data.small;
            data.big = new wrapper;
            if (!empty)
                data.big->v.push_back(val);
        }
        data.big->v.resize(sz, v);
        is_small = false;
    }
    empty = false;
}
void container::reverse() {
    if (!is_small)
        std::reverse(data.big->v.begin(), data.big->v.end());
}
container &container::operator=(container const &other) {
    empty = other.empty;
    if (other.is_small) {
        if (!is_small)
            delete data.big;
        data.small = other.data.small;
    }else {
        if (is_small)
            data.big = new wrapper;
        data.big->v = other.data.big->v;
    }
    is_small = other.is_small;
    return *this;
}

bool operator==(container const &a, container const &b) {
    if (a.empty && b.empty)
        return true;
    if (a.is_small == b.is_small) {
        if (a.is_small && b.is_small)
            return a.data.small == b.data.small;
        else
            return a.data.big->v == b.data.big->v;
    }
    return false;
}