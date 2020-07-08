/*************************************************************************/
/*  typed_array.h                                                        */
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

#ifndef TYPED_ARRAY_H
#define TYPED_ARRAY_H

#include "core/array.h"
#include "core/method_ptrcall.h"
#include "core/variant.h"

template <class T>
class TypedArray : public Array {
public:
	template <class U>
	_FORCE_INLINE_ void operator=(const TypedArray<U> &p_array) {
		static_assert(__is_base_of(T, U));
		_assign(p_array);
	}

	_FORCE_INLINE_ void operator=(const Array &p_array) {
		_assign(p_array);
	}
	_FORCE_INLINE_ TypedArray(const Variant &p_variant) :
			Array(Array(p_variant), Variant::Type::OBJECT, T::get_class_static(), Variant()) {
	}
	_FORCE_INLINE_ TypedArray(const Array &p_array) :
			Array(p_array, Variant::Type::OBJECT, T::get_class_static(), Variant()) {
	}
	_FORCE_INLINE_ TypedArray() {
		set_typed(static_cast<uint32_t>(Variant::Type::OBJECT), T::get_class_static(), Variant());
	}
};

//specialization for the rest of variant types

#define MAKE_TYPED_ARRAY(m_type, m_variant_type)                                                          \
	template <>                                                                                           \
	class TypedArray<m_type> : public Array {                                                             \
	public:                                                                                               \
		_FORCE_INLINE_ void operator=(const Array &p_array) {                                             \
			_assign(p_array);                                                                             \
		}                                                                                                 \
		_FORCE_INLINE_ TypedArray(const Variant &p_variant) :                                             \
				Array(Array(p_variant), static_cast<uint32_t>(m_variant_type), StringName(), Variant()) { \
		}                                                                                                 \
		_FORCE_INLINE_ TypedArray(const Array &p_array) :                                                 \
				Array(p_array, static_cast<uint32_t>(m_variant_type), StringName(), Variant()) {          \
		}                                                                                                 \
		_FORCE_INLINE_ TypedArray() {                                                                     \
			set_typed(static_cast<uint32_t>(m_variant_type), StringName(), Variant());                    \
		}                                                                                                 \
	};

MAKE_TYPED_ARRAY(bool, Variant::Type::BOOL)
MAKE_TYPED_ARRAY(uint8_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(int8_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(uint16_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(int16_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(uint32_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(int32_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(uint64_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(int64_t, Variant::Type::INT)
MAKE_TYPED_ARRAY(float, Variant::Type::FLOAT)
MAKE_TYPED_ARRAY(double, Variant::Type::FLOAT)
MAKE_TYPED_ARRAY(String, Variant::Type::STRING)
MAKE_TYPED_ARRAY(Vector2, Variant::Type::VECTOR2)
MAKE_TYPED_ARRAY(Vector2i, Variant::Type::VECTOR2I)
MAKE_TYPED_ARRAY(Rect2, Variant::Type::RECT2)
MAKE_TYPED_ARRAY(Rect2i, Variant::Type::RECT2I)
MAKE_TYPED_ARRAY(Vector3, Variant::Type::VECTOR3)
MAKE_TYPED_ARRAY(Vector3i, Variant::Type::VECTOR3I)
MAKE_TYPED_ARRAY(Transform2D, Variant::Type::TRANSFORM2D)
MAKE_TYPED_ARRAY(Plane, Variant::Type::PLANE)
MAKE_TYPED_ARRAY(Quat, Variant::Type::QUAT)
MAKE_TYPED_ARRAY(AABB, Variant::Type::AABB)
MAKE_TYPED_ARRAY(Basis, Variant::Type::BASIS)
MAKE_TYPED_ARRAY(Transform, Variant::Type::TRANSFORM)
MAKE_TYPED_ARRAY(Color, Variant::Type::COLOR)
MAKE_TYPED_ARRAY(StringName, Variant::Type::STRING_NAME)
MAKE_TYPED_ARRAY(NodePath, Variant::Type::NODE_PATH)
MAKE_TYPED_ARRAY(RID, Variant::Type::_RID)
MAKE_TYPED_ARRAY(Callable, Variant::Type::CALLABLE)
MAKE_TYPED_ARRAY(Signal, Variant::Type::SIGNAL)
MAKE_TYPED_ARRAY(Dictionary, Variant::Type::DICTIONARY)
MAKE_TYPED_ARRAY(Array, Variant::Type::ARRAY)
MAKE_TYPED_ARRAY(Vector<uint8_t>, Variant::Type::PACKED_BYTE_ARRAY)
MAKE_TYPED_ARRAY(Vector<int32_t>, Variant::Type::PACKED_INT32_ARRAY)
MAKE_TYPED_ARRAY(Vector<int64_t>, Variant::Type::PACKED_INT64_ARRAY)
MAKE_TYPED_ARRAY(Vector<float>, Variant::Type::PACKED_FLOAT32_ARRAY)
MAKE_TYPED_ARRAY(Vector<double>, Variant::Type::PACKED_FLOAT64_ARRAY)
MAKE_TYPED_ARRAY(Vector<String>, Variant::Type::PACKED_STRING_ARRAY)
MAKE_TYPED_ARRAY(Vector<Vector2>, Variant::Type::PACKED_VECTOR2_ARRAY)
MAKE_TYPED_ARRAY(Vector<Vector3>, Variant::Type::PACKED_VECTOR3_ARRAY)
MAKE_TYPED_ARRAY(Vector<Color>, Variant::Type::PACKED_COLOR_ARRAY)

#ifdef PTRCALL_ENABLED

template <class T>
struct PtrToArg<TypedArray<T>> {
	_FORCE_INLINE_ static TypedArray<T> convert(const void *p_ptr) {
		return TypedArray<T>(*reinterpret_cast<const Array *>(p_ptr));
	}

	_FORCE_INLINE_ static void encode(TypedArray<T> p_val, void *p_ptr) {
		*(Array *)p_ptr = p_val;
	}
};

template <class T>
struct PtrToArg<const TypedArray<T> &> {
	_FORCE_INLINE_ static TypedArray<T> convert(const void *p_ptr) {
		return TypedArray<T>(*reinterpret_cast<const Array *>(p_ptr));
	}
};

#endif // PTRCALL_ENABLED

#ifdef DEBUG_METHODS_ENABLED

template <class T>
struct GetTypeInfo<TypedArray<T>> {
	static const Variant::Type VARIANT_TYPE = Variant::Type::ARRAY;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(Variant::Type::ARRAY, String(), PROPERTY_HINT_ARRAY_TYPE, T::get_class_static());
	}
};

template <class T>
struct GetTypeInfo<const TypedArray<T> &> {
	static const Variant::Type VARIANT_TYPE = Variant::Type::ARRAY;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(Variant::Type::ARRAY, String(), PROPERTY_HINT_ARRAY_TYPE, T::get_class_static());
	}
};

#define MAKE_TYPED_ARRAY_INFO(m_type, m_variant_type)                                                                              \
	template <>                                                                                                                    \
	struct GetTypeInfo<TypedArray<m_type>> {                                                                                       \
		static const Variant::Type VARIANT_TYPE = Variant::Type::ARRAY;                                                            \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;                                              \
		static inline PropertyInfo get_class_info() {                                                                              \
			return PropertyInfo(Variant::Type::ARRAY, String(), PROPERTY_HINT_ARRAY_TYPE, Variant::get_type_name(m_variant_type)); \
		}                                                                                                                          \
	};                                                                                                                             \
	template <>                                                                                                                    \
	struct GetTypeInfo<const TypedArray<m_type> &> {                                                                               \
		static const Variant::Type VARIANT_TYPE = Variant::Type::ARRAY;                                                            \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;                                              \
		static inline PropertyInfo get_class_info() {                                                                              \
			return PropertyInfo(Variant::Type::ARRAY, String(), PROPERTY_HINT_ARRAY_TYPE, Variant::get_type_name(m_variant_type)); \
		}                                                                                                                          \
	};

MAKE_TYPED_ARRAY_INFO(bool, Variant::Type::BOOL)
MAKE_TYPED_ARRAY_INFO(uint8_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(int8_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(uint16_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(int16_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(uint32_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(int32_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(uint64_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(int64_t, Variant::Type::INT)
MAKE_TYPED_ARRAY_INFO(float, Variant::Type::FLOAT)
MAKE_TYPED_ARRAY_INFO(double, Variant::Type::FLOAT)
MAKE_TYPED_ARRAY_INFO(String, Variant::Type::STRING)
MAKE_TYPED_ARRAY_INFO(Vector2, Variant::Type::VECTOR2)
MAKE_TYPED_ARRAY_INFO(Vector2i, Variant::Type::VECTOR2I)
MAKE_TYPED_ARRAY_INFO(Rect2, Variant::Type::RECT2)
MAKE_TYPED_ARRAY_INFO(Rect2i, Variant::Type::RECT2I)
MAKE_TYPED_ARRAY_INFO(Vector3, Variant::Type::VECTOR3)
MAKE_TYPED_ARRAY_INFO(Vector3i, Variant::Type::VECTOR3I)
MAKE_TYPED_ARRAY_INFO(Transform2D, Variant::Type::TRANSFORM2D)
MAKE_TYPED_ARRAY_INFO(Plane, Variant::Type::PLANE)
MAKE_TYPED_ARRAY_INFO(Quat, Variant::Type::QUAT)
MAKE_TYPED_ARRAY_INFO(AABB, Variant::Type::AABB)
MAKE_TYPED_ARRAY_INFO(Basis, Variant::Type::BASIS)
MAKE_TYPED_ARRAY_INFO(Transform, Variant::Type::TRANSFORM)
MAKE_TYPED_ARRAY_INFO(Color, Variant::Type::COLOR)
MAKE_TYPED_ARRAY_INFO(StringName, Variant::Type::STRING_NAME)
MAKE_TYPED_ARRAY_INFO(NodePath, Variant::Type::NODE_PATH)
MAKE_TYPED_ARRAY_INFO(RID, Variant::Type::_RID)
MAKE_TYPED_ARRAY_INFO(Callable, Variant::Type::CALLABLE)
MAKE_TYPED_ARRAY_INFO(Signal, Variant::Type::SIGNAL)
MAKE_TYPED_ARRAY_INFO(Dictionary, Variant::Type::DICTIONARY)
MAKE_TYPED_ARRAY_INFO(Array, Variant::Type::ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<uint8_t>, Variant::Type::PACKED_BYTE_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<int32_t>, Variant::Type::PACKED_INT32_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<int64_t>, Variant::Type::PACKED_INT64_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<float>, Variant::Type::PACKED_FLOAT32_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<double>, Variant::Type::PACKED_FLOAT64_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<String>, Variant::Type::PACKED_STRING_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<Vector2>, Variant::Type::PACKED_VECTOR2_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<Vector3>, Variant::Type::PACKED_VECTOR3_ARRAY)
MAKE_TYPED_ARRAY_INFO(Vector<Color>, Variant::Type::PACKED_COLOR_ARRAY)

#endif

#endif // TYPED_ARRAY_H
