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
 * @class Vector
 * @author Juan Linietsky
 * Vector container. Regular Vector Container. Use with care and for smaller arrays when possible. Use PoolVector for large arrays.
*/

#include "core/error_macros.h"
#include "core/os/memory.h"
#include "core/sort_array.h"

#include <algorithm>
#include <vector>

template <class T>
class Vector : public std::vector<std::conditional_t<std::is_same_v<T, bool>, char, T> > {

public:
	void remove(int p_index) {
		write.erase(write.begin() + p_index);
	}
	void erase(const T &p_val) {
		int idx = find(p_val);
		if (idx >= 0) remove(idx);
	};
	void invert();

	T *ptrw() { return write.data(); }
	const T *ptr() const { return write.data(); }

	void clear() { write.clear(); }
	bool empty() const { return write.empty(); }

	T get(int p_index) { return write[p_index]; }
	const T get(int p_index) const { return write[p_index]; }
	void set(unsigned int p_index, const T &p_elem) {
		CRASH_BAD_INDEX(p_index, write.size());
		write[p_index] = p_elem;
	}
	int size() const { return write.size(); }
	void resize(int p_size) {
		write.resize(p_size);
	}
	const T &operator[](int p_index) const {
		return write[p_index];
	}

	void insert(unsigned int p_pos, const T &p_val) {
		write.insert(write.begin() + p_pos, p_val);
	}

	int find(const T &p_val, int p_from = 0) const {
		for (signed int a = 0; a < static_cast<signed int>(write.size()); a++) {
			if (p_val == write[a]) {
				return a;
			}
		}
		return -1;
	}

	void append_array(const Vector<T> &p_other);

	void sort() {
		std::sort(write.begin(), write.end());
	}

	template <class C>
	void sort_custom() {
		C comparator;
		std::sort(write.begin(), write.end(), comparator);
	}

	void ordered_insert(const T &p_val) {
		unsigned int i = 0u;
		for (; i < write.size(); i++) {

			if (p_val < write[i]) {
				break;
			};
		};
		insert(i, p_val);
	}
};

template <class T>
void Vector<T>::invert() {
	std::reverse(std::begin(write), std::end(write));
}

template <class T>
void Vector<T>::append_array(const Vector<T> &p_other) {
	for (const T &a : p_other.write) {
		write.push_back(a);
	}
}

#endif
