/*************************************************************************/
/*  method_ptrcall.h                                                     */
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

#ifndef METHOD_PTRCALL_H
#define METHOD_PTRCALL_H

#include "core/math/transform_2d.h"
#include "core/typedefs.h"
#include "core/variant.h"

#ifdef PTRCALL_ENABLED

template <class T>
struct PtrToArg {
};

#define MAKE_PTRARG(m_type)                                            \
	template <>                                                        \
	struct PtrToArg<m_type> {                                          \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {      \
			return *reinterpret_cast<const m_type *>(p_ptr);           \
		}                                                              \
		_FORCE_INLINE_ static void encode(m_type p_val, void *p_ptr) { \
			*((m_type *)p_ptr) = p_val;                                \
		}                                                              \
	};                                                                 \
	template <>                                                        \
	struct PtrToArg<const m_type &> {                                  \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {      \
			return *reinterpret_cast<const m_type *>(p_ptr);           \
		}                                                              \
		_FORCE_INLINE_ static void encode(m_type p_val, void *p_ptr) { \
			*((m_type *)p_ptr) = p_val;                                \
		}                                                              \
	}

#define MAKE_PTRARGCONV(m_type, m_conv)                                           \
	template <>                                                                   \
	struct PtrToArg<m_type> {                                                     \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {                 \
			return static_cast<m_type>(*reinterpret_cast<const m_conv *>(p_ptr)); \
		}                                                                         \
		_FORCE_INLINE_ static void encode(m_type p_val, void *p_ptr) {            \
			*((m_conv *)p_ptr) = static_cast<m_conv>(p_val);                      \
		}                                                                         \
	};                                                                            \
	template <>                                                                   \
	struct PtrToArg<const m_type &> {                                             \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {                 \
			return static_cast<m_type>(*reinterpret_cast<const m_conv *>(p_ptr)); \
		}                                                                         \
		_FORCE_INLINE_ static void encode(m_type p_val, void *p_ptr) {            \
			*((m_conv *)p_ptr) = static_cast<m_conv>(p_val);                      \
		}                                                                         \
	}

#define MAKE_PTRARG_BY_REFERENCE(m_type)                                      \
	template <>                                                               \
	struct PtrToArg<m_type> {                                                 \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {             \
			return *reinterpret_cast<const m_type *>(p_ptr);                  \
		}                                                                     \
		_FORCE_INLINE_ static void encode(const m_type &p_val, void *p_ptr) { \
			*((m_type *)p_ptr) = p_val;                                       \
		}                                                                     \
	};                                                                        \
	template <>                                                               \
	struct PtrToArg<const m_type &> {                                         \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {             \
			return *reinterpret_cast<const m_type *>(p_ptr);                  \
		}                                                                     \
		_FORCE_INLINE_ static void encode(const m_type &p_val, void *p_ptr) { \
			*((m_type *)p_ptr) = p_val;                                       \
		}                                                                     \
	}

MAKE_PTRARG(bool);
MAKE_PTRARGCONV(uint8_t, int64_t);
MAKE_PTRARGCONV(int8_t, int64_t);
MAKE_PTRARGCONV(uint16_t, int64_t);
MAKE_PTRARGCONV(int16_t, int64_t);
MAKE_PTRARGCONV(uint32_t, int64_t);
MAKE_PTRARGCONV(int32_t, int64_t);
MAKE_PTRARG(int64_t);
MAKE_PTRARG(uint64_t);
MAKE_PTRARGCONV(float, double);
MAKE_PTRARG(double);

MAKE_PTRARG(String);
MAKE_PTRARG(Vector2);
MAKE_PTRARG(Rect2);
MAKE_PTRARG_BY_REFERENCE(Vector3);
MAKE_PTRARG(Transform2D);
MAKE_PTRARG_BY_REFERENCE(Plane);
MAKE_PTRARG(Quat);
MAKE_PTRARG_BY_REFERENCE(AABB);
MAKE_PTRARG_BY_REFERENCE(Basis);
MAKE_PTRARG_BY_REFERENCE(Transform);
MAKE_PTRARG_BY_REFERENCE(Color);
MAKE_PTRARG(NodePath);
MAKE_PTRARG(RID);
MAKE_PTRARG(Dictionary);
MAKE_PTRARG(Array);
MAKE_PTRARG(PoolByteArray);
MAKE_PTRARG(PoolIntArray);
MAKE_PTRARG(PoolRealArray);
MAKE_PTRARG(PoolStringArray);
MAKE_PTRARG(PoolVector2Array);
MAKE_PTRARG(PoolVector3Array);
MAKE_PTRARG(PoolColorArray);
MAKE_PTRARG_BY_REFERENCE(Variant);

//this is for Object

template <class T>
struct PtrToArg<T *> {

	_FORCE_INLINE_ static T *convert(const void *p_ptr) {

		return const_cast<T *>(reinterpret_cast<const T *>(p_ptr));
	}

	_FORCE_INLINE_ static void encode(T *p_var, void *p_ptr) {

		*((T **)p_ptr) = p_var;
	}
};

template <class T>
struct PtrToArg<const T *> {

	_FORCE_INLINE_ static const T *convert(const void *p_ptr) {

		return reinterpret_cast<const T *>(p_ptr);
	}

	_FORCE_INLINE_ static void encode(T *p_var, void *p_ptr) {

		*((T **)p_ptr) = p_var;
	}
};

//this is for the special cases used by Variant

#define MAKE_VECARG(m_type)                                                              \
	template <>                                                                          \
	struct PtrToArg<Vector<m_type> > {                                                   \
		_FORCE_INLINE_ static Vector<m_type> convert(const void *p_ptr) {                \
			const Vector<m_type> *dvs = reinterpret_cast<const Vector<m_type> *>(p_ptr); \
			Vector<m_type> ret;                                                          \
			int len = dvs->size();                                                       \
			ret.resize(len);                                                             \
			{                                                                            \
				Vector<m_type>::Read r = dvs->read();                                    \
				for (int i = 0; i < len; i++) {                                          \
					ret[i] = r[i];                                                       \
				}                                                                        \
			}                                                                            \
			return ret;                                                                  \
		}                                                                                \
		_FORCE_INLINE_ static void encode(Vector<m_type> p_vec, void *p_ptr) {           \
			Vector<m_type> *dv = reinterpret_cast<Vector<m_type> *>(p_ptr);              \
			int len = p_vec.size();                                                      \
			dv->resize(len);                                                             \
			{                                                                            \
				Vector<m_type>::Write w = dv->write();                                   \
				for (int i = 0; i < len; i++) {                                          \
					w[i] = p_vec[i];                                                     \
				}                                                                        \
			}                                                                            \
		}                                                                                \
	};                                                                                   \
	template <>                                                                          \
	struct PtrToArg<const Vector<m_type> &> {                                            \
		_FORCE_INLINE_ static Vector<m_type> convert(const void *p_ptr) {                \
			const Vector<m_type> *dvs = reinterpret_cast<const Vector<m_type> *>(p_ptr); \
			Vector<m_type> ret;                                                          \
			int len = dvs->size();                                                       \
			ret.resize(len);                                                             \
			{                                                                            \
				Vector<m_type>::Read r = dvs->read();                                    \
				for (int i = 0; i < len; i++) {                                          \
					ret[i] = r[i];                                                       \
				}                                                                        \
			}                                                                            \
			return ret;                                                                  \
		}                                                                                \
	}

#define MAKE_VECARG_ALT(m_type, m_type_alt)                                              \
	template <>                                                                          \
	struct PtrToArg<Vector<m_type_alt> > {                                               \
		_FORCE_INLINE_ static Vector<m_type_alt> convert(const void *p_ptr) {            \
			const Vector<m_type> *dvs = reinterpret_cast<const Vector<m_type> *>(p_ptr); \
			Vector<m_type_alt> ret;                                                      \
			int len = dvs->size();                                                       \
			ret.resize(len);                                                             \
			{                                                                            \
				Vector<m_type>::Read r = dvs->read();                                    \
				for (int i = 0; i < len; i++) {                                          \
					ret[i] = r[i];                                                       \
				}                                                                        \
			}                                                                            \
			return ret;                                                                  \
		}                                                                                \
		_FORCE_INLINE_ static void encode(Vector<m_type_alt> p_vec, void *p_ptr) {       \
			Vector<m_type> *dv = reinterpret_cast<Vector<m_type> *>(p_ptr);              \
			int len = p_vec.size();                                                      \
			dv->resize(len);                                                             \
			{                                                                            \
				Vector<m_type>::Write w = dv->write();                                   \
				for (int i = 0; i < len; i++) {                                          \
					w[i] = p_vec[i];                                                     \
				}                                                                        \
			}                                                                            \
		}                                                                                \
	};                                                                                   \
	template <>                                                                          \
	struct PtrToArg<const Vector<m_type_alt> &> {                                        \
		_FORCE_INLINE_ static Vector<m_type_alt> convert(const void *p_ptr) {            \
			const Vector<m_type> *dvs = reinterpret_cast<const Vector<m_type> *>(p_ptr); \
			Vector<m_type_alt> ret;                                                      \
			int len = dvs->size();                                                       \
			ret.resize(len);                                                             \
			{                                                                            \
				Vector<m_type>::Read r = dvs->read();                                    \
				for (int i = 0; i < len; i++) {                                          \
					ret[i] = r[i];                                                       \
				}                                                                        \
			}                                                                            \
			return ret;                                                                  \
		}                                                                                \
	}

//for stuff that gets converted to Array vectors
#define MAKE_VECARR(m_type)                                                    \
	template <>                                                                \
	struct PtrToArg<Vector<m_type> > {                                         \
		_FORCE_INLINE_ static Vector<m_type> convert(const void *p_ptr) {      \
			const Array *arr = reinterpret_cast<const Array *>(p_ptr);         \
			Vector<m_type> ret;                                                \
			int len = arr->size();                                             \
			ret.resize(len);                                                   \
			for (int i = 0; i < len; i++) {                                    \
				ret[i] = (*arr)[i];                                            \
			}                                                                  \
			return ret;                                                        \
		}                                                                      \
		_FORCE_INLINE_ static void encode(Vector<m_type> p_vec, void *p_ptr) { \
			Array *arr = reinterpret_cast<Array *>(p_ptr);                     \
			int len = p_vec.size();                                            \
			arr->resize(len);                                                  \
			for (int i = 0; i < len; i++) {                                    \
				(*arr)[i] = p_vec[i];                                          \
			}                                                                  \
		}                                                                      \
	};                                                                         \
	template <>                                                                \
	struct PtrToArg<const Vector<m_type> &> {                                  \
		_FORCE_INLINE_ static Vector<m_type> convert(const void *p_ptr) {      \
			const Array *arr = reinterpret_cast<const Array *>(p_ptr);         \
			Vector<m_type> ret;                                                \
			int len = arr->size();                                             \
			ret.resize(len);                                                   \
			for (int i = 0; i < len; i++) {                                    \
				ret[i] = (*arr)[i];                                            \
			}                                                                  \
			return ret;                                                        \
		}                                                                      \
	}

MAKE_VECARR(Variant);
MAKE_VECARR(RID);
MAKE_VECARR(Plane);

#define MAKE_DVECARR(m_type)                                                   \
	template <>                                                                \
	struct PtrToArg<Vector<m_type> > {                                         \
		_FORCE_INLINE_ static Vector<m_type> convert(const void *p_ptr) {      \
			const Array *arr = reinterpret_cast<const Array *>(p_ptr);         \
			Vector<m_type> ret;                                                \
			int len = arr->size();                                             \
			ret.resize(len);                                                   \
			{                                                                  \
				Vector<m_type>::Write w = ret.write();                         \
				for (int i = 0; i < len; i++) {                                \
					w[i] = (*arr)[i];                                          \
				}                                                              \
			}                                                                  \
			return ret;                                                        \
		}                                                                      \
		_FORCE_INLINE_ static void encode(Vector<m_type> p_vec, void *p_ptr) { \
			Array *arr = reinterpret_cast<Array *>(p_ptr);                     \
			int len = p_vec.size();                                            \
			arr->resize(len);                                                  \
			{                                                                  \
				Vector<m_type>::Read r = p_vec.read();                         \
				for (int i = 0; i < len; i++) {                                \
					(*arr)[i] = r[i];                                          \
				}                                                              \
			}                                                                  \
		}                                                                      \
	};                                                                         \
	template <>                                                                \
	struct PtrToArg<const Vector<m_type> &> {                                  \
		_FORCE_INLINE_ static Vector<m_type> convert(const void *p_ptr) {      \
			const Array *arr = reinterpret_cast<const Array *>(p_ptr);         \
			Vector<m_type> ret;                                                \
			int len = arr->size();                                             \
			ret.resize(len);                                                   \
			{                                                                  \
				Vector<m_type>::Write w = ret.write();                         \
				for (int i = 0; i < len; i++) {                                \
					w[i] = (*arr)[i];                                          \
				}                                                              \
			}                                                                  \
			return ret;                                                        \
		}                                                                      \
	}

//for special case StringName

#define MAKE_STRINGCONV(m_type)                                        \
	template <>                                                        \
	struct PtrToArg<m_type> {                                          \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {      \
			m_type s = *reinterpret_cast<const String *>(p_ptr);       \
			return s;                                                  \
		}                                                              \
		_FORCE_INLINE_ static void encode(m_type p_vec, void *p_ptr) { \
			String *arr = reinterpret_cast<String *>(p_ptr);           \
			*arr = p_vec;                                              \
		}                                                              \
	};                                                                 \
                                                                       \
	template <>                                                        \
	struct PtrToArg<const m_type &> {                                  \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {      \
			m_type s = *reinterpret_cast<const String *>(p_ptr);       \
			return s;                                                  \
		}                                                              \
	}

#define MAKE_STRINGCONV_BY_REFERENCE(m_type)                                  \
	template <>                                                               \
	struct PtrToArg<m_type> {                                                 \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {             \
			m_type s = *reinterpret_cast<const String *>(p_ptr);              \
			return s;                                                         \
		}                                                                     \
		_FORCE_INLINE_ static void encode(const m_type &p_vec, void *p_ptr) { \
			String *arr = reinterpret_cast<String *>(p_ptr);                  \
			*arr = p_vec;                                                     \
		}                                                                     \
	};                                                                        \
                                                                              \
	template <>                                                               \
	struct PtrToArg<const m_type &> {                                         \
		_FORCE_INLINE_ static m_type convert(const void *p_ptr) {             \
			m_type s = *reinterpret_cast<const String *>(p_ptr);              \
			return s;                                                         \
		}                                                                     \
	}

MAKE_STRINGCONV(StringName);
MAKE_STRINGCONV_BY_REFERENCE(IP_Address);

template <>
struct PtrToArg<Vector<Face3> > {
	_FORCE_INLINE_ static Vector<Face3> convert(const void *p_ptr) {
		const Vector<Vector3> *dvs = reinterpret_cast<const Vector<Vector3> *>(p_ptr);
		Vector<Face3> ret;
		int len = dvs->size() / 3;
		ret.resize(len);
		{
			for (int i = 0; i < len; i++) {
				ret[i].vertex[0] = dvs->operator[](i * 3 + 0);
				ret[i].vertex[1] = dvs->operator[](i * 3 + 1);
				ret[i].vertex[2] = dvs->operator[](i * 3 + 2);
			}
		}
		return ret;
	}
	_FORCE_INLINE_ static void encode(Vector<Face3> p_vec, void *p_ptr) {
		Vector<Vector3> *arr = reinterpret_cast<Vector<Vector3> *>(p_ptr);
		int len = p_vec.size();
		arr->resize(len * 3);
		{
			for (int i = 0; i < len; i++) {
				arr->operator[](i * 3 + 0) = p_vec[i].vertex[0];
				arr->operator[](i * 3 + 1) = p_vec[i].vertex[1];
				arr->operator[](i * 3 + 2) = p_vec[i].vertex[2];
			}
		}
	}
};
template <>
struct PtrToArg<const Vector<Face3> &> {
	_FORCE_INLINE_ static Vector<Face3> convert(const void *p_ptr) {
		const Vector<Vector3> *dvs = reinterpret_cast<const Vector<Vector3> *>(p_ptr);
		Vector<Face3> ret;
		int len = dvs->size() / 3;
		ret.resize(len);
		{
			for (int i = 0; i < len; i++) {
				ret[i].vertex[0] = dvs->operator[](i * 3 + 0);
				ret[i].vertex[1] = dvs->operator[](i * 3 + 1);
				ret[i].vertex[2] = dvs->operator[](i * 3 + 2);
			}
		}
		return ret;
	}
};

#endif // METHOD_PTRCALL_H
#endif
