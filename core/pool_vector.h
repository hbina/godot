/*************************************************************************/
/*  pool_vector.h                                                        */
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

#ifndef POOL_VECTOR_H
#define POOL_VECTOR_H

#include "core/ustring.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <vector>

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

template <class T>
class PoolVectorImpl {
	void _reference(const PoolVectorImpl &p_pool_vector) {
		alloc = p_pool_vector.alloc;
	}

public:
	std::shared_ptr<std::vector<T> > alloc;

	class Read {
	public:
		std::shared_ptr<std::vector<T> > access_alloc;

		void _ref(std::shared_ptr<std::vector<T> > p_alloc) {
			access_alloc = p_alloc;
		}

		const T &operator[](int p_index) const { return access_alloc->operator[](p_index); }
		const T *ptr() const { return access_alloc->data(); }

		void operator=(const Read &p_read) {
			if (&access_alloc == &p_read.access_alloc)
				return;
			_ref(p_read.access_alloc);
		}

		Read(const Read &p_read) {
			_ref(p_read.access_alloc);
		}

		Read() {
			access_alloc = std::make_shared<std::vector<T> >();
		}
	};

	class Write {
	public:
		std::shared_ptr<std::vector<T> > access_alloc;

		void _ref(std::shared_ptr<std::vector<T> > p_alloc) {
			access_alloc = p_alloc;
		}

		T &operator[](int p_index) const { return access_alloc->operator[](p_index); }
		T *ptr() const { return access_alloc->data(); }

		void operator=(const Write &p_read) {
			if (&access_alloc == &p_read.access_alloc)
				return;
			_ref(p_read.access_alloc);
		}

		Write(const Write &p_read) {
			_ref(p_read.access_alloc);
		}

		Write() {
			access_alloc = std::make_shared<std::vector<T> >();
		};
	};

	Read read() const {

		Read r;
		r._ref(alloc);
		return r;
	}

	Write write() {

		Write w;
		w._ref(alloc);
		return w;
	}

	template <class MC>
	void fill_with(const MC &p_mc) {

		int c = p_mc.size();
		resize(c);
		Write w = write();
		int idx = 0;
		for (const typename MC::Element *E = p_mc.front(); E; E = E->next()) {

			w[idx++] = E->get();
		}
	}

	void remove(int p_index) {

		alloc->erase(alloc->begin() + p_index);
	}

	int size() const;
	T get(int p_index) const;
	void set(int p_index, const T &p_val);
	void push_back(const T &p_val);
	void append(const T &p_val) { push_back(p_val); }
	void append_array(const PoolVectorImpl<T> &p_arr) {
		int ds = p_arr.size();
		if (ds == 0)
			return;
		int bs = size();
		resize(bs + ds);
		Write w = write();
		Read r = p_arr.read();
		for (int i = 0; i < ds; i++)
			w[bs + i] = r[i];
	}

	Error insert(int p_pos, const T &p_val) {
		alloc->insert(alloc->begin() + p_pos, p_val);
		return OK;
	}

	String join(String delimiter) {
		String rs = "";
		int s = size();
		Read r = read();
		for (int i = 0; i < s; i++) {
			rs += r[i] + delimiter;
		}
		rs.erase(rs.length() - delimiter.length(), delimiter.length());
		return rs;
	}

	// bool is_locked() const { return alloc && alloc->lock > 0; }

	const T operator[](int p_index) const;

	Error resize(int p_size);

	void invert();

	void operator=(const PoolVectorImpl &p_pool_vector) {
		_reference(p_pool_vector);
	}

	PoolVectorImpl() {
		alloc = std::make_shared<std::vector<T> >();
	}

	PoolVectorImpl(const PoolVectorImpl &p_pool_vector) {
		_reference(p_pool_vector);
	}

	~PoolVectorImpl() = default;
};

template <class T>
int PoolVectorImpl<T>::size() const {
	assert(std::numeric_limits<int>::max() > alloc->size());
	return alloc->size();
}

template <class T>
T PoolVectorImpl<T>::get(int p_index) const {

	return operator[](p_index);
}

template <class T>
void PoolVectorImpl<T>::set(int p_index, const T &p_val) {

	ERR_FAIL_COND(p_index < 0 || p_index >= size());

	Write w = write();
	w[p_index] = p_val;
}

template <class T>
void PoolVectorImpl<T>::push_back(const T &p_val) {

	resize(size() + 1);
	set(size() - 1, p_val);
}

template <class T>
const T PoolVectorImpl<T>::operator[](int p_index) const {

	CRASH_BAD_INDEX(p_index, size());

	Read r = read();
	return r[p_index];
}

template <class T>
Error PoolVectorImpl<T>::resize(int p_size) {

	alloc->resize(p_size);
	return OK;
}

template <class T>
void PoolVectorImpl<T>::invert() {
	std::reverse(alloc->begin(), alloc->end());
}

template <typename T>
struct PoolVector : public PoolVectorImpl<T> {
	PoolVector<T> subarray(int p_from, int p_to) {

		if (p_from < 0) {
			p_from = PoolVectorImpl<T>::size() + p_from;
		}
		if (p_to < 0) {
			p_to = PoolVectorImpl<T>::size() + p_to;
		}

		ERR_FAIL_INDEX_V(p_from, PoolVectorImpl<T>::size(), PoolVector<T>());
		ERR_FAIL_INDEX_V(p_to, PoolVectorImpl<T>::size(), PoolVector<T>());

		PoolVector<T> slice;
		int span = 1 + p_to - p_from;
		slice.resize(span);
		typename PoolVectorImpl<T>::Read r = PoolVectorImpl<T>::read();
		typename PoolVectorImpl<T>::Write w = slice.write();
		for (int i = 0; i < span; ++i) {
			w[i] = r[p_from + i];
		}

		return slice;
	}
};

template <>
struct PoolVector<bool> : public PoolVectorImpl<char> {
	PoolVector<char> subarray(int p_from, int p_to) {

		if (p_from < 0) {
			p_from = PoolVectorImpl<char>::size() + p_from;
		}
		if (p_to < 0) {
			p_to = PoolVectorImpl<char>::size() + p_to;
		}

		ERR_FAIL_INDEX_V(p_from, PoolVectorImpl<char>::size(), PoolVector<char>());
		ERR_FAIL_INDEX_V(p_to, PoolVectorImpl<char>::size(), PoolVector<char>());

		PoolVector<char> slice;
		int span = 1 + p_to - p_from;
		slice.resize(span);
		typename PoolVectorImpl<char>::Read r = PoolVectorImpl<char>::read();
		typename PoolVectorImpl<char>::Write w = slice.write();
		for (int i = 0; i < span; ++i) {
			w[i] = r[p_from + i];
		}

		return slice;
	}
};

#endif // POOL_VECTOR_H
