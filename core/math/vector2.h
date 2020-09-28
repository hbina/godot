/*************************************************************************/
/*  vector2.h                                                            */
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

#ifndef VECTOR2_H
#define VECTOR2_H

#include <type_traits>

#include "core/math/math_funcs.h"
#include "core/ustring.h"

struct Vector2i;

struct Vector2 {
	enum Axis {
		AXIS_X,
		AXIS_Y,
	};

	union {
		real_t x = 0;
		real_t width;
	};
	union {
		real_t y = 0;
		real_t height;
	};

	real_t &operator[](const int p_idx);
	const real_t &operator[](const int p_idx) const;

	void normalize();
	Vector2 normalized() const;
	bool is_normalized() const;

	real_t length() const;
	real_t length_squared() const;

	real_t distance_to(const Vector2 &p_vector2) const;
	real_t distance_squared_to(const Vector2 &p_vector2) const;
	real_t angle_to(const Vector2 &p_vector2) const;
	real_t angle_to_point(const Vector2 &p_vector2) const;
	Vector2 direction_to(const Vector2 &p_b) const;

	real_t dot(const Vector2 &p_other) const;
	real_t cross(const Vector2 &p_other) const;
	Vector2 posmod(const real_t p_mod) const;
	Vector2 posmodv(const Vector2 &p_modv) const;
	Vector2 project(const Vector2 &p_b) const;

	Vector2 plane_project(real_t p_d, const Vector2 &p_vec) const;

	Vector2 clamped(real_t p_len) const;

	Vector2 lerp(const Vector2 &p_b, real_t p_t) const;
	Vector2 slerp(const Vector2 &p_b, real_t p_t) const;
	Vector2 cubic_interpolate(const Vector2 &p_b, const Vector2 &p_pre_a, const Vector2 &p_post_b, real_t p_t) const;
	Vector2 move_toward(const Vector2 &p_to, const real_t p_delta) const;

	Vector2 slide(const Vector2 &p_normal) const;
	Vector2 bounce(const Vector2 &p_normal) const;
	Vector2 reflect(const Vector2 &p_normal) const;

	bool is_equal_approx(const Vector2 &p_v) const;

	Vector2 operator+(const Vector2 &p_v) const;
	void operator+=(const Vector2 &p_v);
	Vector2 operator-(const Vector2 &p_v) const;
	void operator-=(const Vector2 &p_v);
	Vector2 operator*(const Vector2 &p_v1) const;
	void operator*=(const Vector2 &rvalue);
	Vector2 operator/(const Vector2 &p_v1) const;
	void operator/=(const Vector2 &rvalue);

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2 operator+(const T &rvalue) const {
		return Vector2(x + rvalue, y + rvalue);
	};

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator+=(const T &rvalue) {
		x += rvalue;
		y += rvalue;
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2 operator-(const T &rvalue) const {
		return Vector2(x - rvalue, y - rvalue);
	};

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator-=(const T &rvalue) {
		x -= rvalue;
		y -= rvalue;
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2 operator*(const T &rvalue) const {
		return Vector2(x * rvalue, y * rvalue);
	};

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator*=(const T &rvalue) {
		x *= rvalue;
		y *= rvalue;
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2 operator/(const T &rvalue) const {
		return Vector2(x / rvalue, y / rvalue);
	};

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator/=(const T &rvalue) {
		x /= rvalue;
		y /= rvalue;
	}

	Vector2 operator-() const;

	bool operator==(const Vector2 &p_vec2) const;
	bool operator!=(const Vector2 &p_vec2) const;

	bool operator<(const Vector2 &p_vec2) const;
	bool operator>(const Vector2 &p_vec2) const;
	bool operator<=(const Vector2 &p_vec2) const;
	bool operator>=(const Vector2 &p_vec2) const;

	real_t angle() const;

	Vector2 abs() const;

	Vector2 rotated(real_t p_by) const;
	Vector2 tangent() const;

	Vector2 sign() const;
	Vector2 floor() const;
	Vector2 ceil() const;
	Vector2 round() const;
	Vector2 snapped(const Vector2 &p_by) const;
	real_t aspect() const;

	operator String() const;

	Vector2();
	Vector2(real_t p_x, real_t p_y);
};

using Size2 = Vector2;
using Point2 = Vector2;

/* INTEGER STUFF */

struct Vector2i {
	enum Axis {
		AXIS_X,
		AXIS_Y,
	};

	union {
		int x = 0;
		int width;
	};
	union {
		int y = 0;
		int height;
	};

	int &operator[](const int p_idx);
	const int &operator[](const int p_idx) const;

	Vector2i operator+(const Vector2i &p_v) const;
	void operator+=(const Vector2i &p_v);
	Vector2i operator-(const Vector2i &p_v) const;
	void operator-=(const Vector2i &p_v);
	Vector2i operator*(const Vector2i &p_v1) const;
	void operator*=(const Vector2i &p_v);
	Vector2i operator/(const Vector2i &p_v1) const;
	void operator/=(const Vector2i &p_v);

	// Might want to constraint the arithmetic types to just floating point.

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2i operator+(const T &rvalue) const {
		return Vector2i(x + rvalue, y + rvalue);
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator+=(const T &rvalue) {
		x += rvalue;
		y += rvalue;
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2i operator-(const T &rvalue) const {
		return Vector2i(x - rvalue, y - rvalue);
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator-=(const T &rvalue) {
		x -= rvalue;
		y -= rvalue;
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2i operator*(const T &rvalue) const {
		return Vector2i(x * rvalue, y * rvalue);
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator*=(const T &rvalue) {
		x *= rvalue;
		y *= rvalue;
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	Vector2i operator/(const T &rvalue) const {
		return Vector2i(x / rvalue, y / rvalue);
	}

	template <typename T,
			typename = std::enable_if_t<
					std::is_arithmetic<T>::value>>
	void operator/=(const T &rvalue) {
		x /= rvalue;
		y /= rvalue;
	}

	Vector2i operator-() const;
	bool operator<(const Vector2i &p_vec2) const;
	bool operator>(const Vector2i &p_vec2) const;

	bool operator<=(const Vector2i &p_vec2) const;
	bool operator>=(const Vector2i &p_vec2) const;

	bool operator==(const Vector2i &p_vec2) const;
	bool operator!=(const Vector2i &p_vec2) const;

	real_t aspect() const;
	Vector2i sign() const;
	Vector2i abs() const;

	operator String() const;

	operator Vector2() const;

	Vector2 plane_project(real_t p_d, const Vector2 &p_vec) const;
	Vector2 operator+(const Vector2 &p_v) const;
	void operator+=(const Vector2 &p_v);
	Vector2 operator-(const Vector2 &p_v) const;
	void operator-=(const Vector2 &p_v);
	Vector2 operator*(const Vector2 &p_v1) const;
	void operator*=(const Vector2 &p_v);
	Vector2 operator/(const Vector2 &p_v1) const;
	void operator/=(const Vector2 &p_v);
	bool operator==(const Vector2 &p_vec2) const;
	bool operator!=(const Vector2 &p_vec2) const;
	Vector2 lerp(const Vector2 &p_b, real_t p_t) const;
	Vector2 slerp(const Vector2 &p_b, real_t p_t) const;
	Vector2 direction_to(const Vector2 &p_b) const;

	Vector2i();
	Vector2i(const Vector2 &p_vec2);
	Vector2i(int p_x, int p_y);
};

Vector2 operator*(real_t p_scalar, const Vector2 &p_vec);

using Size2i = Vector2i;
using Point2i = Vector2i;

#endif // VECTOR2_H
