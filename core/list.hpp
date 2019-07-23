/*************************************************************************/
/*  list.h                                                               */
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

#ifndef GLOBALS_LIST_H
#define GLOBALS_LIST_H

#include "core/os/memory.h"
#include "core/sort_array.h"

#include <algorithm>
#include <utility>
#include <vector>

/**
 * Generic Templatized Linked List Implementation.
 * The implementation differs from the STL one because
 * a compatible preallocated linked list can be written
 * using the same API, or features such as erasing an element
 * from the iterator.
 */

template <typename T, typename A = DefaultAllocator>
class List {
public:
	class Element {

	public:
		friend class List<T, A>;
		List<T, A> *ptr_to_list;
		std::size_t inner_index;

		Element() = delete;
		Element(List<T, A> *_ptr_to_list, std::size_t _inner_index) :
				ptr_to_list(_ptr_to_list),
				inner_index(_inner_index) {}

		const Element *next() const {

			return ptr_to_list->getNextElement(this);
		};

		Element *next() {

			return ptr_to_list->getNextElement(this);
		};

		const Element *prev() const {

			return ptr_to_list->getPreviousElement(this);
		};

		Element *prev() {

			return ptr_to_list->getPreviousElement(this);
		};

		const T &operator*() const {

			return ptr_to_list->internal_vector[inner_index];
		};

		const T *operator->() const {

			return &ptr_to_list->internal_vector[inner_index];
		};

		T &operator*() {

			return ptr_to_list->internal_vector[inner_index];
		};

		T *operator->() {

			return &ptr_to_list->internal_vector[inner_index];
		};

		T &get() {

			return ptr_to_list->internal_vector[inner_index];
		};

		const T &get() const {

			// What does this actually do?
			return ptr_to_list->internal_vector[inner_index];
		};

		void set(const T &p_value) {

			// Sets ptr_to_list->internal_vector[inner_index] to p_value
			ptr_to_list->internal_vector[inner_index] = p_value;
		};

		void erase() {

			ptr_to_list->erase(this);
		};
	};
	
private:
	std::vector<T> internal_vector;
	std::vector<Element *> internal_state;

public:
	typename std::vector<T>::iterator begin() noexcept;
	typename std::vector<T>::iterator end() noexcept;
	typename std::vector<T>::const_iterator begin() const noexcept;
	typename std::vector<T>::const_iterator end() const noexcept;

	Element *getNextElement(const Element *p_elem);
	const Element *getNextElement(const Element *p_elem) const;
	Element *getPreviousElement(const Element *p_elem);
	const Element *getPreviousElement(const Element *p_elem) const;

	const Element *front() const;
	Element *front();
	const T &frontT() const;
	T &frontT();

	const Element *back() const;
	Element *back();

	Element *push_back(T &p_value);
	Element *push_back(const T &p_value);

	void pop_back();
	Element *push_front(const T &p_value);
	void pop_front();

	Element *insert_after(Element *p_element, const T &p_value);
	Element *insert_before(Element *p_element, const T &p_value);

	bool erase(const Element *p_elem);
	bool erase(const T &p_value);

	bool empty() const noexcept;
	void clear() noexcept;
	int size() const noexcept;
	void swap(Element *p_elem_left, Element *p_elem_right);

	List &operator=(const List &p_list);
	T &operator[](std::size_t p_index);
	const T &operator[](std::size_t p_index) const;

	void move_to_back(Element *p_elem);
	void invert();
	void move_to_front(Element *p_elem);
	void move_before(Element *p_value, Element *p_where);

	template <class T2>
	Element *find(const T2 &p_value) const;

	void sort();
	template <typename C>
	void sort_custom_inplace();
	template <class C>
	void sort_custom();

	const void *id() const;

	List(const List &p_list);
	List();
	~List();

private:
	void updateIndexes();
};

#endif
