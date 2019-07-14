/*************************************************************************/
/*  type_info.h                                                          */
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

#ifndef GET_TYPE_INFO_H
#define GET_TYPE_INFO_H

#ifdef DEBUG_METHODS_ENABLED

template <bool C, typename T = void>
struct EnableIf {

	typedef T type;
};

template <typename T>
struct EnableIf<false, T> {
};

template <typename, typename>
struct TypesAreSame {

	static bool const value = false;
};

template <typename A>
struct TypesAreSame<A, A> {

	static bool const value = true;
};

template <typename B, typename D>
struct TypeInherits {

	static D *get_d();

	static char (&test(B *))[1];
	static char (&test(...))[2];

	static bool const value = sizeof(test(get_d())) == sizeof(char) &&
							  !TypesAreSame<B volatile const, void volatile const>::value;
};

namespace GodotTypeInfo {
enum Metadata {
	METADATA_NONE,
	METADATA_INT_IS_INT8,
	METADATA_INT_IS_INT16,
	METADATA_INT_IS_INT32,
	METADATA_INT_IS_INT64,
	METADATA_INT_IS_UINT8,
	METADATA_INT_IS_UINT16,
	METADATA_INT_IS_UINT32,
	METADATA_INT_IS_UINT64,
	METADATA_REAL_IS_FLOAT,
	METADATA_REAL_IS_DOUBLE
};
}

template <class T, typename = void>
struct GetTypeInfo {
	static const VariantType VARIANT_TYPE = VariantType::NIL;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		ERR_PRINT("GetTypeInfo fallback. Bug!");
		return PropertyInfo(); // Not "Nil", this is an error
	}
};

#define MAKE_TYPE_INFO(m_type, m_var_type)                                            \
	template <>                                                                       \
	struct GetTypeInfo<m_type> {                                                      \
		static const VariantType VARIANT_TYPE = m_var_type;                           \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};                                                                                \
	template <>                                                                       \
	struct GetTypeInfo<const m_type &> {                                              \
		static const VariantType VARIANT_TYPE = m_var_type;                           \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};

#define MAKE_TYPE_INFO_WITH_META(m_type, m_var_type, m_metadata)    \
	template <>                                                     \
	struct GetTypeInfo<m_type> {                                    \
		static const VariantType VARIANT_TYPE = m_var_type;         \
		static const GodotTypeInfo::Metadata METADATA = m_metadata; \
		static inline PropertyInfo get_class_info() {               \
			return PropertyInfo(VARIANT_TYPE, String());            \
		}                                                           \
	};                                                              \
	template <>                                                     \
	struct GetTypeInfo<const m_type &> {                            \
		static const VariantType VARIANT_TYPE = m_var_type;         \
		static const GodotTypeInfo::Metadata METADATA = m_metadata; \
		static inline PropertyInfo get_class_info() {               \
			return PropertyInfo(VARIANT_TYPE, String());            \
		}                                                           \
	};

MAKE_TYPE_INFO(bool, VariantType::BOOL)
MAKE_TYPE_INFO_WITH_META(uint8_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_UINT8)
MAKE_TYPE_INFO_WITH_META(int8_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_INT8)
MAKE_TYPE_INFO_WITH_META(uint16_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_UINT16)
MAKE_TYPE_INFO_WITH_META(int16_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_INT16)
MAKE_TYPE_INFO_WITH_META(uint32_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_UINT32)
MAKE_TYPE_INFO_WITH_META(int32_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_INT32)
MAKE_TYPE_INFO_WITH_META(uint64_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_UINT64)
MAKE_TYPE_INFO_WITH_META(int64_t, VariantType::INT, GodotTypeInfo::METADATA_INT_IS_INT64)
MAKE_TYPE_INFO(wchar_t, VariantType::INT)
MAKE_TYPE_INFO_WITH_META(float, VariantType::REAL, GodotTypeInfo::METADATA_REAL_IS_FLOAT)
MAKE_TYPE_INFO_WITH_META(double, VariantType::REAL, GodotTypeInfo::METADATA_REAL_IS_DOUBLE)

MAKE_TYPE_INFO(String, VariantType::STRING)
MAKE_TYPE_INFO(Vector2, VariantType::VECTOR2)
MAKE_TYPE_INFO(Rect2, VariantType::RECT2)
MAKE_TYPE_INFO(Vector3, VariantType::VECTOR3)
MAKE_TYPE_INFO(Transform2D, VariantType::TRANSFORM2D)
MAKE_TYPE_INFO(Plane, VariantType::PLANE)
MAKE_TYPE_INFO(Quat, VariantType::QUAT)
MAKE_TYPE_INFO(AABB, VariantType::AABB)
MAKE_TYPE_INFO(Basis, VariantType::BASIS)
MAKE_TYPE_INFO(Transform, VariantType::TRANSFORM)
MAKE_TYPE_INFO(Color, VariantType::COLOR)
MAKE_TYPE_INFO(NodePath, VariantType::NODE_PATH)
MAKE_TYPE_INFO(RID, VariantType::_RID)
MAKE_TYPE_INFO(Dictionary, VariantType::DICTIONARY)
MAKE_TYPE_INFO(Array, VariantType::ARRAY)
MAKE_TYPE_INFO(PoolByteArray, VariantType::POOL_BYTE_ARRAY)
MAKE_TYPE_INFO(PoolIntArray, VariantType::POOL_INT_ARRAY)
MAKE_TYPE_INFO(PoolRealArray, VariantType::POOL_REAL_ARRAY)
MAKE_TYPE_INFO(PoolStringArray, VariantType::POOL_STRING_ARRAY)
MAKE_TYPE_INFO(PoolVector2Array, VariantType::POOL_VECTOR2_ARRAY)
MAKE_TYPE_INFO(PoolVector3Array, VariantType::POOL_VECTOR3_ARRAY)
MAKE_TYPE_INFO(PoolColorArray, VariantType::POOL_COLOR_ARRAY)

MAKE_TYPE_INFO(StringName, VariantType::STRING)
MAKE_TYPE_INFO(IP_Address, VariantType::STRING)

class BSP_Tree;
MAKE_TYPE_INFO(BSP_Tree, VariantType::DICTIONARY)

//for RefPtr
template <>
struct GetTypeInfo<RefPtr> {
	static const VariantType VARIANT_TYPE = VariantType::OBJECT;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(VariantType::OBJECT, String(), PROPERTY_HINT_RESOURCE_TYPE, "Reference");
	}
};
template <>
struct GetTypeInfo<const RefPtr &> {
	static const VariantType VARIANT_TYPE = VariantType::OBJECT;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(VariantType::OBJECT, String(), PROPERTY_HINT_RESOURCE_TYPE, "Reference");
	}
};

//for variant
template <>
struct GetTypeInfo<Variant> {
	static const VariantType VARIANT_TYPE = VariantType::NIL;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(VariantType::NIL, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT);
	}
};

template <>
struct GetTypeInfo<const Variant &> {
	static const VariantType VARIANT_TYPE = VariantType::NIL;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(VariantType::NIL, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT);
	}
};

#define MAKE_TEMPLATE_TYPE_INFO(m_template, m_type, m_var_type)                       \
	template <>                                                                       \
	struct GetTypeInfo<m_template<m_type> > {                                         \
		static const VariantType VARIANT_TYPE = m_var_type;                           \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};                                                                                \
	template <>                                                                       \
	struct GetTypeInfo<const m_template<m_type> &> {                                  \
		static const VariantType VARIANT_TYPE = m_var_type;                           \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};

MAKE_TEMPLATE_TYPE_INFO(Vector, uint8_t, VariantType::POOL_BYTE_ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, int, VariantType::POOL_INT_ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, float, VariantType::POOL_REAL_ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, String, VariantType::POOL_STRING_ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, Vector2, VariantType::POOL_VECTOR2_ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, Vector3, VariantType::POOL_VECTOR3_ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, Color, VariantType::POOL_COLOR_ARRAY)

MAKE_TEMPLATE_TYPE_INFO(Vector, Variant, VariantType::ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, RID, VariantType::ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, Plane, VariantType::ARRAY)
MAKE_TEMPLATE_TYPE_INFO(Vector, StringName, VariantType::POOL_STRING_ARRAY)

MAKE_TEMPLATE_TYPE_INFO(PoolVector, Plane, VariantType::ARRAY)
MAKE_TEMPLATE_TYPE_INFO(PoolVector, Face3, VariantType::POOL_VECTOR3_ARRAY)

template <typename T>
struct GetTypeInfo<T *, typename EnableIf<TypeInherits<Object, T>::value>::type> {
	static const VariantType VARIANT_TYPE = VariantType::OBJECT;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(StringName(T::get_class_static()));
	}
};

template <typename T>
struct GetTypeInfo<const T *, typename EnableIf<TypeInherits<Object, T>::value>::type> {
	static const VariantType VARIANT_TYPE = VariantType::OBJECT;
	static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;
	static inline PropertyInfo get_class_info() {
		return PropertyInfo(StringName(T::get_class_static()));
	}
};

#define TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_impl)                                                                                                                                     \
	template <>                                                                                                                                                                       \
	struct GetTypeInfo<m_impl> {                                                                                                                                                      \
		static const VariantType VARIANT_TYPE = VariantType::INT;                                                                                                                     \
		static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_NONE;                                                                                                 \
		static inline PropertyInfo get_class_info() {                                                                                                                                 \
			return PropertyInfo(VariantType::INT, String(), PROPERTY_HINT_NONE, String(), PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_CLASS_IS_ENUM, String(#m_enum).replace("::", ".")); \
		}                                                                                                                                                                             \
	};

#define MAKE_ENUM_TYPE_INFO(m_enum)                 \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_enum)       \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_enum const) \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, m_enum &)     \
	TEMPL_MAKE_ENUM_TYPE_INFO(m_enum, const m_enum &)

template <typename T>
inline StringName __constant_get_enum_name(T param, const String &p_constant) {
	if (GetTypeInfo<T>::VARIANT_TYPE == VariantType::NIL)
		ERR_PRINTS("Missing VARIANT_ENUM_CAST for constant's enum: " + p_constant);
	return GetTypeInfo<T>::get_class_info().class_name;
}

#define CLASS_INFO(m_type)                                     \
	(GetTypeInfo<m_type *>::VARIANT_TYPE != VariantType::NIL ? \
					GetTypeInfo<m_type *>::get_class_info() :  \
					GetTypeInfo<m_type>::get_class_info())

#else

#define MAKE_ENUM_TYPE_INFO(m_enum)
#define CLASS_INFO(m_type)

#endif // DEBUG_METHODS_ENABLED

#endif // GET_TYPE_INFO_H
