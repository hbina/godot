/*************************************************************************/
/*  array.cpp                                                            */
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

#include "array.h"

#include "core/hashfuncs.h"
#include "core/object.h"
#include "core/variant.h"
#include "core/vector.h"

#include <iterator>

Variant &Array::operator[](int p_idx) {

	return internal_vector[p_idx];
}

const Variant &Array::operator[](int p_idx) const {

	return internal_vector[p_idx];
}

int Array::size() const {

	return internal_vector.size();
}
bool Array::empty() const {

	return internal_vector.empty();
}
void Array::clear() {

	internal_vector.clear();
}

bool Array::operator==(const Array &p_array) const {

	return this == &p_array;
}

uint32_t Array::hash() const {

	uint32_t h = hash_djb2_one_32(0);
	for (const auto &v : internal_vector) {
		h = hash_djb2_one_32(v.hash(), h);
	}
	return h;
}

Array &Array::operator=(const Array &p_array) {

	internal_vector = p_array.internal_vector;
	return *this;
}

void Array::push_back(const Variant &p_value) {

	internal_vector.push_back(p_value);
}

void Array::resize(int p_new_size) {

	internal_vector.resize(p_new_size);
}

void Array::insert(int p_pos, const Variant &p_value) {

	internal_vector.insert(p_pos, p_value);
}

void Array::erase(const Variant &p_value) {

	internal_vector.erase(p_value);
}

Variant Array::front() const {

	if (internal_vector.size() == 0) {
		CRASH_NOW();
	}
	return operator[](0);
}

Variant Array::back() const {

	if (internal_vector.size() == 0) {
		CRASH_NOW();
	}
	return internal_vector[internal_vector.size() - 1];
}

int Array::find(const Variant &p_value, int p_from) const {

	int return_index = static_cast<int>(
			std::distance(
					internal_vector.begin() + p_from,
					std::find_if(
							internal_vector.begin(),
							internal_vector.end(),
							[&p_value](const auto &value) {
								return value == p_value;
							})));
	return return_index == internal_vector.size() ? -1 : return_index;
}

int Array::rfind(const Variant &p_value, int p_from) const {

	auto return_index = static_cast<int>(
			std::distance(
					internal_vector.rbegin(),
					std::find_if(
							internal_vector.rbegin() + p_from,
							internal_vector.rend(),
							[&p_value](const auto &value) {
								return value == p_value;
							})));
	return return_index == internal_vector.size() ? -1 : return_index;
}

int Array::find_last(const Variant &p_value) const {

	return rfind(p_value);
}

int Array::count(const Variant &p_value) const {

	int amount = 0;
	std::for_each(
			internal_vector.begin(),
			internal_vector.end(),
			[&p_value, &amount](const auto &value) {
				if (value == p_value) {
					++amount;
				}
			});
	return amount;
}

bool Array::has(const Variant &p_value) const {

	return find(p_value) != -1;
}

void Array::remove(int p_pos) {

	internal_vector.remove(p_pos);
}

void Array::set(int p_idx, const Variant &p_value) {

	internal_vector[p_idx] = p_value;
}

const Variant &Array::get(int p_idx) const {

	return internal_vector[p_idx];
}

Array Array::duplicate(bool p_deep) const {

	Array new_arr;
	for (const auto &variant : internal_vector) {
		new_arr.push_back(variant);
	}
	return new_arr;
}

struct _ArrayVariantSort {

	bool operator()(const Variant &p_l, const Variant &p_r) const {
		bool valid = false;
		Variant res;
		Variant::evaluate(Variant::OP_LESS, p_l, p_r, res, valid);
		if (!valid)
			res = false;
		return res;
	}
};

Array &Array::sort() {

	internal_vector.sort_custom<_ArrayVariantSort>();
	return *this;
}

struct _ArrayVariantSortCustom {

	Object *obj;
	StringName func;

	bool operator()(const Variant &p_l, const Variant &p_r) const {

		const Variant *args[2] = { &p_l, &p_r };
		Variant::CallError err;
		bool res = obj->call(func, args, 2, err);
		if (err.error != Variant::CallError::CALL_OK)
			res = false;
		return res;
	}
};

Array &Array::sort_custom(Object *p_obj, const StringName &p_function) {

	// Fuck does this actually do?
	SortArray<Variant, _ArrayVariantSortCustom, true> avs;
	avs.compare.obj = p_obj;
	avs.compare.func = p_function;
	avs.sort(internal_vector.ptrw(), internal_vector.size());
	return *this;
}

void Array::shuffle() {

	const int n = _p->array.size();
	if (n < 2)
		return;
	Variant *data = _p->array.ptrw();
	for (int i = n - 1; i >= 1; i--) {
		const int j = Math::rand() % (i + 1);
		const Variant tmp = data[j];
		data[j] = data[i];
		data[i] = tmp;
	}
}

template <typename Less>
int bisect(const Vector<Variant> &p_array, const Variant &p_value, bool p_before, const Less &p_less) {

	int lo = 0;
	int hi = p_array.size();
	if (p_before) {
		while (lo < hi) {
			const int mid = (lo + hi) / 2;
			if (p_less(p_array.get(mid), p_value)) {
				lo = mid + 1;
			} else {
				hi = mid;
			}
		}
	} else {
		while (lo < hi) {
			const int mid = (lo + hi) / 2;
			if (p_less(p_value, p_array.get(mid))) {
				hi = mid;
			} else {
				lo = mid + 1;
			}
		}
	}
	return lo;
}

int Array::bsearch(const Variant &p_value, bool p_before) {

	return bisect(_p->array, p_value, p_before, _ArrayVariantSort());
}

int Array::bsearch_custom(const Variant &p_value, Object *p_obj, const StringName &p_function, bool p_before) {

	ERR_FAIL_NULL_V(p_obj, 0);

	_ArrayVariantSortCustom less;
	less.obj = p_obj;
	less.func = p_function;

	return bisect(_p->array, p_value, p_before, less);
}

Array &Array::invert() {

	_p->array.invert();
	return *this;
}

void Array::push_front(const Variant &p_value) {

	_p->array.insert(0, p_value);
}

Variant Array::pop_back() {

	if (!_p->array.empty()) {
		int n = _p->array.size() - 1;
		Variant ret = _p->array.get(n);
		_p->array.resize(n);
		return ret;
	}
	return Variant();
}

Variant Array::pop_front() {

	if (!_p->array.empty()) {
		Variant ret = _p->array.get(0);
		_p->array.remove(0);
		return ret;
	}
	return Variant();
}

Variant Array::min() const {

	Variant minval;
	for (int i = 0; i < size(); i++) {
		if (i == 0) {
			minval = get(i);
		} else {
			bool valid;
			Variant ret;
			Variant test = get(i);
			Variant::evaluate(Variant::OP_LESS, test, minval, ret, valid);
			if (!valid) {
				return Variant(); //not a valid comparison
			}
			if (bool(ret)) {
				//is less
				minval = test;
			}
		}
	}
	return minval;
}

Variant Array::max() const {

	Variant maxval;
	for (int i = 0; i < size(); i++) {
		if (i == 0) {
			maxval = get(i);
		} else {
			bool valid;
			Variant ret;
			Variant test = get(i);
			Variant::evaluate(Variant::OP_GREATER, test, maxval, ret, valid);
			if (!valid) {
				return Variant(); //not a valid comparison
			}
			if (bool(ret)) {
				//is less
				maxval = test;
			}
		}
	}
	return maxval;
}

const void *Array::id() const {
	return _p->array.ptr();
}

Array::Array(const Array &p_from) {
	_p = nullptr;
	_ref(p_from);
}

Array::Array() {

	_p = memnew(ArrayPrivate);
	_p->refcount.init();
}
Array::~Array() {

	_unref();
}
