#pragma once

#include <stdlib.h>
#include <cmath>
#include "DataBuffer.h"

/**
 * @brief Class that wraps a `DataBuffer` object for accessing a lower number of points (downsampling)
 * This implementation interpolates every point linearly
 * @tparam T The data type inside the buffer
 */
template<typename T>
class InterpolatedBufferAccessor {
public:

    InterpolatedBufferAccessor(DataBuffer<T> &data, size_t newSize) :
    data(data), originalSize(data.size()), _size(newSize), factor((double) originalSize / _size), 
            invFactor((double) _size / originalSize) {
    }

    InterpolatedBufferAccessor(DataBuffer<T> &data,
            unsigned int originalSampleRate, unsigned int downsampledSampleRate) :
    data(data), originalSize(data.size()), _size(
    std::round(downsampledSampleRate * (double) data.size() / originalSampleRate)), 
            factor((double) originalSize / _size), 
            invFactor((double) _size / originalSize) {
    }

    struct iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;

        iterator(pointer ptr) :
        m_ptr(ptr) {

        }

        reference operator*() const {
            return *m_ptr;
        }

        pointer operator->() {
            return m_ptr;
        }

        reference operator[](difference_type i) const {
            return m_ptr[getOriginalIndex(i)];
        }

        iterator& operator++() {
            m_ptr += (size_t) std::round((double) originalSize / _size);
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator+=(difference_type val) {
            m_ptr += (size_t)
                    std::round(val * (double) originalSize / _size);
            return *this;
        }

        iterator& operator-=(difference_type val) {
            m_ptr -= val * (size_t)
                    std::round(val * (double) originalSize / _size);
            return *this;
        }

        iterator operator+(difference_type val) {
            iterator result(m_ptr);
            result += val;
            return result;
        }

        iterator operator-(difference_type val) {
            iterator result(m_ptr);
            result -= val;
            return result;
        }

        difference_type operator-(const iterator &rhs) {
            return (difference_type) std::round((m_ptr - rhs.m_ptr) /
                    ((double) originalSize / _size));
        }

        friend bool operator==(const iterator &a, const iterator &b) {
            return a.m_ptr == b.m_ptr;
        }

        friend bool operator!=(const iterator &a, const iterator &b) {
            return a.m_ptr != b.m_ptr;
        }

        friend bool operator>(const iterator &a, const iterator &b) {
            return a.m_ptr > b.m_ptr;
        }

        friend bool operator<(const iterator &a, const iterator &b) {
            return a.m_ptr < b.m_ptr;
        }

        friend bool operator<=(const iterator &a, const iterator &b) {
            return a.m_ptr <= b.m_ptr;
        }

        friend bool operator>=(const iterator &a, const iterator &b) {
            return a.m_ptr >= b.m_ptr;
        }

    private:
        pointer m_ptr;
    };

    T& at(size_t i) {
        return data.at(getOriginalIndex(i));
    }

    const T& at(size_t i) const {
        return data.at(getOriginalIndex(i));
    }

    T& operator[](size_t i) {
        return data[getOriginalIndex(i)];
    }

    const T& operator[](size_t i) const {
        return data[getOriginalIndex(i)];
    }

    iterator begin() {
        return iterator(&data[0]);
    }

    iterator end() {
        return iterator(&data[originalSize]);
    }

    size_t size() const {
        return _size;
    }

private:
    DataBuffer<T> &data;
    size_t originalSize;
    size_t _size;
    double factor, invFactor;

    size_t getInterpolatedIndex(size_t originalIndex) {
        return size_t(std::round(originalIndex * invFactor));
    }

    size_t getOriginalIndex(size_t interpolatedIndex) {
        return size_t(std::round(factor * interpolatedIndex));
    }
};
