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

#include "core/os/copymem.h"
#include "core/os/memory.h"
#include "core/os/rw_lock.h"
#include "core/pool_allocator.h"
#include "core/safe_refcount.h"
#include "core/ustring.h"
#include "core/vector.h"

struct MemoryPool {};

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

template <class T>
class PoolVector {

	Vector<T> internal_vector;

public:
	class Access {
		friend class PoolVector;

	protected:
		T *mem;

	public:
		virtual ~Access() {}
		void release() {}
	};

	class Read : public Access {
	public:
		const T &operator[](int p_index) const { return this->mem[p_index]; }
		const T *ptr() const { return this->mem; }

		void operator=(const Read &p_read) {
			mem = p_read.mem;
		}

		Read(const Read &p_read) {
			if (this == &p_read) {
				return;
			}
			mem = p_read.mem;
		}

		Read() {}
	};

	class Write : public Access {
	public:
		T &operator[](int p_index) const { return this->mem[p_index]; }
		T *ptr() const { return this->mem; }

		void operator=(const Write &p_write) {
			mem = p_write.mem;
		}

		Write(const Write &p_write) {
			if (this == &p_write) {
				return;
			}
			mem = p_write.mem;
		}

		Write() {}
	};

	Read read() const {

		Read read;
		read.mem = internal_vector.data();
		return read;
	}
	Write write() {

		Write write;
		write.mem = internal_vector.data();
		return write;
	}

	template <class T2>
	void fill_with(const T2 &p_mc) {

		int c = p_mc.size();
		resize(c);
		Write w = write();
		int idx = 0;
		for (const typename T2::Element *E = p_mc.front(); E; E = E->next()) {

			w[idx++] = E->get();
		}
	}

	void remove(int p_index) {

		int s = size();
		ERR_FAIL_INDEX(p_index, s);
		Write w = write();
		for (int i = p_index; i < s - 1; i++) {

			w[i] = w[i + 1];
		};
		w = Write();
		resize(s - 1);
	}

	inline int size() const;
	T get(int p_index) const;
	void set(int p_index, const T &p_val);
	void push_back(const T &p_val);
	void append(const T &p_val) { push_back(p_val); }
	void append_array(const PoolVector<T> &p_arr) {
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

	PoolVector<T> subarray(int p_from, int p_to) {

		if (p_from < 0) {
			p_from = size() + p_from;
		}
		if (p_to < 0) {
			p_to = size() + p_to;
		}

		ERR_FAIL_INDEX_V(p_from, size(), PoolVector<T>());
		ERR_FAIL_INDEX_V(p_to, size(), PoolVector<T>());

		PoolVector<T> slice;
		int span = 1 + p_to - p_from;
		slice.resize(span);
		Read r = read();
		Write w = slice.write();
		for (int i = 0; i < span; ++i) {
			w[i] = r[p_from + i];
		}

		return slice;
	}

	Error insert(int p_pos, const T &p_val) {

		int s = size();
		ERR_FAIL_INDEX_V(p_pos, s + 1, ERR_INVALID_PARAMETER);
		resize(s + 1);
		{
			Write w = write();
			for (int i = s; i > p_pos; i--)
				w[i] = w[i - 1];
			w[p_pos] = p_val;
		}

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

	bool is_locked() const { return alloc && alloc->lock > 0; }

	inline const T operator[](int p_index) const;

	Error resize(int p_size);

	void invert();

	void operator=(const PoolVector &p_pool_vector) { _reference(p_pool_vector); }
	PoolVector() { alloc = NULL; }
	PoolVector(const PoolVector &p_pool_vector) {
		alloc = NULL;
		_reference(p_pool_vector);
	}
	~PoolVector() { _unreference(); }
};

template <class T>
int PoolVector<T>::size() const {

	return alloc ? alloc->size / sizeof(T) : 0;
}

template <class T>
T PoolVector<T>::get(int p_index) const {

	return operator[](p_index);
}

template <class T>
void PoolVector<T>::set(int p_index, const T &p_val) {

	ERR_FAIL_INDEX(p_index, size());

	Write w = write();
	w[p_index] = p_val;
}

template <class T>
void PoolVector<T>::push_back(const T &p_val) {

	resize(size() + 1);
	set(size() - 1, p_val);
}

template <class T>
const T PoolVector<T>::operator[](int p_index) const {

	CRASH_BAD_INDEX(p_index, size());

	Read r = read();
	return r[p_index];
}

template <class T>
Error PoolVector<T>::resize(int p_size) {

	ERR_FAIL_COND_V(p_size < 0, ERR_INVALID_PARAMETER);

	if (alloc == NULL) {

		if (p_size == 0)
			return OK; //nothing to do here

		//must allocate something
		MemoryPool::alloc_mutex->lock();
		if (MemoryPool::allocs_used == MemoryPool::alloc_count) {
			MemoryPool::alloc_mutex->unlock();
			ERR_EXPLAINC("All memory pool allocations are in use.");
			ERR_FAIL_V(ERR_OUT_OF_MEMORY);
		}

		//take one from the free list
		alloc = MemoryPool::free_list;
		MemoryPool::free_list = alloc->free_list;
		//increment the used counter
		MemoryPool::allocs_used++;

		//cleanup the alloc
		alloc->size = 0;
		alloc->refcount.init();
		alloc->pool_id = POOL_ALLOCATOR_INVALID_ID;
		MemoryPool::alloc_mutex->unlock();

	} else {

		ERR_FAIL_COND_V(alloc->lock > 0, ERR_LOCKED); //can't resize if locked!
	}

	size_t new_size = sizeof(T) * p_size;

	if (alloc->size == new_size)
		return OK; //nothing to do

	if (p_size == 0) {
		_unreference();
		return OK;
	}

	_copy_on_write(); // make it unique

#ifdef DEBUG_ENABLED
	MemoryPool::alloc_mutex->lock();
	MemoryPool::total_memory -= alloc->size;
	MemoryPool::total_memory += new_size;
	if (MemoryPool::total_memory > MemoryPool::max_memory) {
		MemoryPool::max_memory = MemoryPool::total_memory;
	}
	MemoryPool::alloc_mutex->unlock();
#endif

	int cur_elements = alloc->size / sizeof(T);

	if (p_size > cur_elements) {

		if (MemoryPool::memory_pool) {
			//resize memory pool
			//if none, create
			//if some resize
		} else {

			if (alloc->size == 0) {
				alloc->mem = memalloc(new_size);
			} else {
				alloc->mem = memrealloc(alloc->mem, new_size);
			}
		}

		alloc->size = new_size;

		Write w = write();

		for (int i = cur_elements; i < p_size; i++) {

			memnew_placement(&w[i], T);
		}

	} else {

		{
			Write w = write();
			for (int i = p_size; i < cur_elements; i++) {

				w[i].~T();
			}
		}

		if (MemoryPool::memory_pool) {
			//resize memory pool
			//if none, create
			//if some resize
		} else {

			if (new_size == 0) {
				memfree(alloc->mem);
				alloc->mem = NULL;
				alloc->size = 0;

				MemoryPool::alloc_mutex->lock();
				alloc->free_list = MemoryPool::free_list;
				MemoryPool::free_list = alloc;
				MemoryPool::allocs_used--;
				MemoryPool::alloc_mutex->unlock();

			} else {
				alloc->mem = memrealloc(alloc->mem, new_size);
				alloc->size = new_size;
			}
		}
	}

	return OK;
}

template <class T>
void PoolVector<T>::invert() {
	T temp;
	Write w = write();
	int s = size();
	int half_s = s / 2;

	for (int i = 0; i < half_s; i++) {
		temp = w[i];
		w[i] = w[s - i - 1];
		w[s - i - 1] = temp;
	}
}

#endif // POOL_VECTOR_H
