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

#include <algorithm>
#include <type_traits>
#include <vector>

template <class T>
class VectorImpl : public std::vector<T> {

public:
	void remove(int);
	void erase(const T &);
	void invert();
	T *ptrw();
	const T *ptr() const;
	T get(int);
	const T get(int) const;
	void set(int, const T &);
	void insert(int, const T &);
	int find(const T &, int p_from = 0) const;
	void append_array(const VectorImpl<T> &);
	void sort();
	template <class C>
	void sort_custom();
	void ordered_insert(const T &);
};

template <typename T>
void VectorImpl<T>::remove(int p_index) {
	std::vector<T>::erase(std::vector<T>::begin() + p_index);
}

template <typename T>
void VectorImpl<T>::erase(const T &p_val) {
	int index = find(p_val);
	if (index >= 0) {
		remove(index);
	}
}

template <typename T>
T *VectorImpl<T>::ptrw() {
	return std::vector<T>::data();
}

template <typename T>
const T *VectorImpl<T>::ptr() const {
	return std::vector<T>::data();
}

template <typename T>
T VectorImpl<T>::get(int p_index) {
	return std::vector<T>::operator[](p_index);
}

template <typename T>
const T VectorImpl<T>::get(int p_index) const {
	return std::vector<T>::operator[](p_index);
}

template <typename T>
void VectorImpl<T>::set(int p_index, const T &p_elem) {
	std::vector<T>::operator[](p_index) = p_elem;
}

template <typename T>
void VectorImpl<T>::insert(int p_pos, const T &p_val) {
	std::vector<T>::insert(std::vector<T>::begin() + p_pos, p_val);
}

template <typename T>
int VectorImpl<T>::find(const T &p_val, int p_from) const {
	for (int a = 0; a < static_cast<int>(std::vector<T>::size()); a++) {
		if (p_val == std::vector<T>::operator[](a)) {
			return a;
		}
	}
	return -1;
}

template <typename T>
void VectorImpl<T>::sort() {
	std::sort(std::vector<T>::begin(), std::vector<T>::end());
}

template <typename T>
template <typename C>
void VectorImpl<T>::sort_custom() {
	C comparator;
	std::sort(std::vector<T>::begin(), std::vector<T>::end(), comparator);
}

template <typename T>
void VectorImpl<T>::ordered_insert(const T &p_val) {
	int i = 0;
	for (; i < std::vector<T>::size(); i++) {

		if (p_val < std::vector<T>::operator[](i)) {
			break;
		};
	};
	insert(i, p_val);
}

template <class T>
void VectorImpl<T>::invert() {
	std::reverse(std::vector<T>::begin(), std::vector<T>::end());
}

template <class T>
void VectorImpl<T>::append_array(const VectorImpl<T> &p_other) {
	for (const T &a : p_other) {
		std::vector<T>::push_back(a);
	}
}

template <typename T>
class Vector : public VectorImpl<T> {};

template <>
class Vector<bool> : public VectorImpl<char> {};

#endif
