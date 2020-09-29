/*************************************************************************/
/*  rect2.cpp                                                            */
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

#include "core/math/transform_2d.h" // Includes rect2.h but Rect2 needs Transform2D

bool Rect2::is_equal_approx(const Rect2 &p_rect) const {
	return position.is_equal_approx(p_rect.position) && size.is_equal_approx(p_rect.size);
}

bool Rect2::intersects_segment(const Point2 &p_from, const Point2 &p_to, Point2 *r_pos, Point2 *r_normal) const {
	real_t min = 0, max = 1;
	int axis = 0;
	real_t sign = 0;

	for (int i = 0; i < 2; i++) {
		real_t seg_from = p_from[i];
		real_t seg_to = p_to[i];
		real_t box_begin = position[i];
		real_t box_end = box_begin + size[i];
		real_t cmin, cmax;
		real_t csign;

		if (seg_from < seg_to) {
			if (seg_from > box_end || seg_to < box_begin) {
				return false;
			}
			real_t length = seg_to - seg_from;
			cmin = (seg_from < box_begin) ? ((box_begin - seg_from) / length) : 0;
			cmax = (seg_to > box_end) ? ((box_end - seg_from) / length) : 1;
			csign = -1.0;

		} else {
			if (seg_to > box_end || seg_from < box_begin) {
				return false;
			}
			real_t length = seg_to - seg_from;
			cmin = (seg_from > box_end) ? (box_end - seg_from) / length : 0;
			cmax = (seg_to < box_begin) ? (box_begin - seg_from) / length : 1;
			csign = 1.0;
		}

		if (cmin > min) {
			min = cmin;
			axis = i;
			sign = csign;
		}
		if (cmax < max) {
			max = cmax;
		}
		if (max < min) {
			return false;
		}
	}

	Vector2 rel = p_to - p_from;

	if (r_normal) {
		Vector2 normal;
		normal[axis] = sign;
		*r_normal = normal;
	}

	if (r_pos) {
		*r_pos = p_from + rel * min;
	}

	return true;
}

bool Rect2::intersects_transformed(const Transform2D &p_xform, const Rect2 &p_rect) const {
	//SAT intersection between local and transformed rect2

	Vector2 xf_points[4] = {
		p_xform.xform(p_rect.position),
		p_xform.xform(Vector2(p_rect.position.x + p_rect.size.x, p_rect.position.y)),
		p_xform.xform(Vector2(p_rect.position.x, p_rect.position.y + p_rect.size.y)),
		p_xform.xform(Vector2(p_rect.position.x + p_rect.size.x, p_rect.position.y + p_rect.size.y)),
	};

	real_t low_limit;

	//base rect2 first (faster)

	if (xf_points[0].y > position.y) {
		goto next1;
	}
	if (xf_points[1].y > position.y) {
		goto next1;
	}
	if (xf_points[2].y > position.y) {
		goto next1;
	}
	if (xf_points[3].y > position.y) {
		goto next1;
	}

	return false;

next1:

	low_limit = position.y + size.y;

	if (xf_points[0].y < low_limit) {
		goto next2;
	}
	if (xf_points[1].y < low_limit) {
		goto next2;
	}
	if (xf_points[2].y < low_limit) {
		goto next2;
	}
	if (xf_points[3].y < low_limit) {
		goto next2;
	}

	return false;

next2:

	if (xf_points[0].x > position.x) {
		goto next3;
	}
	if (xf_points[1].x > position.x) {
		goto next3;
	}
	if (xf_points[2].x > position.x) {
		goto next3;
	}
	if (xf_points[3].x > position.x) {
		goto next3;
	}

	return false;

next3:

	low_limit = position.x + size.x;

	if (xf_points[0].x < low_limit) {
		goto next4;
	}
	if (xf_points[1].x < low_limit) {
		goto next4;
	}
	if (xf_points[2].x < low_limit) {
		goto next4;
	}
	if (xf_points[3].x < low_limit) {
		goto next4;
	}

	return false;

next4:

	Vector2 xf_points2[4] = {
		position,
		Vector2(position.x + size.x, position.y),
		Vector2(position.x, position.y + size.y),
		Vector2(position.x + size.x, position.y + size.y),
	};

	real_t maxa = p_xform.elements[0].dot(xf_points2[0]);
	real_t mina = maxa;

	real_t dp = p_xform.elements[0].dot(xf_points2[1]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[0].dot(xf_points2[2]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[0].dot(xf_points2[3]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	real_t maxb = p_xform.elements[0].dot(xf_points[0]);
	real_t minb = maxb;

	dp = p_xform.elements[0].dot(xf_points[1]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[0].dot(xf_points[2]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[0].dot(xf_points[3]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	if (mina > maxb) {
		return false;
	}
	if (minb > maxa) {
		return false;
	}

	maxa = p_xform.elements[1].dot(xf_points2[0]);
	mina = maxa;

	dp = p_xform.elements[1].dot(xf_points2[1]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[1].dot(xf_points2[2]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[1].dot(xf_points2[3]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	maxb = p_xform.elements[1].dot(xf_points[0]);
	minb = maxb;

	dp = p_xform.elements[1].dot(xf_points[1]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[1].dot(xf_points[2]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[1].dot(xf_points[3]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	if (mina > maxb) {
		return false;
	}
	if (minb > maxa) {
		return false;
	}

	return true;
}

// RECTI

const Point2i &Rect2i::get_position() const {
	return position;
}
void Rect2i::set_position(const Point2i &p_position) {
	position = p_position;
}
const Size2i &Rect2i::get_size() const {
	return size;
}
void Rect2i::set_size(const Size2i &p_size) {
	size = p_size;
}

int Rect2i::get_area() const {
	return size.width * size.height;
}

bool Rect2i::intersects(const Rect2i &p_rect) const {
	if (position.x > (p_rect.position.x + p_rect.size.width)) {
		return false;
	}
	if ((position.x + size.width) < p_rect.position.x) {
		return false;
	}
	if (position.y > (p_rect.position.y + p_rect.size.height)) {
		return false;
	}
	if ((position.y + size.height) < p_rect.position.y) {
		return false;
	}

	return true;
}

bool Rect2i::encloses(const Rect2i &p_rect) const {
	return (p_rect.position.x >= position.x) && (p_rect.position.y >= position.y) &&
		   ((p_rect.position.x + p_rect.size.x) < (position.x + size.x)) &&
		   ((p_rect.position.y + p_rect.size.y) < (position.y + size.y));
}

bool Rect2i::has_no_area() const {
	return (size.x <= 0 || size.y <= 0);
}

Rect2i Rect2i::clip(const Rect2i &p_rect) const { /// return a clipped rect

	Rect2i new_rect = p_rect;

	if (!intersects(new_rect)) {
		return Rect2i();
	}

	new_rect.position.x = MAX(p_rect.position.x, position.x);
	new_rect.position.y = MAX(p_rect.position.y, position.y);

	Point2 p_rect_end = p_rect.position + p_rect.size;
	Point2 end = position + size;

	new_rect.size.x = (int)(MIN(p_rect_end.x, end.x) - new_rect.position.x);
	new_rect.size.y = (int)(MIN(p_rect_end.y, end.y) - new_rect.position.y);

	return new_rect;
}

Rect2i Rect2i::merge(const Rect2i &p_rect) const { ///< return a merged rect

	Rect2i new_rect;

	new_rect.position.x = MIN(p_rect.position.x, position.x);
	new_rect.position.y = MIN(p_rect.position.y, position.y);

	new_rect.size.x = MAX(p_rect.position.x + p_rect.size.x, position.x + size.x);
	new_rect.size.y = MAX(p_rect.position.y + p_rect.size.y, position.y + size.y);

	new_rect.size = new_rect.size - new_rect.position; //make relative again

	return new_rect;
}
bool Rect2i::has_point(const Point2 &p_point) const {
	if (p_point.x < position.x) {
		return false;
	}
	if (p_point.y < position.y) {
		return false;
	}

	if (p_point.x >= (position.x + size.x)) {
		return false;
	}
	if (p_point.y >= (position.y + size.y)) {
		return false;
	}

	return true;
}

bool Rect2i::operator==(const Rect2i &p_rect) const {
	return position == p_rect.position && size == p_rect.size;
}

bool Rect2i::operator!=(const Rect2i &p_rect) const {
	return position != p_rect.position || size != p_rect.size;
}

Rect2i Rect2i::grow(int p_by) const {
	Rect2i g = *this;
	g.position.x -= p_by;
	g.position.y -= p_by;
	g.size.width += p_by * 2;
	g.size.height += p_by * 2;
	return g;
}

Rect2i Rect2i::grow_margin(Margin p_margin, int p_amount) const {
	Rect2i g = *this;
	g = g.grow_individual((MARGIN_LEFT == p_margin) ? p_amount : 0,
			(MARGIN_TOP == p_margin) ? p_amount : 0,
			(MARGIN_RIGHT == p_margin) ? p_amount : 0,
			(MARGIN_BOTTOM == p_margin) ? p_amount : 0);
	return g;
}

Rect2i Rect2i::grow_individual(int p_left, int p_top, int p_right, int p_bottom) const {
	Rect2i g = *this;
	g.position.x -= p_left;
	g.position.y -= p_top;
	g.size.width += p_left + p_right;
	g.size.height += p_top + p_bottom;

	return g;
}

Rect2i Rect2i::expand(const Vector2i &p_vector) const {
	Rect2i r = *this;
	r.expand_to(p_vector);
	return r;
}

void Rect2i::expand_to(const Point2i &p_vector) {
	Point2i begin = position;
	Point2i end = position + size;

	if (p_vector.x < begin.x) {
		begin.x = p_vector.x;
	}
	if (p_vector.y < begin.y) {
		begin.y = p_vector.y;
	}

	if (p_vector.x > end.x) {
		end.x = p_vector.x;
	}
	if (p_vector.y > end.y) {
		end.y = p_vector.y;
	}

	position = begin;
	size = end - begin;
}

Rect2i Rect2i::abs() const {
	return Rect2i(Point2i(position.x + MIN(size.x, 0), position.y + MIN(size.y, 0)), size.abs());
}

Rect2i::operator String() const {
	return String(position) + ", " + String(size);
}

Rect2i::operator Rect2() const {
	return Rect2(position, size);
}

Rect2i::Rect2i() {}

Rect2i::Rect2i(const Rect2 &p_r2) :
		position(p_r2.position),
		size(p_r2.size) {
}

Rect2i::Rect2i(int p_x, int p_y, int p_width, int p_height) :
		position(Point2(p_x, p_y)),
		size(Size2(p_width, p_height)) {
}

Rect2i::Rect2i(const Point2 &p_pos, const Size2 &p_size) :
		position(p_pos),
		size(p_size) {
}
