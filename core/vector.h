/*************************************************************************/
/*  vector.h                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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
 * Vector container. Regular Vector Container. Use with care and for smaller arrays when possible. Use Vector for large arrays.
*/

#include <algorithm>
#include <vector>

#include "core/error_macros.h"
#include "core/os/memory.h"
#include "core/sort_array.h"

template <class T>
class Vector {

private:
	std::vector<T> data;

public:
	void push_back(const T &p_elem) {
		data.push_back(p_elem);
	};
	void append(const T &p_elem) { push_back(p_elem); } //alias
	void remove(const int &p_index) { data.erase(std::next(std::begin(data), p_index)); }
	void erase(const T &p_val) {
		std::remove(std::begin(data),
				std::end(data), p_val);
	}
	void invert() {
		std::reverse(std::begin(data), std::end(data));
	}
	void clear() { data.clear(); }
	bool empty() const { return data.empty(); }

	std::vector<T>::iterator begin() {
		return std::begin(data);
	}
	std::vector<T>::iterator end() {
		return std::end(data);
	}

	std::vector<T>::const_iterator cbegin() const {
		return std::cbegin(data);
	}
	std::vector<T>::const_iterator cend() const {
		return std::cend(data);
	}

	T get(const int &p_index) { return data[p_index]; }
	const T get(const int &p_index) const { return data[p_index]; }
	void set(const int &p_index, const T &p_elem) { data[p_index] = p_elem; }
	int size() const { return static_cast<int>(data.size()); }
	Error resize(const int &p_size) {
		data.resize(p_size);
		return OK;
	}
	void reserve(const int &p_size) {
		data.reserve(p_size);
		return OK;
	}
	const T &operator[](const int &p_index) const { return data[p_index]; }
	T &operator[](const int &p_index) { return data[p_index]; }
	Error insert(const int &p_pos, T p_val) {
		data.insert(std::next(std::begin(data), p_pos), p_val);
		return OK;
	}
	int find(const T &p_val, const int &p_from = 0) const {
		return std::distance(std::cbegin(data),
				std::find(std::next(std::cbegin(data), p_from),
						std::cend(data), p_val));
	}
	void append_array(const Vector<T> &p_other) {
		data.reserve(data.size() + p_other.size());
		for (const auto &x : p_other.data) {
			data.emplace_back(x);
		}
	}
	template <class F>
	void sort_custom() {
		std::sort(std::begin(data), std::end(data), F{});
	}
	template <class F>
	void sort_custom(const F &f) {
		std::sort(std::begin(data), std::end(data), f);
	}
	void sort() {
		std::sort(std::begin(data), std::end(data));
	}

	void ordered_insert(const T &p_val) {
		data.insert(std::distance(
							std::cbegin(data),
							std::find_if(
									std::cbegin(data),
									std::cend(data),
									[](const T &lhs, const T &rhs) {
										return lhs < rhs;
									})),
				p_val);
	}

	Vector() {}
	Vector(const Vector &p_from) {
		if (this != &p_from) {
			data = p_from.data;
		}
	}
	Vector &operator=(const Vector &p_from) {
		if (this != &p_from) {
			data = p_from.data;
		}
		return *this;
	}

	Vector<T> subarray(int p_from, int p_to) const {
		if (p_from >= p_to || p_from >= data.size() || p_to <= 0) {
			return Vector<T>();
		}

		if (p_from < 0) {
			p_from = 0;
		}
		if (p_to > data.size()) {
			p_to = data.size();
		}

		Vector<T> slice;
		slice.reserve(p_to - p_from);
		for (const auto &x : data) {
			slice.push_back(x);
		}

		return slice;
	}

	~Vector() {}
};

#endif // VECTOR_H
