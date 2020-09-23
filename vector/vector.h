#ifndef VECTOR_H
#define VECTOR_H

#ifdef _GLIBCXX_DEBUG
#define debug_assert(expression) assert((expression))
#else
#define debug_assert(expression)
#endif

#include <cstddef>
#include <cstring>
#include <cassert>
#include <utility>
#include <algorithm>

template<typename T>
struct vector {
  typedef T *iterator;
  typedef T const *const_iterator;

  // O(1) nothrow
  vector();
  // O(N) strong
  vector(vector const &other);
  // O(N) strong
  vector &operator=(vector const &other);

  // O(N) nothrow
  ~vector();

  // O(1) nothrow
  T &operator[](size_t index);
  // O(1) nothrow
  T const &operator[](size_t index) const;

  // O(1) nothrow
  T *data();
  // O(1) nothrow
  T const *data() const;
  // O(1) nothrow
  size_t size() const;

  // O(1) nothrow
  T &front();
  // O(1) nothrow
  T const &front() const;

  // O(1) nothrow
  T &back();
  // O(1) nothrow
  T const &back() const;
  // O(1)* strong
  void push_back(T const &value);
  // O(1) nothrow
  void pop_back();

  // O(1) nothrow
  bool empty() const;

  // O(1) nothrow
  size_t capacity() const;
  // O(N) strong
  void reserve(size_t);
  // O(N) strong
  void shrink_to_fit();

  // O(N) nothrow
  void clear();

  // O(1) nothrow
  void swap(vector &other);

  // O(1) nothrow
  iterator begin();
  // O(1) nothrow
  iterator end();

  // O(1) nothrow
  const_iterator begin() const;
  // O(1) nothrow
  const_iterator end() const;

  // O(N) weak
  iterator insert(const_iterator pos, T const &value);

  // O(N) weak
  iterator erase(const_iterator pos);

  // O(N) weak
  iterator erase(const_iterator first, const_iterator last);

 private:
  void change_capacity(size_t new_capacity);
  static T *copy_data(T *source, size_t from, size_t count, size_t dest_size);
  static void clear_data_buffer(T *data_buffer, size_t from, size_t count);

  T *data_;
  size_t size_;
  size_t capacity_;
};

template<typename T>
T *vector<T>::copy_data(T *source, size_t from, size_t count, size_t dest_size) {
  T *destination_ = dest_size == 0 ? nullptr : static_cast<T *>(operator new(dest_size * sizeof(T)));
  size_t filled_size_ = 0;

  try {
    for (; filled_size_ < count; filled_size_++) {
      new(destination_ + filled_size_) T(source[from + filled_size_]);
    }
  } catch (...) {
    clear_data_buffer(destination_, 0, filled_size_);
    operator delete(destination_);
    throw;
  }

  return destination_;
}

template<typename T>
void vector<T>::clear_data_buffer(T *data_buffer, size_t from, size_t count) {
  while (count-- > 0) {
    data_buffer[from + count].~T();
  }
}

template<typename T>
vector<T>::vector()
  : data_(nullptr), size_(0), capacity_(0) {}

template<typename T>
vector<T>::vector(vector<T> const &rhs) {
  T *data_buffer_ = copy_data(rhs.data_, 0, rhs.size_, rhs.size_);
  data_ = data_buffer_;
  size_ = rhs.size_;
  capacity_ = rhs.size_;
}

template<typename T>
vector<T> &vector<T>::operator=(vector<T> const &other) {
  vector<T> temp(other);
  swap(temp);
  return *this;
}

template<typename T>
vector<T>::~vector() {
  clear();
  operator delete(data_);
  data_ = nullptr;
}

template<typename T>
T &vector<T>::operator[](size_t index) {
  debug_assert(index < size_);
  return data_[index];
}

template<typename T>
T const &vector<T>::operator[](size_t index) const {
  debug_assert(index < size_);
  return data_[index];
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
T &vector<T>::front() {
  debug_assert(size_ > 0);
  return *data_;
}

template<typename T>
T const &vector<T>::front() const {
  debug_assert(size_ > 0);
  return *data_;
}

template<typename T>
T &vector<T>::back() {
  debug_assert(size_ > 0);
  return data_[size_ - 1];
}

template<typename T>
T const &vector<T>::back() const {
  debug_assert(size_ > 0);
  return data_[size_ - 1];
}

template<typename T>
void vector<T>::push_back(T const &value) {
  if (size_ == capacity_) {
    T push_value(value);
    reserve(capacity_ > 0 ? capacity_ * 2 : 1);
    new(data_ + size_) T(push_value);
  } else {
    new(data_ + size_) T(value);
  }

  size_++;
}

template<typename T>
void vector<T>::pop_back() {
  debug_assert(size_ > 0);
  data_[--size_].~T();
}

template<typename T>
bool vector<T>::empty() const {
  return size_ == 0;
}

template<typename T>
size_t vector<T>::capacity() const {
  return capacity_;
}

template<typename T>
void vector<T>::change_capacity(size_t new_capacity) {
  debug_assert(size_ <= new_capacity);
  T *data_buffer_ = copy_data(data_, 0u, std::min(size_, new_capacity), new_capacity);
  clear_data_buffer(data_, 0u, size_);
  operator delete(data_);
  data_ = data_buffer_;
  capacity_ = new_capacity;
}

template<typename T>
void vector<T>::reserve(size_t size) {
  if (size > capacity_) {
    change_capacity(size);
  }
}

template<typename T>
void vector<T>::shrink_to_fit() {
  if (capacity_ != size_) {
    if (size_ == 0) {
      operator delete(data_);
      data_ = nullptr;
      capacity_ = size_;
    } else {
      change_capacity(size_);
    }
  }
}

template<typename T>
void vector<T>::clear() {
  clear_data_buffer(data_, 0, size_);
  size_ = 0;
}

template<typename T>
void vector<T>::swap(vector<T> &other) {
  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

template<typename T>
typename vector<T>::iterator vector<T>::begin() {
  return data_;
}
template<typename T>
typename vector<T>::iterator vector<T>::end() {
  return data_ + size_;
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
typename vector<T>::iterator vector<T>::insert(const_iterator pos, T const &value) {
  debug_assert(begin() <= pos && pos <= end());
  ptrdiff_t pos_ = pos - data_;
  push_back(value);

  for (ptrdiff_t i = size_ - 1; i != pos_; --i) {
    std::swap(data_[i], data_[i - 1]);
  }

  return begin() + pos_;
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
  return erase(pos, pos + 1);
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last) {
  debug_assert(begin() <= first && first < last && last <= end());
  ptrdiff_t begin_ = first - data_;
  ptrdiff_t end_ = last - data_;
  ptrdiff_t delta_ = end_ - begin_;

  for (size_t i = end_; i < size_; i++) {
    std::swap(data_[i - delta_], data_[i]);
  }

  clear_data_buffer(data_, size_ - delta_, delta_);
  size_ -= delta_;
  return begin() + begin_;
}
#endif // VECTOR_H
