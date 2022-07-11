/*
 * DataBuffer.h
 *
 *  Created on: 8 lug 2022
 *      Author: paolo
 */

#pragma once

#include <stdlib.h>
#include <string.h>

template<typename T>
class DataBuffer {
public:
	DataBuffer() = delete;
	DataBuffer(size_t size) :
			buffer(new T[size]), _size(size) {
	}
	DataBuffer(const T *data, size_t size) :
			buffer(new T[size]), _size(size) {
		memcpy(buffer, data, sizeof(T) * _size);
	}
	DataBuffer(const DataBuffer<T> &rhs) :
			buffer(new T[rhs._size]), _size(rhs._size) {
		memcpy(buffer, rhs.buffer, sizeof(T) * _size);
	}
	DataBuffer(DataBuffer<T> &&rhs) :
			buffer(rhs.buffer), _size(rhs._size) {
		rhs.buffer = nullptr;
		rhs._size = 0;
	}
	~DataBuffer() {
		delete[] buffer;
	}
	T* get() {
		return buffer;
	}
	const T* get() const {
		return buffer;
	}
	T& operator[](size_t index) {
		return buffer[index];
	}
	size_t size() const {
		return _size;
	}

private:
	T *buffer;
	size_t _size;
};
