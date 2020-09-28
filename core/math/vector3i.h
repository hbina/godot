/*************************************************************************/
/*  vector3i.h                                                           */
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

#ifndef VECTOR3I_H
#define VECTOR3I_H

#include "core/typedefs.h"
#include "core/ustring.h"

struct Vector3i {
	enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
	};

	union {
		struct {
			int32_t x;
			int32_t y;
			int32_t z;
		};

		int32_t coord[3] = { 0 };
	};

	const int32_t &operator[](int p_axis) const;

	int32_t &operator[](int p_axis);

	void set_axis(int p_axis, int32_t p_value);
	int32_t get_axis(int p_axis) const;

	int min_axis() const;
	int max_axis() const;

	void zero();

	Vector3i abs() const;
	Vector3i sign() const;

	/* Operators */

	Vector3i &operator+=(const Vector3i &p_v);
	Vector3i operator+(const Vector3i &p_v) const;
	Vector3i &operator-=(const Vector3i &p_v);
	Vector3i operator-(const Vector3i &p_v) const;
	Vector3i &operator*=(const Vector3i &p_v);
	Vector3i operator*(const Vector3i &p_v) const;
	Vector3i &operator/=(const Vector3i &p_v);
	Vector3i operator/(const Vector3i &p_v) const;

	Vector3i &operator*=(int32_t p_scalar);
	Vector3i operator*(int32_t p_scalar) const;
	Vector3i &operator/=(int32_t p_scalar);
	Vector3i operator/(int32_t p_scalar) const;

	Vector3i operator-() const;

	bool operator==(const Vector3i &p_v) const;
	bool operator!=(const Vector3i &p_v) const;
	bool operator<(const Vector3i &p_v) const;
	bool operator<=(const Vector3i &p_v) const;
	bool operator>(const Vector3i &p_v) const;
	bool operator>=(const Vector3i &p_v) const;

	operator String() const;

	Vector3i();
	Vector3i(const Vector3i &);
	Vector3i(int32_t p_x, int32_t p_y, int32_t p_z);
};

Vector3i operator*(int32_t p_scalar, const Vector3i &p_vec);

#endif // VECTOR3I_H
