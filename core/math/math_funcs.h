/*************************************************************************/
/*  math_funcs.h                                                         */
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

#ifndef MATH_FUNCS_H
#define MATH_FUNCS_H

#include "core/math/math_defs.h"
#include "core/math/random_pcg.h"
#include "core/typedefs.h"

#include "thirdparty/gcem/include/gcem.hpp"
#include "thirdparty/misc/pcg.h"

#include <float.h>
#include <math.h>

class Math {

	static RandomPCG default_rand;

public:
	Math() {} // useless to instance

	static const uint64_t RANDOM_MAX = 0xFFFFFFFF;

	constexpr static double sin(double p_x) { return gcem::sin(p_x); }
	constexpr static float sin(float p_x) { return gcem::sin(p_x); }

	constexpr static double cos(double p_x) { return gcem::cos(p_x); }
	constexpr static float cos(float p_x) { return gcem::cos(p_x); }

	constexpr static double tan(double p_x) { return gcem::tan(p_x); }
	constexpr static float tan(float p_x) { return gcem::tan(p_x); }

	constexpr static double sinh(double p_x) { return gcem::sin(p_x); }
	constexpr static float sinh(float p_x) { return gcem::sinh(p_x); }

	constexpr static float sinc(float p_x) { return p_x == 0 ? 1 : gcem::sin(p_x) / p_x; }
	constexpr static double sinc(double p_x) { return p_x == 0 ? 1 : gcem::sin(p_x) / p_x; }

	constexpr static float sincn(float p_x) { return sinc(Math_PI * p_x); }
	constexpr static double sincn(double p_x) { return sinc(Math_PI * p_x); }

	constexpr static double cosh(double p_x) { return gcem::cosh(p_x); }
	constexpr static float cosh(float p_x) { return gcem::cosh(p_x); }

	constexpr static double tanh(double p_x) { return gcem::tanh(p_x); }
	constexpr static float tanh(float p_x) { return gcem::tanh(p_x); }

	constexpr static double asin(double p_x) { return gcem::asin(p_x); }
	constexpr static float asin(float p_x) { return gcem::asin(p_x); }

	constexpr static double acos(double p_x) { return gcem::acos(p_x); }
	constexpr static float acos(float p_x) { return gcem::acos(p_x); }

	constexpr static double atan(double p_x) { return gcem::atan(p_x); }
	constexpr static float atan(float p_x) { return gcem::atan(p_x); }

	constexpr static double atan2(double p_y, double p_x) { return gcem::atan2(p_y, p_x); }
	constexpr static float atan2(float p_y, float p_x) { return gcem::atan2(p_y, p_x); }

	constexpr static double sqrt(double p_x) { return gcem::sqrt(p_x); }
	constexpr static float sqrt(float p_x) { return gcem::sqrt(p_x); }

	// TODO :: 	It should be possible to make this constexpr
	//			We just have to make PR in gcem
	static double fmod(double p_x, double p_y) { return ::fmod(p_x, p_y); }
	static float fmod(float p_x, float p_y) { return ::fmodf(p_x, p_y); }

	constexpr static double floor(double p_x) { return gcem::floor(p_x); }
	constexpr static float floor(float p_x) { return gcem::floor(p_x); }

	constexpr static double ceil(double p_x) { return gcem::ceil(p_x); }
	constexpr static float ceil(float p_x) { return gcem::ceil(p_x); }

	constexpr static double pow(double p_x, double p_y) { return gcem::pow(p_x, p_y); }
	constexpr static float pow(float p_x, float p_y) { return gcem::pow(p_x, p_y); }

	constexpr static double log(double p_x) { return gcem::log(p_x); }
	constexpr static float log(float p_x) { return gcem::log(p_x); }

	constexpr static double exp(double p_x) { return gcem::exp(p_x); }
	constexpr static float exp(float p_x) { return gcem::exp(p_x); }

	constexpr static bool is_nan(double p_val) { return gcem::internal::is_nan(p_val); }

	constexpr static bool is_nan(float p_val) { return gcem::internal::is_nan(p_val); }

	constexpr static bool is_inf(double p_val) { return gcem::internal::is_inf(p_val); }

	constexpr static bool is_inf(float p_val) { return gcem::internal::is_inf(p_val); }

	constexpr static double abs(double g) { return gcem::abs(g); }
	constexpr static float abs(float g) { return gcem::abs(g); }
	constexpr static int abs(int g) { return gcem::abs(g); }

	static double fposmod(double p_x, double p_y) {
		double value = Math::fmod(p_x, p_y);
		if ((value < 0 && p_y > 0) || (value > 0 && p_y < 0)) {
			value += p_y;
		}
		value += 0.0;
		return value;
	}
	static float fposmod(float p_x, float p_y) {
		float value = Math::fmod(p_x, p_y);
		if ((value < 0 && p_y > 0) || (value > 0 && p_y < 0)) {
			value += p_y;
		}
		value += 0.0;
		return value;
	}

	constexpr static int posmod(int p_x, int p_y) {
		int value = p_x % p_y;
		if ((value < 0 && p_y > 0) || (value > 0 && p_y < 0)) {
			value += p_y;
		}
		return value;
	}

	constexpr static double deg2rad(double p_y) { return p_y * Math_PI / 180.0; }
	constexpr static float deg2rad(float p_y) { return p_y * Math_PI / 180.0; }

	constexpr static double rad2deg(double p_y) { return p_y * 180.0 / Math_PI; }
	constexpr static float rad2deg(float p_y) { return p_y * 180.0 / Math_PI; }

	constexpr static double lerp(double p_from, double p_to, double p_weight) { return p_from + (p_to - p_from) * p_weight; }
	constexpr static float lerp(float p_from, float p_to, float p_weight) { return p_from + (p_to - p_from) * p_weight; }

	static double lerp_angle(double p_from, double p_to, double p_weight) {
		double difference = fmod(p_to - p_from, Math_TAU);
		double distance = fmod(2.0 * difference, Math_TAU) - difference;
		return p_from + distance * p_weight;
	}
	static float lerp_angle(float p_from, float p_to, float p_weight) {
		float difference = fmod(p_to - p_from, (float)Math_TAU);
		float distance = fmod(2.0f * difference, (float)Math_TAU) - difference;
		return p_from + distance * p_weight;
	}

	static double inverse_lerp(double p_from, double p_to, double p_value) { return (p_value - p_from) / (p_to - p_from); }
	static float inverse_lerp(float p_from, float p_to, float p_value) { return (p_value - p_from) / (p_to - p_from); }

	static double range_lerp(double p_value, double p_istart, double p_istop, double p_ostart, double p_ostop) { return Math::lerp(p_ostart, p_ostop, Math::inverse_lerp(p_istart, p_istop, p_value)); }
	static float range_lerp(float p_value, float p_istart, float p_istop, float p_ostart, float p_ostop) { return Math::lerp(p_ostart, p_ostop, Math::inverse_lerp(p_istart, p_istop, p_value)); }

	constexpr static double smoothstep(double p_from, double p_to, double p_weight) {
		if (is_equal_approx(p_from, p_to)) return p_from;
		double x = CLAMP((p_weight - p_from) / (p_to - p_from), 0.0, 1.0);
		return x * x * (3.0 - 2.0 * x);
	}
	constexpr static float smoothstep(float p_from, float p_to, float p_weight) {
		if (is_equal_approx(p_from, p_to)) return p_from;
		float x = CLAMP((p_weight - p_from) / (p_to - p_from), 0.0f, 1.0f);
		return x * x * (3.0f - 2.0f * x);
	}

	constexpr static double move_toward(double p_from, double p_to, double p_delta) { return gcem::abs(p_to - p_from) <= p_delta ? p_to : p_from + SGN(p_to - p_from) * p_delta; }
	constexpr static float move_toward(float p_from, float p_to, float p_delta) { return gcem::abs(p_to - p_from) <= p_delta ? p_to : p_from + SGN(p_to - p_from) * p_delta; }

	constexpr static double linear2db(double p_linear) { return Math::log(p_linear) * 8.6858896380650365530225783783321; }
	constexpr static float linear2db(float p_linear) { return Math::log(p_linear) * 8.6858896380650365530225783783321; }

	constexpr static double db2linear(double p_db) { return Math::exp(p_db * 0.11512925464970228420089957273422); }
	constexpr static float db2linear(float p_db) { return Math::exp(p_db * 0.11512925464970228420089957273422); }

	constexpr static double round(double p_val) { return (p_val >= 0) ? Math::floor(p_val + 0.5) : -Math::floor(-p_val + 0.5); }
	constexpr static float round(float p_val) { return (p_val >= 0) ? Math::floor(p_val + 0.5) : -Math::floor(-p_val + 0.5); }

	constexpr static int64_t wrapi(int64_t value, int64_t min, int64_t max) {
		int64_t range = max - min;
		return range == 0 ? min : min + ((((value - min) % range) + range) % range);
	}
	constexpr static double wrapf(double value, double min, double max) {
		double range = max - min;
		return is_zero_approx(range) ? min : value - (range * Math::floor((value - min) / range));
	}
	constexpr static float wrapf(float value, float min, float max) {
		float range = max - min;
		return is_zero_approx(range) ? min : value - (range * Math::floor((value - min) / range));
	}

	// double only, as these functions are mainly used by the editor and not performance-critical,
	static double ease(double p_x, double p_c);
	static int step_decimals(double p_step);
	static int range_step_decimals(double p_step);
	static double stepify(double p_value, double p_step);
	static double dectime(double p_value, double p_amount, double p_step);

	static uint32_t larger_prime(uint32_t p_val);

	static void seed(uint64_t x);
	static void randomize();
	static uint32_t rand_from_seed(uint64_t *seed);
	static uint32_t rand();
	static double randd() { return (double)rand() / (double)Math::RANDOM_MAX; }
	static float randf() { return (float)rand() / (float)Math::RANDOM_MAX; }

	static double random(double from, double to);
	static float random(float from, float to);
	static real_t random(int from, int to) { return (real_t)random((real_t)from, (real_t)to); }

	constexpr static bool is_equal_approx_ratio(real_t a, real_t b, real_t epsilon = CMP_EPSILON, real_t min_epsilon = CMP_EPSILON) {
		// this is an approximate way to check that numbers are close, as a ratio of their average size
		// helps compare approximate numbers that may be very big or very small
		real_t diff = abs(a - b);
		if (diff == 0.0 || diff < min_epsilon) {
			return true;
		}
		real_t avg_size = (abs(a) + abs(b)) / 2.0;
		diff /= avg_size;
		return diff < epsilon;
	}

	constexpr static bool is_equal_approx(real_t a, real_t b) {
		// Check for exact equality first, required to handle "infinity" values.
		if (a == b) {
			return true;
		}
		// Then check for approximate equality.
		real_t tolerance = CMP_EPSILON * abs(a);
		if (tolerance < CMP_EPSILON) {
			tolerance = CMP_EPSILON;
		}
		return abs(a - b) < tolerance;
	}

	constexpr static bool is_equal_approx(real_t a, real_t b, real_t tolerance) {
		// Check for exact equality first, required to handle "infinity" values.
		if (a == b) {
			return true;
		}
		// Then check for approximate equality.
		return abs(a - b) < tolerance;
	}

	constexpr static bool is_zero_approx(real_t s) {
		return abs(s) < CMP_EPSILON;
	}

	// TODO :: do we still need this?
	constexpr static float absf(float g) {
		return gcem::abs(g);
	}

	constexpr static double absd(double g) {
		return gcem::abs(g);
	}

	//this function should be as fast as possible and rounding mode should not matter
	constexpr static int fast_ftoi(float a) {
		return static_cast<int>(a);
	}

	static uint32_t halfbits_to_floatbits(uint16_t h) {
		uint16_t h_exp, h_sig;
		uint32_t f_sgn, f_exp, f_sig;

		h_exp = (h & 0x7c00u);
		f_sgn = ((uint32_t)h & 0x8000u) << 16;
		switch (h_exp) {
			case 0x0000u: /* 0 or subnormal */
				h_sig = (h & 0x03ffu);
				/* Signed zero */
				if (h_sig == 0) {
					return f_sgn;
				}
				/* Subnormal */
				h_sig <<= 1;
				while ((h_sig & 0x0400u) == 0) {
					h_sig <<= 1;
					h_exp++;
				}
				f_exp = ((uint32_t)(127 - 15 - h_exp)) << 23;
				f_sig = ((uint32_t)(h_sig & 0x03ffu)) << 13;
				return f_sgn + f_exp + f_sig;
			case 0x7c00u: /* inf or NaN */
				/* All-ones exponent and a copy of the significand */
				return f_sgn + 0x7f800000u + (((uint32_t)(h & 0x03ffu)) << 13);
			default: /* normalized */
				/* Just need to adjust the exponent and shift */
				return f_sgn + (((uint32_t)(h & 0x7fffu) + 0x1c000u) << 13);
		}
	}

	static float halfptr_to_float(const uint16_t *h) {

		union {
			uint32_t u32;
			float f32;
		} u;

		u.u32 = halfbits_to_floatbits(*h);
		return u.f32;
	}

	static float half_to_float(const uint16_t h) {
		return halfptr_to_float(&h);
	}

	static uint16_t make_half_float(float f) {

		union {
			float fv;
			uint32_t ui;
		} ci;
		ci.fv = f;

		uint32_t x = ci.ui;
		uint32_t sign = (unsigned short)(x >> 31);
		uint32_t mantissa;
		uint32_t exp;
		uint16_t hf;

		// get mantissa
		mantissa = x & ((1 << 23) - 1);
		// get exponent bits
		exp = x & (0xFF << 23);
		if (exp >= 0x47800000) {
			// check if the original single precision float number is a NaN
			if (mantissa && (exp == (0xFF << 23))) {
				// we have a single precision NaN
				mantissa = (1 << 23) - 1;
			} else {
				// 16-bit half-float representation stores number as Inf
				mantissa = 0;
			}
			hf = (((uint16_t)sign) << 15) | (uint16_t)((0x1F << 10)) |
				 (uint16_t)(mantissa >> 13);
		}
		// check if exponent is <= -15
		else if (exp <= 0x38000000) {

			/*// store a denorm half-float value or zero
		exp = (0x38000000 - exp) >> 23;
		mantissa >>= (14 + exp);

		hf = (((uint16_t)sign) << 15) | (uint16_t)(mantissa);
		*/
			hf = 0; //denormals do not work for 3D, convert to zero
		} else {
			hf = (((uint16_t)sign) << 15) |
				 (uint16_t)((exp - 0x38000000) >> 13) |
				 (uint16_t)(mantissa >> 13);
		}

		return hf;
	}

	static float snap_scalar(float p_offset, float p_step, float p_target) {
		return p_step != 0 ? Math::stepify(p_target - p_offset, p_step) + p_offset : p_target;
	}

	static float snap_scalar_seperation(float p_offset, float p_step, float p_target, float p_separation) {
		if (p_step != 0) {
			float a = Math::stepify(p_target - p_offset, p_step + p_separation) + p_offset;
			float b = a;
			if (p_target >= 0)
				b -= p_separation;
			else
				b += p_step;
			return (Math::abs(p_target - a) < Math::abs(p_target - b)) ? a : b;
		}
		return p_target;
	}
};

#endif // MATH_FUNCS_H
