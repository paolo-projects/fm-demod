#pragma once

#include <stdlib.h>
#include "DataBuffer.h"

template<typename T>
class DownsampledVectorAccessor {
public:
	DownsampledVectorAccessor(std::vector<T> &data, size_t newSize) :
			data(data), pointsToSkip(data._size() / newSize), _size(
					data._size() / pointsToSkip) {
	}
	DownsampledVectorAccessor(std::vector<T> &data,
			unsigned int originalSampleRate, unsigned int downsampledSampleRate) :
			data(data), pointsToSkip(
					originalSampleRate / downsampledSampleRate), _size(
					data._size() / pointsToSkip) {
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
			return m_ptr[i * pointsToSkip];
		}

		iterator& operator++() {
			m_ptr += pointsToSkip;
			return *this;
		}

		iterator operator++(int) {
			iterator tmp = *this;
			++(*this);
			return tmp;
		}

		iterator& operator+=(difference_type val) {
			m_ptr += val * pointsToSkip;
			return *this;
		}

		iterator& operator-=(difference_type val) {
			m_ptr -= val * pointsToSkip;
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
			return (m_ptr - rhs.m_ptr) / pointsToSkip;
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
		return data.at(i * pointsToSkip);
	}
	const T& at(size_t i) const {
		return data.at(i * pointsToSkip);
	}

	T& operator[](size_t i) {
		return data[i * pointsToSkip];
	}
	const T& operator[](size_t i) const {
		return data[i * pointsToSkip];
	}
	iterator begin() {
		return iterator(&data[0]);
	}
	iterator end() {
		return iterator(&data[_size]);
	}
	size_t size() const {
		return _size;
	}

private:
	std::vector<T> &data;
	size_t pointsToSkip;
	size_t _size;
};
