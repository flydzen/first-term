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

     iterator insert(iterator pos, T const &);
     iterator insert(const_iterator pos, T const &);

     iterator erase(iterator pos);
     iterator erase(const_iterator pos);
     iterator erase(iterator first, iterator last);
     iterator erase(const_iterator first, const_iterator last);
 private:
     T *data_;
     size_t size_;
     size_t capacity_;
     T *genCapacity(size_t);
     void saveSetCapacity(size_t);
     void doCopy(T *to, T const *from, size_t size);
     void clear(T *mas, size_t size);
};

template<typename T>
T *vector<T>::genCapacity(size_t newCap) {
    if (newCap == 0)
        return nullptr;
    T *temp = static_cast<T *>(operator new(newCap * sizeof(T)));
    return temp;
}

template<typename T>
void vector<T>::saveSetCapacity(size_t newCap) {
    if (size_ > newCap || capacity_ == newCap)
        return;
    T *temp = genCapacity(newCap);
    doCopy(temp, data_, size_);
    std::swap(temp, data_);
    clear(temp, size_);
    operator delete(temp);
    capacity_ = newCap;
}

template<typename T>
vector<T>::vector() noexcept : data_(nullptr), size_(0), capacity_(0) {}

template<typename T>
vector<T>::vector(std::size_t cap_size) :
    data_(genCapacity(cap_size)),
    size_(0),
    capacity_(cap_size) {
}

template<typename T>
void vector<T>::swap(vector &other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}
template<typename T>
vector<T>::vector(vector const &other) : vector() {
    T* temp = genCapacity(other.size_);
    doCopy(temp, other.data_, other.size_);
    std::swap(data_, temp);
    operator delete(temp);
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
        throw std::runtime_error("reallocation_throw");
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
    while (size_ > 0)
        pop_back();
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
    return data_ ? data_ : nullptr;
}
template<typename T>
T const *vector<T>::data() const {
    return data_ ? data_ : nullptr;
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
    T valueCopy = value;
    if (size_ == capacity_)
        saveSetCapacity(size_ * 2 + 1);
    new(data_ + size_) T(valueCopy);
    ++size_;
}
template<typename T>
void vector<T>::pop_back() {
    if (empty())
        throw std::runtime_error("delete from empty vector");
    data_[--size_].~T();
}
template<typename T>
void vector<T>::reserve(size_t new_cap) {
    if (capacity_ < new_cap) {
        saveSetCapacity(new_cap);
    }
}
template<typename T>
void vector<T>::shrink_to_fit() {
    saveSetCapacity(size_);
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
typename vector<T>::iterator vector<T>::insert(vector::iterator pos, const T &value) {
    T newValue = value;
    ptrdiff_t ind = pos - begin();
    if (ind == size_){
        push_back(newValue);
        return &data_[size_-1];
    }
    size_t nCap = size_ == capacity_ ? capacity_ * 2 : capacity_;
    vector temp(nCap);
    for (size_t i = 0; i < ind; i++)
        temp.push_back(data_[i]);
    temp.push_back(value);
    for (size_t i = ind; i < size_; i++)
        temp.push_back(data_[i]);
    swap(temp);
    return &data_[ind];
}
template<typename T>
typename vector<T>::iterator vector<T>::insert(vector::const_iterator pos, const T &value) {
    return insert(pos, value);
}
template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::iterator pos) {
    return erase(pos, pos + 1);
}
template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator pos) {
    return erase(pos);
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
typename vector<T>::iterator vector<T>::erase(vector::iterator first, vector::iterator last) {
    ptrdiff_t bgn = first - begin();
    ptrdiff_t nd = last - begin();
    T* temp = genCapacity(capacity_);
    doCopy(temp, data_, bgn);
    doCopy(temp + bgn, data_ + nd, (size_t) (end()-last));
    std::swap(temp, data_);
    clear(temp, size_);
    size_ -= nd - bgn;
    operator delete(temp);
    return data_ + bgn;
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator first, vector::const_iterator last) {
    return erase(first, last);
}

#endif // VECTOR_H