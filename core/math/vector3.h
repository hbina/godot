/*************************************************************************/
/*  vector3.h                                                            */
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

#ifndef VECTOR3_H
#define VECTOR3_H

#include "core/math/math_funcs.h"
#include "core/math/vector3i.h"
#include "core/ustring.h"

class Basis;

struct Vector3 {
	enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
	};

	union {
		struct {
			real_t x;
			real_t y;
			real_t z;
		};

		real_t coord[3] = { 0 };
	};

	const real_t &operator[](int p_axis) const {
		return coord[p_axis];
	}

	real_t &operator[](int p_axis) {
		return coord[p_axis];
	}

	void set_axis(int p_axis, real_t p_value);
	real_t get_axis(int p_axis) const;

	int min_axis() const;
	int max_axis() const;

	real_t length() const;
	real_t length_squared() const;

	void normalize();
	Vector3 normalized() const;
	bool is_normalized() const;
	Vector3 inverse() const;

	void zero();

	void snap(Vector3 p_val);
	Vector3 snapped(Vector3 p_val) const;

	void rotate(const Vector3 &p_axis, real_t p_phi);
	Vector3 rotated(const Vector3 &p_axis, real_t p_phi) const;

	/* Static Methods between 2 vector3s */

	Vector3 lerp(const Vector3 &p_b, real_t p_t) const;
	Vector3 slerp(const Vector3 &p_b, real_t p_t) const;
	Vector3 cubic_interpolate(const Vector3 &p_b, const Vector3 &p_pre_a, const Vector3 &p_post_b, real_t p_t) const;
	Vector3 cubic_interpolaten(const Vector3 &p_b, const Vector3 &p_pre_a, const Vector3 &p_post_b, real_t p_t) const;
	Vector3 move_toward(const Vector3 &p_to, const real_t p_delta) const;

	Vector3 cross(const Vector3 &p_b) const;
	real_t dot(const Vector3 &p_b) const;
	Basis outer(const Vector3 &p_b) const;
	Basis to_diagonal_matrix() const;

	Vector3 abs() const;
	Vector3 floor() const;
	Vector3 sign() const;
	Vector3 ceil() const;
	Vector3 round() const;

	real_t distance_to(const Vector3 &p_b) const;
	real_t distance_squared_to(const Vector3 &p_b) const;

	Vector3 posmod(const real_t p_mod) const;
	Vector3 posmodv(const Vector3 &p_modv) const;
	Vector3 project(const Vector3 &p_b) const;

	real_t angle_to(const Vector3 &p_b) const;
	Vector3 direction_to(const Vector3 &p_b) const;

	Vector3 slide(const Vector3 &p_normal) const;
	Vector3 bounce(const Vector3 &p_normal) const;
	Vector3 reflect(const Vector3 &p_normal) const;

	bool is_equal_approx(const Vector3 &p_v) const;

	/* Operators */

	Vector3 &operator+=(const Vector3 &p_v);
	Vector3 operator+(const Vector3 &p_v) const;
	Vector3 &operator-=(const Vector3 &p_v);
	Vector3 operator-(const Vector3 &p_v) const;
	Vector3 &operator*=(const Vector3 &p_v);
	Vector3 operator*(const Vector3 &p_v) const;
	Vector3 &operator/=(const Vector3 &p_v);
	Vector3 operator/(const Vector3 &p_v) const;

	Vector3 &operator*=(real_t p_scalar);
	Vector3 operator*(real_t p_scalar) const;
	Vector3 &operator/=(real_t p_scalar);
	Vector3 operator/(real_t p_scalar) const;

	Vector3 operator-() const;

	bool operator==(const Vector3 &p_v) const;
	bool operator!=(const Vector3 &p_v) const;
	bool operator<(const Vector3 &p_v) const;
	bool operator<=(const Vector3 &p_v) const;
	bool operator>(const Vector3 &p_v) const;
	bool operator>=(const Vector3 &p_v) const;

	operator String() const;
	operator Vector3i() const {
		return Vector3i(x, y, z);
	}

	Vector3();
	Vector3(const Vector3i &p_ivec);
	Vector3(real_t p_x, real_t p_y, real_t p_z);
};

Vector3 operator*(real_t p_scalar, const Vector3 &p_vec);
Vector3 vec3_cross(const Vector3 &p_a, const Vector3 &p_b);
real_t vec3_dot(const Vector3 &p_a, const Vector3 &p_b);

#endif // VECTOR3_H
