#ifndef VECTOR_H
#define VECTOR_H

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <memory>

template<typename T>
struct vector {
 public:
     typedef T *iterator;
     typedef T const *const_iterator;

     vector() noexcept;
     vector(std::size_t);
     vector(vector const &);
     vector &operator=(vector const &other);

     ~vector();

     T &operator[](size_t i);
     T const &operator[](size_t i) const;

     T *data();
     T const *data() const;
     size_t size() const;

     T &front();
     T const &front() const;
     T &back();
     T const &back() const;
     void push_back(T const &);
     void pop_back();

     bool empty() const;

     size_t capacity() const;
     void reserve(size_t);
     void shrink_to_fit();
     void clear();
     void swap(vector &);

     iterator begin();
     iterator end();
     const_iterator begin() const;
     const_iterator end() const;

     iterator insert(const_iterator pos, T const &);

     iterator erase(const_iterator pos);
     iterator erase(const_iterator first, const_iterator last);
 private:
     T *data_;
     size_t size_;
     size_t capacity_;
     T *allocMemory(size_t);
     void setCapacity(size_t);
     static void doCopy(T *to, T const *from, size_t size);
     static void clear(T *mas, size_t size);
};

template<typename T>
T *vector<T>::allocMemory(size_t newCap) {
    if (newCap == 0)
        return nullptr;
    T *temp = static_cast<T *>(operator new(newCap * sizeof(T)));
    return temp;
}

template<typename T>
void vector<T>::setCapacity(size_t newCap) {
    if (capacity_ == newCap)
        return;
    T *temp = allocMemory(newCap);
    doCopy(temp, data_, size_);
    clear(data_, size_);
    operator delete(data_);
    data_ = temp;
    capacity_ = newCap;
}

template<typename T>
vector<T>::vector() noexcept : data_(nullptr), size_(0), capacity_(0) {}

template<typename T>
vector<T>::vector(std::size_t cap_size) :
    data_(allocMemory(cap_size)),
    size_(0),
    capacity_(cap_size) {
}

template<typename T>
void vector<T>::swap(vector &other) {
    using std::swap;
    swap(data_, other.data_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
}
template<typename T>
vector<T>::vector(vector const &other) : vector() {
    T *temp = allocMemory(other.size_);
    doCopy(temp, other.data_, other.size_);
    data_ = temp;
    size_ = other.size_;
    capacity_ = size_;
}

template<typename T>
void vector<T>::doCopy(T *to, T const *from, size_t size) {
    size_t i = 0;
    try {
        for (; i < size; i++) {
            new(to + i) T(from[i]);
        }
    } catch (...) {
        clear(to, i);
        operator delete(to);
        throw;
    }
}

template<typename T>
void vector<T>::clear(T *mas, size_t size) {
    for (size_t i = size; i > 0; i--) {
        mas[i - 1].~T();
    }
}
template<typename T>
vector<T> &vector<T>::operator=(vector const &other) {
    vector temp(other);
    swap(temp);
    return *this;
}
template<typename T>
vector<T>::~vector() {
    clear();
    operator delete(data_);
}
template<typename T>
void vector<T>::clear() {
    clear(data_, size_);
    size_ = 0;
}
template<typename T>
T &vector<T>::operator[](size_t i) {
    return data_[i];
}
template<typename T>
T const &vector<T>::operator[](size_t i) const {
    return data_[i];
}
template<typename T>
T *vector<T>::data() {
    return data_;
}
template<typename T>
T const *vector<T>::data() const {
    return data_;
}
template<typename T>
size_t vector<T>::size() const {
    return size_;
}
template<typename T>
T &vector<T>::back() {
    return data_[size_ - 1];
}
template<typename T>
T const &vector<T>::back() const {
    return data_[size_ - 1];
}
template<typename T>
typename vector<T>::iterator vector<T>::begin() {
    return data_;
}
template<typename T>
bool vector<T>::empty() const {
    return size_ == 0;
}
template<typename T>
typename vector<T>::iterator vector<T>::end() {
    return data_ + size_;
}
template<typename T>
size_t vector<T>::capacity() const {
    return capacity_;
}
template<typename T>
void vector<T>::push_back(const T &value) {
    if (size_ == capacity_) {
        T valueCopy = value;
        setCapacity(size_ * 2 + 1);
        new(data_ + size_) T(valueCopy);
    } else {
        new(data_ + size_) T(value);
    }
    ++size_;
}
template<typename T>
void vector<T>::pop_back() {
    if (empty())
        throw;
    data_[--size_].~T();
}
template<typename T>
void vector<T>::reserve(size_t new_cap) {
    if (capacity_ < new_cap)
        setCapacity(new_cap);
}
template<typename T>
void vector<T>::shrink_to_fit() {
    setCapacity(size_);
    capacity_ = size_;
}
template<typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
    return data_;
}
template<typename T>
typename vector<T>::const_iterator vector<T>::end() const {
    return data_ + size_;
}

template<typename T>
typename vector<T>::iterator vector<T>::insert(vector::const_iterator pos, const T &value) {
    using std::swap;
    ptrdiff_t ind = pos - begin();
    push_back(value);
    for (size_t i = size_-1; i > ind; i--)
        swap(data_[i], data_[i-1]);
    return &data_[ind];
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator pos) {
    return erase(pos, pos+1);
}
template<typename T>
T &vector<T>::front() {
    return data_[0];
}
template<typename T>
T const &vector<T>::front() const {
    return data_[0];
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator first, vector::const_iterator last) {
    using std::swap;
    ptrdiff_t bgn = first - begin();
    ptrdiff_t range = last-first;
    if (range <= 0)
        return data_ + bgn;
    for (size_t i = bgn; i != size_-range; i++)
        swap(data_[i], data_[i+range]);
    for (size_t i = 0; i != range; i++)
        pop_back();
    return data_ + bgn;
}

#endif // VECTOR_H