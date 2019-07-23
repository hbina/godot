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

#include "core/list.h"

template <typename T, typename A>
typename std::vector<T>::iterator List<T, A>::begin() noexcept {
	return internal_vector.begin();
}

template <typename T, typename A>
typename std::vector<T>::iterator List<T, A>::end() noexcept {
	return internal_vector.end();
}

template <typename T, typename A>
typename std::vector<T>::const_iterator List<T, A>::begin() const noexcept {
	return internal_vector.cbegin();
}

template <typename T, typename A>
typename std::vector<T>::const_iterator List<T, A>::end() const noexcept {
	return internal_vector.cend();
}

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::getNextElement(const List<T, A>::Element *p_elem) {

	if (internal_state.size() < 2) {
		return nullptr;
	}
	for (std::size_t iter = 0; iter < internal_state.size() - 1; ++iter) {
		if (p_elem == internal_state[iter]) {
			return internal_state[iter + 1];
		}
	}

	return nullptr; // p_elem does not exist in this List
};

template <typename T, typename A>
const typename List<T, A>::Element *List<T, A>::getNextElement(const List<T, A>::Element *p_elem) const {

	if (internal_state.size() < 2 || p_elem->inner_index > internal_state.size() - 2) {
		return nullptr;
	}
	return internal_state[p_elem->inner_index + 1]; // p_elem does not exist in this List
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::getPreviousElement(const List<T, A>::Element *p_elem) {
	// TODO: Because we maintain the ordering all the time, just skip the check
	if (internal_state.size() < 2) {
		return nullptr;
	}

	for (std::size_t iter = 1; iter < internal_state.size(); ++iter) {

		if (p_elem == internal_state[iter]) {
			return internal_state[iter - 1];
		}
	}

	return nullptr; // p_elem does not exist in this List
};

template <typename T, typename A>
const typename List<T, A>::Element *List<T, A>::getPreviousElement(const List<T, A>::Element *p_elem) const {

	if (internal_state.size() < 2) {
		return nullptr;
	}

	for (std::size_t iter = 1; iter < internal_state.size(); ++iter) {

		if (p_elem == internal_state[iter]) {
			return internal_state[iter - 1];
		}
	}

	return nullptr; // p_elem does not exist in this List
};

template <typename T, typename A>
const typename List<T, A>::Element *List<T, A>::front() const {

	if (internal_state.size() == 0) {
		return nullptr;
	}
	return internal_state[0];
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::front() {

	if (internal_state.size() == 0) {
		return nullptr;
	}

	return internal_state[0];
};

template <typename T, typename A>
const T &List<T, A>::frontT() const {

	return internal_vector[0];
};

template <typename T, typename A>
T &List<T, A>::frontT() {

	return internal_vector[0];
};

template <typename T, typename A>
const typename List<T, A>::Element *List<T, A>::back() const {

	if (internal_state.size() == 0) {
		return nullptr;
	}

	return internal_state.back();
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::back() {

	if (internal_state.size() == 0) {
		return nullptr;
	}

	return internal_state.back();
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::push_back(T &p_value) {

	std::size_t new_idx = internal_vector.size();
	internal_vector.push_back(p_value);
	internal_state.push_back(new List<T, A>::Element(this, new_idx));

	return internal_state[new_idx];
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::push_back(const T &p_value) {

	std::size_t new_idx = internal_vector.size();
	internal_vector.push_back(p_value);
	internal_state.push_back(new List<T, A>::Element(this, new_idx));

	return internal_state[new_idx];
};

template <typename T, typename A>
void List<T, A>::pop_back() {

	if (internal_vector.size() == 0) {
		return;
	}

	internal_vector.erase(internal_vector.end() - 1);
	internal_state.erase(internal_state.end() - 1);
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::push_front(const T &p_value) {

	internal_vector.insert(internal_vector.begin(), p_value);
	internal_state.insert(internal_state.begin(), new List<T, A>::Element(this, 0));

	for (std::size_t iter = 1; iter < internal_state.size(); ++iter) {
		++(internal_state[iter]->inner_index);
	}
	return internal_state[0];
};

template <typename T, typename A>
void List<T, A>::pop_front() {

	if (internal_vector.size() == 0) {
		return;
	}

	internal_vector.erase(internal_vector.begin());
	internal_state.erase(internal_state.begin());
	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {
		--(internal_state[iter]->inner_index);
	}
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::insert_after(List<T, A>::Element *p_element, const T &p_value) {

	List<T, A>::Element *return_val = nullptr;
	bool found = false;

	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {

		if (internal_state[iter] == p_element) {

			internal_vector.insert(internal_vector.begin() + iter + 1, p_value);
			internal_state.insert(internal_state.begin() + iter + 1, new List<T, A>::Element(this, iter + 1));
			return_val = internal_state[iter + 1];
			found = true;
		}

		if (found) {
			++(internal_state[iter]->inner_index);
		}
	}

	return return_val;
};

template <typename T, typename A>
typename List<T, A>::Element *List<T, A>::insert_before(List<T, A>::Element *p_element, const T &p_value) {

	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {
		if (internal_state[iter] == p_element) {
			internal_vector.insert(internal_vector.begin() + iter, p_value);
			internal_state.insert(internal_state.begin() + iter, new List<T, A>::Element(this, iter));

			return internal_state[iter];
		} else {
			++(internal_state[iter]->inner_index);
		}
	}

	return nullptr;
};

template <typename T, typename A>
template <typename T2>
typename List<T, A>::Element *List<T, A>::find(const T2 &p_value) const {

	std::size_t find_idx = 0;
	for (; find_idx < internal_vector.size(); ++find_idx) {
		if (p_value == internal_vector[find_idx]) {
			break;
		}
	}
	return find_idx == internal_vector.size() ? nullptr : internal_state[find_idx];
};

template <typename T, typename A>
bool List<T, A>::erase(const List<T, A>::Element *p_elem) {

	bool found = false;
	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {
		if (internal_state[iter] == p_elem) {
			internal_vector.erase(internal_vector.begin() + iter);
			internal_state.erase(internal_state.begin() + iter);
			found = true;
			break;
		}
	}
	updateIndexes(); // Temporary...can be improved by moving all calculations to lambda... I think..
	return found;
};

template <typename T, typename A>
bool List<T, A>::erase(const T &p_value) {
	List<T, A>::Element *elem = find(p_value);
	if (elem) {
		return erase(elem);
	}
	return false;
};

template <typename T, typename A>
bool List<T, A>::empty() const noexcept {

	return internal_vector.empty();
};

template <typename T, typename A>
void List<T, A>::clear() noexcept {
	internal_state.clear();
	internal_vector.clear();
};

template <typename T, typename A>
int List<T, A>::size() const noexcept {

	return internal_vector.size();
};

template <typename T, typename A>
void List<T, A>::swap(List<T, A>::Element *p_elem_left, List<T, A>::Element *p_elem_right) {

	std::size_t p_idx_left = internal_state.size() + 1;
	std::size_t p_idx_right = internal_state.size() + 1;

	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {

		if (internal_state[iter] == p_elem_left) {
			p_idx_left = iter;
		} else if (internal_state[iter] == p_elem_right) {
			p_elem_right = iter;
		}
	}
	if (p_idx_left != internal_state.size() + 1 && p_idx_right != internal_state.size() + 1) {

		internal_state[p_idx_left]->inner_index = p_idx_right;
		internal_state[p_idx_right]->inner_index = p_idx_left;
		std::iter_swap(internal_state.begin() + p_idx_left, internal_state.begin() + p_idx_right);
		std::iter_swap(internal_vector.begin() + p_idx_left, internal_vector.begin() + p_idx_right);
	}
};

template <typename T, typename A>
List<T, A> &List<T, A>::operator=(const List &p_list) {

	clear();

	for (const T &p_value : p_list.internal_vector) {
		internal_vector.push_back(p_value);
	}

	for (std::size_t iter = 0; iter < p_list.internal_state.size(); ++iter) {
		internal_state.push_back(new List<T, A>::Element(this, iter));
	}

	return *this;
};

template <typename T, typename A>
T &List<T, A>::operator[](std::size_t p_index) {

	return internal_vector[p_index];
};

template <typename T, typename A>
const T &List<T, A>::operator[](std::size_t p_index) const {

	return internal_vector[p_index];
};

template <typename T, typename A>
void List<T, A>::move_to_back(List<T, A>::Element *p_elem) {

	// Find p_elem in internal_vector
	// Move it to the back???
	if (internal_state.size() == 0) {
		return;
	}

	const std::size_t last_idx = internal_state.size() - 1;
	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {
		if (internal_state[iter]->inner_index == last_idx) {
			internal_state[iter]->inner_index = p_elem->inner_index;
		}
	}

	std::iter_swap(internal_vector.end(), internal_vector.begin() + p_elem->inner_index);
	p_elem->inner_index = last_idx;
};

template <typename T, typename A>
void List<T, A>::invert() {

	// Iterate through internal_vector
	// change inner_index = size() - inner_index
	// invert internal_vector
	std::reverse(internal_state.begin(), internal_state.end());
	std::reverse(internal_vector.begin(), internal_vector.end());
};

template <typename T, typename A>
void List<T, A>::move_to_front(List<T, A>::Element *p_elem) {

	std::size_t iter = 1;
	for (; iter < internal_state.size(); ++iter) {
		if (internal_state[iter] == p_elem) {
			std::iter_swap(internal_state.begin(), internal_state.begin() + iter);
			break;
		}
	}

	if (iter != internal_state.size()) {
		std::iter_swap(internal_vector.begin(), internal_vector.begin() + iter);
	}
};

template <typename T, typename A>
void List<T, A>::move_before(List<T, A>::Element *p_value, List<T, A>::Element *p_where) {

	std::size_t iter_p_value = internal_state.size();
	std::size_t iter_p_where = internal_state.size();

	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {

		if (internal_state[iter] == p_value) {
			iter_p_value = iter;
		} else if (internal_state[iter] == p_where) {
			iter_p_where = iter;
		}
		if (iter_p_value != internal_state.size() && iter_p_where != internal_state.size()) {
			return;
		}
	}

	if (iter_p_value != internal_state.size() && iter_p_where != internal_state.size()) {

		std::iter_swap(internal_vector.begin() + iter_p_value, internal_vector.begin() + iter_p_where);
		std::iter_swap(internal_state.begin() + iter_p_value, internal_state.begin() + iter_p_where);
	}
};

template <typename T, typename A>
void List<T, A>::sort() {

	std::sort(
			internal_vector.begin(),
			internal_vector.end());
	std::sort(
			internal_state.begin(),
			internal_state.end(),
			[this](const List<T, A>::Element *p_left, const List<T, A>::Element *p_right) {
				return this->internal_vector[p_left->inner_index] < this->internal_vector[p_right->inner_index];
			});
	updateIndexes();
};

template <typename T, typename A>
template <typename C>
void List<T, A>::sort_custom_inplace() {
	C c;
	std::stable_sort(
			internal_vector.begin(),
			internal_vector.end(),
			[&c](const T &p_left, const T &p_right) {
				return c(p_left, p_right);
			});
	std::stable_sort(
			internal_state.begin(),
			internal_state.end(),
			[&c, this](const List<T, A>::Element *p_left, const List<T, A>::Element *p_right) {
				return c(this->internal_vector[p_left->inner_index], this->internal_vector[p_right->inner_index]);
			});
	updateIndexes();
};

template <typename T, typename A>
template <typename C>
void List<T, A>::sort_custom() {
	C c;
	std::sort(
			internal_vector.begin(),
			internal_vector.end(),
			[&c](const T &p_left, const T &p_right) {
				return c(p_left, p_right);
			});
	std::sort(
			internal_state.begin(),
			internal_state.end(),
			[&c, this](const List<T, A>::Element *p_left, const List<T, A>::Element *p_right) {
				return c(this->internal_vector[p_left->inner_index], this->internal_vector[p_right->inner_index]);
			});
	updateIndexes();
};

template <typename T, typename A>
const void *List<T, A>::id() const {
	return (void *)&internal_vector;
};

template <typename T, typename A>
List<T, A>::List(const List &p_list) {

	if (this == &p_list) {
		return;
	}

	internal_state.clear();
	internal_vector.clear();

	// Copies
	for (std::size_t iter = 0; iter < p_list.internal_vector.size(); ++iter) {
		internal_vector.push_back(p_list.internal_vector[iter]);
		internal_state.push_back(new List<T, A>::Element(this, iter));
	}
};

template <typename T, typename A>
List<T, A>::List() = default;

template <typename T, typename A>
List<T, A>::~List() {
	clear();
};

template <typename T, typename A>
void List<T, A>::updateIndexes() {

	for (std::size_t iter = 0; iter < internal_state.size(); ++iter) {
		internal_state[iter]->inner_index = iter;
	}
};