/*************************************************************************/
/*  quat.h                                                               */
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

// Circular dependency between Vector3 and Basis :/
#include "core/math/vector3.h"

#ifndef QUAT_H
#define QUAT_H

#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/ustring.h"

class Quat {
public:
	real_t x, y, z, w;

	constexpr _FORCE_INLINE_ real_t length_squared() const;
	constexpr real_t length() const;
	constexpr void normalize();
	constexpr Quat normalized() const;
	constexpr bool is_normalized() const;
	constexpr Quat inverse() const;
	constexpr _FORCE_INLINE_ real_t dot(const Quat &q) const;

	constexpr void set_euler_xyz(const Vector3 &p_euler);
	constexpr Vector3 get_euler_xyz() const;
	constexpr void set_euler_yxz(const Vector3 &p_euler);
	constexpr Vector3 get_euler_yxz() const;

	constexpr void set_euler(const Vector3 &p_euler) { set_euler_yxz(p_euler); };
	constexpr Vector3 get_euler() const { return get_euler_yxz(); };

	constexpr Quat slerp(const Quat &q, const real_t &t) const;
	constexpr Quat slerpni(const Quat &q, const real_t &t) const;
	constexpr Quat cubic_slerp(const Quat &q, const Quat &prep, const Quat &postq, const real_t &t) const;

	constexpr void set_axis_angle(const Vector3 &axis, const real_t &angle);
	constexpr _FORCE_INLINE_ void get_axis_angle(Vector3 &r_axis, real_t &r_angle) const {
		r_angle = 2 * Math::acos(w);
		real_t r = ((real_t)1) / Math::sqrt(1 - w * w);
		r_axis.x = x * r;
		r_axis.y = y * r;
		r_axis.z = z * r;
	}

	constexpr void operator*=(const Quat &q);
	constexpr Quat operator*(const Quat &q) const;

	constexpr Quat operator*(const Vector3 &v) const {
		return Quat(w * v.x + y * v.z - z * v.y,
				w * v.y + z * v.x - x * v.z,
				w * v.z + x * v.y - y * v.x,
				-x * v.x - y * v.y - z * v.z);
	}

	constexpr _FORCE_INLINE_ Vector3 xform(const Vector3 &v) const {
#ifdef MATH_CHECKS
		ERR_FAIL_COND_V(!is_normalized(), v);
#endif
		Vector3 u(x, y, z);
		Vector3 uv = u.cross(v);
		return v + ((uv * w) + u.cross(uv)) * ((real_t)2);
	}

	constexpr _FORCE_INLINE_ void operator+=(const Quat &q);
	constexpr _FORCE_INLINE_ void operator-=(const Quat &q);
	constexpr _FORCE_INLINE_ void operator*=(const real_t &s);
	constexpr _FORCE_INLINE_ void operator/=(const real_t &s);
	constexpr _FORCE_INLINE_ Quat operator+(const Quat &q2) const;
	constexpr _FORCE_INLINE_ Quat operator-(const Quat &q2) const;
	constexpr _FORCE_INLINE_ Quat operator-() const;
	constexpr _FORCE_INLINE_ Quat operator*(const real_t &s) const;
	constexpr _FORCE_INLINE_ Quat operator/(const real_t &s) const;

	constexpr _FORCE_INLINE_ bool operator==(const Quat &p_quat) const;
	constexpr _FORCE_INLINE_ bool operator!=(const Quat &p_quat) const;

	constexpr operator String() const;

	constexpr inline void set(real_t p_x, real_t p_y, real_t p_z, real_t p_w) {
		x = p_x;
		y = p_y;
		z = p_z;
		w = p_w;
	}

	constexpr inline Quat(real_t p_x, real_t p_y, real_t p_z, real_t p_w) :
			x(p_x),
			y(p_y),
			z(p_z),
			w(p_w) {
	}

	constexpr Quat(const Vector3 &axis, const real_t &angle) :
			x(0),
			y(0),
			z(0),
			w(0) { set_axis_angle(axis, angle); }

	constexpr Quat(const Vector3 &euler) :
			x(0),
			y(0),
			z(0),
			w(0) { set_euler(euler); }
	constexpr Quat(const Quat &q) :
			x(q.x),
			y(q.y),
			z(q.z),
			w(q.w) {}

	constexpr Quat operator=(const Quat &q) {
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
		return *this;
	}

	constexpr Quat(const Vector3 &v0, const Vector3 &v1) // shortest arc
			:
			x(0),
			y(0),
			z(0),
			w(0) {
		Vector3 c = v0.cross(v1);
		real_t d = v0.dot(v1);

		if (d < -1.0 + CMP_EPSILON) {
			x = 0;
			y = 1;
			z = 0;
			w = 0;
		} else {

			real_t s = Math::sqrt((1.0 + d) * 2.0);
			real_t rs = 1.0 / s;

			x = c.x * rs;
			y = c.y * rs;
			z = c.z * rs;
			w = s * 0.5;
		}
	}

	inline Quat() :
			x(0),
			y(0),
			z(0),
			w(1) {
	}
};

constexpr real_t Quat::dot(const Quat &q) const {
	return x * q.x + y * q.y + z * q.z + w * q.w;
}

constexpr real_t Quat::length_squared() const {
	return dot(*this);
}

constexpr void Quat::operator+=(const Quat &q) {
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;
}

constexpr void Quat::operator-=(const Quat &q) {
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;
}

constexpr void Quat::operator*=(const real_t &s) {
	x *= s;
	y *= s;
	z *= s;
	w *= s;
}

constexpr void Quat::operator/=(const real_t &s) {

	*this *= 1.0 / s;
}

constexpr Quat Quat::operator+(const Quat &q2) const {
	const Quat &q1 = *this;
	return Quat(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

constexpr Quat Quat::operator-(const Quat &q2) const {
	const Quat &q1 = *this;
	return Quat(q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w);
}

constexpr Quat Quat::operator-() const {
	const Quat &q2 = *this;
	return Quat(-q2.x, -q2.y, -q2.z, -q2.w);
}

constexpr Quat Quat::operator*(const real_t &s) const {
	return Quat(x * s, y * s, z * s, w * s);
}

constexpr Quat Quat::operator/(const real_t &s) const {
	return *this * (1.0 / s);
}

constexpr bool Quat::operator==(const Quat &p_quat) const {
	return x == p_quat.x && y == p_quat.y && z == p_quat.z && w == p_quat.w;
}

constexpr bool Quat::operator!=(const Quat &p_quat) const {
	return x != p_quat.x || y != p_quat.y || z != p_quat.z || w != p_quat.w;
}

#endif
