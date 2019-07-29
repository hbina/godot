/*************************************************************************/
/*  vector.h                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef VECTOR_H
#define VECTOR_H

/**
 * @class VectorImpl
 * @author Juan Linietsky
 * VectorImpl container. Regular VectorImpl Container. Use with care and for smaller arrays when possible. Use PoolVector for large arrays.
*/

#include "core/error_list.h"
#include "core/typedefs.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

template <class T>
class VectorImpl {

	std::vector<T> internal_vector;

public:
	void remove(int);
	void erase(const T &);
	void invert();
	void resize(int);
	int size() const noexcept;
	T *ptrw() noexcept;
	const T *ptr() const noexcept;
	void insert(int, const T &);
	int find(const T &, int p_from = 0) const;
	void append_array(const VectorImpl<T> &);
	void sort();
	template <class C>
	void sort_custom();
	void ordered_insert(const T &);

	// STL interfaces
	T &operator[](int) noexcept;
	const T &operator[](int) const noexcept;
	T &get(int) noexcept;
	const T &get(int) const noexcept;
	void set(int, const T &) noexcept;
	bool empty() const noexcept;

	void push_back(const T &);
	void push_back(const T &&);

	void clear() noexcept;

	T *data() noexcept;
	const T *data() const noexcept;
	typename std::vector<T>::iterator begin() noexcept;
	typename std::vector<T>::iterator end() noexcept;
	typename std::vector<T>::const_iterator begin() const noexcept;
	typename std::vector<T>::const_iterator end() const noexcept;

	// Hanif's additions
	const T &back() const noexcept;
};

template <typename T>
T *VectorImpl<T>::data() noexcept {
	return internal_vector.data();
}

template <typename T>
typename std::vector<T>::iterator VectorImpl<T>::begin() noexcept {
	return internal_vector.begin();
}

template <typename T>
typename std::vector<T>::iterator VectorImpl<T>::end() noexcept {
	return internal_vector.end();
}

template <typename T>
typename std::vector<T>::const_iterator VectorImpl<T>::begin() const noexcept {
	return internal_vector.begin();
}

template <typename T>
typename std::vector<T>::const_iterator VectorImpl<T>::end() const noexcept {
	return internal_vector.end();
}

template <typename T>
const T *VectorImpl<T>::data() const noexcept {
	return internal_vector.data();
}

template <typename T>
void VectorImpl<T>::clear() noexcept {
	internal_vector.clear();
}

template <typename T>
void VectorImpl<T>::remove(int p_index) {
	internal_vector.erase(internal_vector.begin() + p_index);
}

template <typename T>
void VectorImpl<T>::push_back(const T &p_val) {
	internal_vector.push_back(p_val);
}

template <typename T>
void VectorImpl<T>::push_back(const T &&p_val) {
	internal_vector.push_back(p_val);
}

template <typename T>
bool VectorImpl<T>::empty() const noexcept {
	return internal_vector.empty();
}

template <typename T>
void VectorImpl<T>::resize(int p_size) {
	internal_vector.resize(p_size);
}

template <typename T>
int VectorImpl<T>::size() const noexcept {
	assert(((std::size_t)std::numeric_limits<int>::max()) > internal_vector.size());
	return internal_vector.size();
}

template <typename T>
void VectorImpl<T>::erase(const T &p_val) {
	int index = find(p_val);
	if (index >= 0) {
		remove(index);
	}
}

template <typename T>
T *VectorImpl<T>::ptrw() noexcept {
	return internal_vector.data();
}

template <typename T>
const T *VectorImpl<T>::ptr() const noexcept {
	return internal_vector.data();
}

template <typename T>
T &VectorImpl<T>::operator[](int p_index) noexcept {
	return internal_vector.operator[](p_index);
}

template <typename T>
const T &VectorImpl<T>::operator[](int p_index) const noexcept {
	return internal_vector.operator[](p_index);
}

template <typename T>
T &VectorImpl<T>::get(int p_index) noexcept {
	return internal_vector.operator[](p_index);
}

template <typename T>
const T &VectorImpl<T>::get(int p_index) const noexcept {
	return internal_vector.operator[](p_index);
}

template <typename T>
void VectorImpl<T>::set(int p_index, const T &p_value) noexcept {
	internal_vector[p_index] = p_value;
}

template <typename T>
void VectorImpl<T>::insert(int p_pos, const T &p_val) {
	internal_vector.insert(internal_vector.begin() + p_pos, p_val);
}

template <typename T>
int VectorImpl<T>::find(const T &p_val, int p_from) const {
	for (int a = 0; a < static_cast<int>(size()); a++) {
		if (p_val == internal_vector.operator[](a)) {
			return a;
		}
	}
	return -1;
}

template <typename T>
void VectorImpl<T>::sort() {
	std::sort(internal_vector.begin(), internal_vector.end());
}

template <typename T>
template <typename C>
void VectorImpl<T>::sort_custom() {
	C comparator;
	std::sort(internal_vector.begin(), internal_vector.end(), comparator);
}

template <typename T>
void VectorImpl<T>::ordered_insert(const T &p_val) {
	int i = 0;
	for (; i < size(); i++) {

		if (p_val < internal_vector.operator[](i)) {
			break;
		};
	};
	insert(i, p_val);
}

template <class T>
void VectorImpl<T>::invert() {
	std::reverse(internal_vector.begin(), internal_vector.end());
}

template <class T>
void VectorImpl<T>::append_array(const VectorImpl<T> &p_other) {
	for (const T &a : p_other) {
		internal_vector.push_back(a);
	}
}

template <typename T>
const T &VectorImpl<T>::back() const noexcept {
	return internal_vector.operator[](size());
}

template <typename T>
class Vector : public VectorImpl<T> {};

template <>
class Vector<bool> : public VectorImpl<char> {};

#endif
