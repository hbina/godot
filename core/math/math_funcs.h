/*************************************************************************/
/*  math_funcs.h                                                         */
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

#pragma once

#include "core/math/math_defs.h"
#include "core/math/random_pcg.h"
#include "core/typedefs.h"

#include "thirdparty/gcem/include/gcem.hpp"
#include "thirdparty/misc/pcg.h"

#include <float.h>
#include <math.h>
#include <array>
#include <type_traits>

class Math {

	static RandomPCG default_rand;

public:
	Math() = default;

	static const uint64_t RANDOM_MAX = 0xFFFFFFFF;

	/**
	 * CONSTEXPR STATIC MATH FUNCTIONS
	 */

	static constexpr double sin(const double &p_x) { return gcem::sin(p_x); }

	static constexpr double cos(const double &p_x) { return gcem::cos(p_x); }

	static constexpr double tan(const double &p_x) { return gcem::tan(p_x); }

	static constexpr double sinh(const double &p_x) { return gcem::sin(p_x); }

	static constexpr double sinc(const double &p_x) { return p_x == 0 ? 1 : gcem::sin(p_x) / p_x; }

	static constexpr double sincn(const double &p_x) { return sinc(Math_PI * p_x); }

	static constexpr double cosh(const double &p_x) { return gcem::cosh(p_x); }

	static constexpr double tanh(const double &p_x) { return gcem::tanh(p_x); }

	static constexpr double asin(const double &p_x) { return gcem::asin(p_x); }

	static constexpr double acos(const double &p_x) { return gcem::acos(p_x); }

	static constexpr double atan(const double &p_x) { return gcem::atan(p_x); }

	static constexpr double atan2(const double &p_y, const double &p_x) { return gcem::atan2(p_y, p_x); }

	static constexpr double sqrt(const double &p_x) { return gcem::sqrt(p_x); }

	static constexpr double floor(const double &p_x) { return gcem::floor(p_x); }

	static constexpr double ceil(const double &p_x) { return gcem::ceil(p_x); }

	static constexpr double pow(const double &p_x, const double &p_y) { return gcem::pow(p_x, p_y); }

	static constexpr double log(const double &p_x) { return gcem::log(p_x); }

	static constexpr double exp(const double &p_x) { return gcem::exp(p_x); }

	static constexpr bool is_nan(const double &p_val) { return gcem::internal::is_nan(p_val); }

	static constexpr bool is_inf(const double &p_val) { return gcem::internal::is_inf(p_val); }

	static constexpr double abs(const double &g) { return gcem::abs(g); }

	static constexpr double deg2rad(const double &p_y) { return p_y * Math_PI / 180.0; }

	static constexpr double rad2deg(const double &p_y) { return p_y * 180.0 / Math_PI; }

	static constexpr double lerp(const double &p_from, const double &p_to, const double &p_weight) { return p_from + (p_to - p_from) * p_weight; }

	static constexpr int posmod(const int &p_x, const int &p_y) {
		int value = p_x % p_y;
		if ((value < 0 && p_y > 0) || (value > 0 && p_y < 0)) {
			value += p_y;
		}
		return value;
	}

	static constexpr float inverse_lerp(float p_from, float p_to, float p_value) { return (p_value - p_from) / (p_to - p_from); }

	static constexpr double range_lerp(double p_value, double p_istart, double p_istop, double p_ostart, double p_ostop) { return Math::lerp(p_ostart, p_ostop, Math::inverse_lerp(p_istart, p_istop, p_value)); }
	static constexpr float range_lerp(float p_value, float p_istart, float p_istop, float p_ostart, float p_ostop) { return Math::lerp(p_ostart, p_ostop, Math::inverse_lerp(p_istart, p_istop, p_value)); }

	static constexpr double smoothstep(double p_from, double p_to, double p_weight) {
		if (is_equal_approx(p_from, p_to))
			return p_from;
		double x = CLAMP((p_weight - p_from) / (p_to - p_from), 0.0, 1.0);
		return x * x * (3.0 - 2.0 * x);
	}
	static constexpr float smoothstep(float p_from, float p_to, float p_weight) {
		if (is_equal_approx(p_from, p_to))
			return p_from;
		float x = CLAMP((p_weight - p_from) / (p_to - p_from), 0.0f, 1.0f);
		return x * x * (3.0f - 2.0f * x);
	}

	static constexpr double move_toward(double p_from, double p_to, double p_delta) { return abs(p_to - p_from) <= p_delta ? p_to : p_from + SGN(p_to - p_from) * p_delta; }
	static constexpr float move_toward(float p_from, float p_to, float p_delta) { return abs(p_to - p_from) <= p_delta ? p_to : p_from + SGN(p_to - p_from) * p_delta; }

	static constexpr double linear2db(const double &p_linear) { return Math::log(p_linear) * 8.6858896380650365530225783783321; }

	static constexpr double db2linear(const double &p_db) { return Math::exp(p_db * 0.11512925464970228420089957273422); }

	static constexpr double round(const double &p_val) { return (p_val >= 0) ? Math::floor(p_val + 0.5) : -Math::floor(-p_val + 0.5); }

	static constexpr int wrapi(const int &value, const int &min, const int &max) {
		int range = max - min;
		return range == 0 ? min : min + ((((value - min) % range) + range) % range);
	}

	static constexpr double wrapf(const double &value, const double &min, const double &max) {
		double range = max - min;
		return is_zero_approx(range) ? min : value - (range * Math::floor((value - min) / range));
	}

	// NOTE :: 	This function is not a template because the calculation inside is done in `double`
	// TODO ::	Consider using const T&
	static constexpr double ease(double p_x, double p_c) {
		if (p_x < 0)
			p_x = 0;
		else if (p_x > 1.0)
			p_x = 1.0;
		if (p_c > 0) {
			if (p_c < 1.0) {
				return 1.0 - Math::pow(1.0 - p_x, 1.0 / p_c);
			} else {
				return Math::pow(p_x, p_c);
			}
		} else if (p_c < 0) {
			//inout ease

			if (p_x < 0.5) {
				return Math::pow(p_x * 2.0, -p_c) * 0.5;
			} else {
				return (1.0 - Math::pow(1.0 - (p_x - 0.5) * 2.0, -p_c)) * 0.5 + 0.5;
			}
		} else
			return 0.0; // no ease (raw)
	};

	static constexpr double dectime(const double &p_value, const double &p_amount, const double &p_step) {
		double sgn = p_value < 0 ? -1.0 : 1.0;
		double val = Math::abs(p_value);
		val -= p_amount * p_step;
		if (val < 0.0)
			val = 0.0;
		return val * sgn;
	}

	static constexpr double stepify(const double &p_value, const double &p_step) {
		if (p_step != 0) {
			return Math::floor(p_value / p_step + 0.5) * p_step;
		}
		return p_value;
	};

	static constexpr bool is_equal_approx_ratio(const real_t &a, const real_t &b, const real_t &epsilon = CMP_EPSILON, const real_t &min_epsilon = CMP_EPSILON) {
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

	static constexpr bool is_equal_approx(const real_t &a, const real_t &b) {
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

	static constexpr bool is_equal_approx(const real_t &a, const real_t &b, const real_t &tolerance) {
		// Check for exact equality first, required to handle "infinity" values.
		if (a == b) {
			return true;
		}
		// Then check for approximate equality.
		return abs(a - b) < tolerance;
	}

	static constexpr bool is_zero_approx(const real_t &s) {
		return abs(s) < CMP_EPSILON;
	}

	// TODO :: do we still need this?
	static constexpr float absf(const float &g) {
		return gcem::abs(g);
	}

	static constexpr double absd(const double &g) {
		return gcem::abs(g);
	}

	//this function should be as fast as possible and rounding mode should not matter
	static constexpr int fast_ftoi(const double &a) {
		return static_cast<int>(a);
	}

	static constexpr double snap_scalar(const double &p_offset, const double &p_step, const double &p_target) {
		return p_step != 0 ? Math::stepify(p_target - p_offset, p_step) + p_offset : p_target;
	}

	static constexpr double snap_scalar_separation(const double &p_offset, const double &p_step, const double &p_target, const double &p_separation) {
		if (p_step != 0) {
			double a = Math::stepify(p_target - p_offset, p_step + p_separation) + p_offset;
			double b = a;
			if (p_target >= 0)
				b -= p_separation;
			else
				b += p_step;
			return (Math::abs(p_target - a) < Math::abs(p_target - b)) ? a : b;
		}
		return p_target;
	}

	static constexpr double fmod(const double &p_x, const double &p_y) {
		double mod = p_x;
		double local_y = p_y;
		// Handling negative values
		if (p_x < 0)
			mod = -p_x;
		if (p_y < 0)
			local_y = -p_y;

		// Finding mod by repeated subtraction

		while (mod >= local_y)
			mod = mod - local_y;

		// Sign of result typically depends
		// on sign of p_x.
		if (p_x < 0)
			return -mod;

		return mod;
	}

	static constexpr double fposmod(const double &p_x, const double &p_y) {
		double value = Math::fmod(p_x, p_y);
		if ((value < 0 && p_y > 0) || (value > 0 && p_y < 0)) {
			value += p_y;
		}
		value += 0.0;
		return value;
	}

	// FIXME :: Once `constexpr fmod` is implemented, make this constexpr as well...
	static constexpr double lerp_angle(const double &p_from, const double &p_to, const double &p_weight) {
		double difference = fmod(p_to - p_from, Math_TAU);
		double distance = fmod(2.0 * difference, Math_TAU) - difference;
		return p_from + distance * p_weight;
	}

	static int step_decimals(const double &p_step) {
		static const std::array<double, 10> sd = {
			0.9999, // somehow compensate for floating point error
			0.09999,
			0.009999,
			0.0009999,
			0.00009999,
			0.000009999,
			0.0000009999,
			0.00000009999,
			0.000000009999,
			0.0000000009999
		};

		double abs = Math::abs(p_step);
		double decs = abs - static_cast<int>(abs); // Strip away integer part
		int x = 0;
		for (const auto &i : sd) {
			if (decs >= i) {
				return x;
			}
			x++;
		}

		return 0;
	};

	static constexpr int range_step_decimals(const double &p_step) {
		if (p_step < 0.0000000000001) {
			return 16; // Max value hardcoded in String::num
		}
		return step_decimals(p_step);
	};

	// TODO ::	This can be made into constexpr function by using recursive templates
	static uint32_t larger_prime(const uint32_t &p_val) {
		static const std::array<uint32_t, 30> primes = {
			5,
			13,
			23,
			47,
			97,
			193,
			389,
			769,
			1543,
			3079,
			6151,
			12289,
			24593,
			49157,
			98317,
			196613,
			393241,
			786433,
			1572869,
			3145739,
			6291469,
			12582917,
			25165843,
			50331653,
			100663319,
			201326611,
			402653189,
			805306457,
			1610612741,
			0,
		};

		for (const auto &idx : primes) {
			if (idx > p_val)
				return idx;
		}
		return 0;
	};

	/**
	 * NON-CONSTEXPR STATIC MATH FUNCTIONS
	 */
	static void seed(uint64_t x);
	static void randomize();
	static uint32_t rand_from_seed(uint64_t *seed);
	static uint32_t rand();
	static double randd() {
		return (double)rand() / (double)Math::RANDOM_MAX;
	}
	static float randf() {
		return (float)rand() / (float)Math::RANDOM_MAX;
	}

	static double random(double from, double to);
	static float random(float from, float to);
	static real_t random(int from, int to) {
		return (real_t)random((real_t)from, (real_t)to);
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
};
