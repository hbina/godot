/*************************************************************************/
/*  variant.cpp                                                          */
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

#include "variant.h"

#include "core/core_string_names.h"
#include "core/io/marshalls.h"
#include "core/math/math_funcs.h"
#include "core/print_string.h"
#include "core/resource.h"
#include "core/variant_parser.h"
#include "scene/gui/control.h"
#include "scene/main/node.h"

static std::atomic<int> variant_counter;

static std::unordered_map<int, String> variant_string_map;
static std::unordered_map<int, Vector3> variant_vector3_map;
static std::unordered_map<int, Vector2> variant_vector2_map;
static std::unordered_map<int, Rect2> variant_rect2_map;
static std::unordered_map<int, Plane> variant_plane_map;
static std::unordered_map<int, bool> variant_bool_map;
static std::unordered_map<int, uint64_t> variant_uint64_map;
static std::unordered_map<int, real_t> variant_real_map;
static std::unordered_map<int, double> variant_double_map;
static std::unordered_map<int, float> variant_float_map;
static std::unordered_map<int, Transform2D> variant_transform2d_map;
static std::unordered_map<int, Color> variant_color_map;
static std::unordered_map<int, NodePath> variant_node_path_map;
static std::unordered_map<int, RID> variant_rid_map;
static std::unordered_map<int, Dictionary> variant_dictionary_map;
static std::unordered_map<int, ::AABB> variant_aabb_map;
static std::unordered_map<int, Quat> variant_quat_map;
static std::unordered_map<int, Basis> variant_basis_map;
static std::unordered_map<int, Transform> variant_transform_map;
static std::unordered_map<int, Array> variant_array_map;
static std::unordered_map<int, Vector<Plane> > variant_vector_plane_map;
static std::unordered_map<int, Vector<RID> > variant_vector_rid_map;
static std::unordered_map<int, Vector<Vector2> > variant_vector_vector2_map;
static std::unordered_map<int, Vector<Variant> > variant_vector_variant_map;
static std::unordered_map<int, Vector<uint8_t> > variant_vector_uint8_map;
static std::unordered_map<int, Vector<int> > variant_vector_int_map;
static std::unordered_map<int, Vector<Color> > variant_vector_color_map;
static std::unordered_map<int, Vector<real_t> > variant_vector_real_map;
static std::unordered_map<int, Vector<double> > variant_vector_double_map;
static std::unordered_map<int, Vector<float> > variant_vector_float_map;
static std::unordered_map<int, Vector<String> > variant_vector_string_map;
static std::unordered_map<int, Vector<StringName> > variant_vector_string_name_map;
static std::unordered_map<int, Vector<Vector3> > variant_vector_vector3_map;
static std::unordered_map<int, PoolVector<Plane> > variant_pool_vector_plane_map;
static std::unordered_map<int, PoolVector<uint8_t> > variant_pool_vector_uint8_map;
static std::unordered_map<int, PoolVector<int> > variant_pool_vector_int_map;
static std::unordered_map<int, PoolVector<uint64_t> > variant_pool_vector_uint64_map;
static std::unordered_map<int, PoolVector<real_t> > variant_pool_vector_real_map;
static std::unordered_map<int, PoolVector<String> > variant_pool_vector_string_map;
static std::unordered_map<int, PoolVector<Vector2> > variant_pool_vector_vector2_map;
static std::unordered_map<int, PoolVector<Vector3> > variant_pool_vector_vector3_map;
static std::unordered_map<int, PoolVector<Color> > variant_pool_vector_color_map;
static std::unordered_map<int, PoolVector<Face3> > variant_pool_vector_face3_map;
static std::unordered_map<int, ObjData> variant_objdata_map;

Variant::Type Variant::get_type() const {
	return type;
}

String Variant::get_type_name(const Variant::Type p_type) {

	switch (p_type) {
		case NIL: {

			return "Nil";
		} break;

		// atomic types
		case BOOL: {

			return "bool";
		} break;
		case INT: {

			return "int";

		} break;
		case REAL: {

			return "float";

		} break;
		case STRING: {

			return "String";
		} break;

		// math types
		case VECTOR2: {

			return "Vector2";
		} break;
		case RECT2: {

			return "Rect2";
		} break;
		case TRANSFORM2D: {

			return "Transform2D";
		} break;
		case VECTOR3: {

			return "Vector3";
		} break;
		case PLANE: {

			return "Plane";
		} break;
		case AABB: {

			return "AABB";
		} break;
		case QUAT: {

			return "Quat";
		} break;
		case BASIS: {

			return "Basis";
		} break;
		case TRANSFORM: {

			return "Transform";
		} break;

		// misc types
		case COLOR: {

			return "Color";
		} break;
		case _RID: {

			return "RID";
		} break;
		case OBJECT: {

			return "Object";
		} break;
		case NODE_PATH: {

			return "NodePath";
		} break;
		case DICTIONARY: {

			return "Dictionary";
		} break;
		case ARRAY: {

			return "Array";
		} break;

		// arrays
		case POOL_BYTE_ARRAY: {

			return "PoolByteArray";
		} break;
		case POOL_INT_ARRAY: {

			return "PoolIntArray";
		} break;
		case POOL_REAL_ARRAY: {

			return "PoolRealArray";
		} break;
		case POOL_STRING_ARRAY: {

			return "PoolStringArray";
		} break;
		case POOL_VECTOR2_ARRAY: {

			return "PoolVector2Array";
		} break;
		case POOL_VECTOR3_ARRAY: {

			return "PoolVector3Array";
		} break;
		case POOL_COLOR_ARRAY: {

			return "PoolColorArray";
		} break;
		default: {
			CRASH_NOW();
		}
	}

	return "";
}

bool Variant::can_convert(const Variant::Type p_type_from, const Variant::Type p_type_to) {

	if (p_type_from == p_type_to)
		return true;
	if (p_type_to == NIL && p_type_from != NIL) //nil can convert to anything
		return true;

	if (p_type_from == NIL) {
		return (p_type_to == OBJECT);
	};

	const Type *valid_types = nullptr;
	const Type *invalid_types = nullptr;

	switch (p_type_to) {
		case BOOL: {

			static const Type valid[] = {
				INT,
				REAL,
				STRING,
				NIL,
			};

			valid_types = valid;
		} break;
		case INT: {

			static const Type valid[] = {
				BOOL,
				REAL,
				STRING,
				NIL,
			};

			valid_types = valid;

		} break;
		case REAL: {

			static const Type valid[] = {
				BOOL,
				INT,
				STRING,
				NIL,
			};

			valid_types = valid;

		} break;
		case STRING: {

			static const Type invalid[] = {
				OBJECT,
				NIL
			};

			invalid_types = invalid;
		} break;
		case TRANSFORM2D: {

			static const Type valid[] = {
				TRANSFORM,
				NIL
			};

			valid_types = valid;
		} break;
		case QUAT: {

			static const Type valid[] = {
				BASIS,
				NIL
			};

			valid_types = valid;

		} break;
		case BASIS: {

			static const Type valid[] = {
				QUAT,
				VECTOR3,
				NIL
			};

			valid_types = valid;

		} break;
		case TRANSFORM: {

			static const Type valid[] = {
				TRANSFORM2D,
				QUAT,
				BASIS,
				NIL
			};

			valid_types = valid;

		} break;

		case COLOR: {

			static const Type valid[] = {
				STRING,
				INT,
				NIL,
			};

			valid_types = valid;

		} break;

		case _RID: {

			static const Type valid[] = {
				OBJECT,
				NIL
			};

			valid_types = valid;
		} break;
		case OBJECT: {

			static const Type valid[] = {
				NIL
			};

			valid_types = valid;
		} break;
		case NODE_PATH: {

			static const Type valid[] = {
				STRING,
				NIL
			};

			valid_types = valid;
		} break;
		case ARRAY: {

			static const Type valid[] = {
				POOL_BYTE_ARRAY,
				POOL_INT_ARRAY,
				POOL_STRING_ARRAY,
				POOL_REAL_ARRAY,
				POOL_COLOR_ARRAY,
				POOL_VECTOR2_ARRAY,
				POOL_VECTOR3_ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		// arrays
		case POOL_BYTE_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		case POOL_INT_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;
		} break;
		case POOL_REAL_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		case POOL_STRING_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;
		} break;
		case POOL_VECTOR2_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;

		} break;
		case POOL_VECTOR3_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;

		} break;
		case POOL_COLOR_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;

		} break;
		default: {
		}
	}

	if (valid_types) {

		int i = 0;
		while (valid_types[i] != NIL) {

			if (p_type_from == valid_types[i])
				return true;
			i++;
		}

	} else if (invalid_types) {

		int i = 0;
		while (invalid_types[i] != NIL) {

			if (p_type_from == invalid_types[i])
				return false;
			i++;
		}

		return true;
	}

	return false;
}

bool Variant::can_convert_strict(const Variant::Type p_type_from, const Variant::Type p_type_to) {

	if (p_type_from == p_type_to)
		return true;
	if (p_type_to == NIL && p_type_from != NIL) //nil can convert to anything
		return true;

	if (p_type_from == NIL) {
		return (p_type_to == OBJECT);
	};

	const Type *valid_types = nullptr;

	switch (p_type_to) {
		case BOOL: {

			static const Type valid[] = {
				INT,
				REAL,
				NIL,
			};

			valid_types = valid;
		} break;
		case INT: {

			static const Type valid[] = {
				BOOL,
				REAL,
				NIL,
			};

			valid_types = valid;

		} break;
		case REAL: {

			static const Type valid[] = {
				BOOL,
				INT,
				NIL,
			};

			valid_types = valid;

		} break;
		case STRING: {

			static const Type valid[] = {
				NODE_PATH,
				NIL
			};

			valid_types = valid;
		} break;
		case TRANSFORM2D: {

			static const Type valid[] = {
				TRANSFORM,
				NIL
			};

			valid_types = valid;
		} break;
		case QUAT: {

			static const Type valid[] = {
				BASIS,
				NIL
			};

			valid_types = valid;

		} break;
		case BASIS: {

			static const Type valid[] = {
				QUAT,
				VECTOR3,
				NIL
			};

			valid_types = valid;

		} break;
		case TRANSFORM: {

			static const Type valid[] = {
				TRANSFORM2D,
				QUAT,
				BASIS,
				NIL
			};

			valid_types = valid;

		} break;

		case COLOR: {

			static const Type valid[] = {
				STRING,
				INT,
				NIL,
			};

			valid_types = valid;

		} break;

		case _RID: {

			static const Type valid[] = {
				OBJECT,
				NIL
			};

			valid_types = valid;
		} break;
		case OBJECT: {

			static const Type valid[] = {
				NIL
			};

			valid_types = valid;
		} break;
		case NODE_PATH: {

			static const Type valid[] = {
				STRING,
				NIL
			};

			valid_types = valid;
		} break;
		case ARRAY: {

			static const Type valid[] = {
				POOL_BYTE_ARRAY,
				POOL_INT_ARRAY,
				POOL_STRING_ARRAY,
				POOL_REAL_ARRAY,
				POOL_COLOR_ARRAY,
				POOL_VECTOR2_ARRAY,
				POOL_VECTOR3_ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		// arrays
		case POOL_BYTE_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		case POOL_INT_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;
		} break;
		case POOL_REAL_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;
		} break;
		case POOL_STRING_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;
		} break;
		case POOL_VECTOR2_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;

		} break;
		case POOL_VECTOR3_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};
			valid_types = valid;

		} break;
		case POOL_COLOR_ARRAY: {

			static const Type valid[] = {
				ARRAY,
				NIL
			};

			valid_types = valid;

		} break;
		default: {
		}
	}

	if (valid_types) {

		int i = 0;
		while (valid_types[i] != NIL) {

			if (p_type_from == valid_types[i])
				return true;
			i++;
		}
	}

	return false;
}

bool Variant::operator==(const Variant &p_variant) const {

	if (type != p_variant.type) //evaluation of operator== needs to be more strict
		return false;
	bool v;
	Variant r;
	evaluate(OP_EQUAL, *this, p_variant, r, v);
	return r;
}

bool Variant::operator!=(const Variant &p_variant) const {

	if (type != p_variant.type) //evaluation of operator== needs to be more strict
		return true;
	bool v;
	Variant r;
	evaluate(OP_NOT_EQUAL, *this, p_variant, r, v);
	return r;
}

bool Variant::operator<(const Variant &p_variant) const {
	if (type != p_variant.type) //if types differ, then order by type first
		return type < p_variant.type;
	bool v;
	Variant r;
	evaluate(OP_LESS, *this, p_variant, r, v);
	return r;
}

bool Variant::is_zero() const {

	// TODO : Add check if the entry does not exist
	// TODO : Add check if it does exist, but the entry is empty
	switch (type) {
		case NIL: {

			return true;
		} break;

		// atomic types
		case BOOL: {

			return variant_bool_map.find(variant_id) == variant_bool_map.end();
		} break;
		case INT: {

			return variant_uint64_map.find(variant_id) == variant_uint64_map.end();
		} break;
		case REAL: {

			return variant_real_map.find(variant_id) == variant_real_map.end();
		} break;
		case STRING: {

			return variant_string_map.find(variant_id) == variant_string_map.end();
		} break;

		// math types
		case VECTOR2: {

			return variant_vector2_map.find(variant_id) == variant_vector2_map.end();
		} break;
		case RECT2: {

			return variant_rect2_map.find(variant_id) == variant_rect2_map.end();
		} break;
		case TRANSFORM2D: {

			return variant_transform2d_map.find(variant_id) == variant_transform2d_map.end();
		} break;
		case VECTOR3: {

			return variant_vector3_map.find(variant_id) == variant_vector3_map.end();
		} break;
		case PLANE: {

			return variant_plane_map.find(variant_id) == variant_plane_map.end();
		} break;
		case AABB: {

			return variant_aabb_map.find(variant_id) == variant_aabb_map.end();
		} break;
		case QUAT: {

			return variant_quat_map.find(variant_id) == variant_quat_map.end();
		} break;
		case BASIS: {

			return variant_basis_map.find(variant_id) == variant_basis_map.end();
		} break;
		case TRANSFORM: {

			return variant_transform_map.find(variant_id) == variant_transform_map.end();
		} break;

		// misc types
		case COLOR: {

			return variant_color_map.find(variant_id) == variant_color_map.end();
		} break;
		case _RID: {

			return variant_rid_map.find(variant_id) == variant_rid_map.end();
		} break;
		case OBJECT: {

			// TODO : Figure out what this actually does
			// return variant_object_map.find(variant_id) == variant_object_map.end();
		} break;
		case NODE_PATH: {

			return variant_node_path_map.find(variant_id) == variant_node_path_map.end();
		} break;
		case DICTIONARY: {

			return variant_dictionary_map.find(variant_id) == variant_dictionary_map.end();
		} break;
		case ARRAY: {

			return variant_transform_map.find(variant_id) == variant_transform_map.end();
		} break;

		// TODO : Figure out if we want to store these as pointers or as actual stuff...
		// Check if array exist
		case POOL_BYTE_ARRAY: {

			return variant_pool_vector_uint8_map.find(variant_id) == variant_pool_vector_uint8_map.end();
		} break;
		case POOL_INT_ARRAY: {

			return variant_pool_vector_uint64_map.find(variant_id) == variant_pool_vector_uint64_map.end();
		} break;
		case POOL_REAL_ARRAY: {

			return variant_pool_vector_real_map.find(variant_id) == variant_pool_vector_real_map.end();
		} break;
		case POOL_STRING_ARRAY: {

			return variant_pool_vector_string_map.find(variant_id) == variant_pool_vector_string_map.end();
		} break;
		case POOL_VECTOR2_ARRAY: {

			return variant_pool_vector_vector2_map.find(variant_id) == variant_pool_vector_vector2_map.end();
		} break;
		case POOL_VECTOR3_ARRAY: {

			return variant_pool_vector_vector3_map.find(variant_id) == variant_pool_vector_vector3_map.end();
		} break;
		case POOL_COLOR_ARRAY: {

			return variant_pool_vector_color_map.find(variant_id) == variant_pool_vector_color_map.end();
		} break;
		default: {
		}
	}

	return false;
}

// TODO : What if I only want to check if such map exist...without adding an entry.
bool Variant::is_one() const {

	switch (type) {
		case NIL: {

			return true;
		} break;

		// atomic types
		case BOOL: {

			auto return_var = variant_bool_map.find(variant_id);
			return return_var != variant_bool_map.end() ? return_var->second : false;
		} break;
		case INT: {

			auto return_var = variant_uint64_map.find(variant_id);
			return return_var != variant_uint64_map.end() ? return_var->second == 1 : false;
		} break;
		case REAL: {

			auto return_var = variant_real_map.find(variant_id);
			return return_var != variant_real_map.end() ? return_var->second == 1.0f : false;
		} break;
		case VECTOR2: {

			auto return_var = variant_vector2_map.find(variant_id);
			return return_var != variant_vector2_map.end() ? return_var->second == Vector2(1, 1) : false;
		} break;
		case RECT2: {

			auto return_var = variant_rect2_map.find(variant_id);
			return return_var != variant_rect2_map.end() ? return_var->second == Rect2(1, 1, 1, 1) : false;
		} break;
		case VECTOR3: {

			auto return_var = variant_vector3_map.find(variant_id);
			return return_var != variant_vector3_map.end() ? return_var->second == Vector3(1, 1, 1) : false;
		} break;
		case PLANE: {

			auto return_var = variant_plane_map.find(variant_id);
			return return_var != variant_plane_map.end() ? return_var->second == Plane(1, 1, 1, 1) : false;
		} break;
		case COLOR: {

			auto return_var = variant_color_map.find(variant_id);
			return return_var != variant_color_map.end() ? return_var->second == Color(1, 1, 1, 1) : false;
		} break;

		default: {
			return !is_zero();
		}
	}

	return false;
}

void Variant::reference(const Variant &p_variant) {

	clear();

	type = p_variant.type;

	switch (p_variant.type) {
		case NIL: {

			// do nothing
		} break;
		case BOOL: {

			variant_bool_map[variant_id] = variant_bool_map[p_variant.variant_id];
		} break;
		case INT: {

			variant_uint64_map[variant_id] = variant_uint64_map[p_variant.variant_id];
		} break;
		case REAL: {

			variant_real_map[variant_id] = variant_real_map[p_variant.variant_id];
		} break;
		case STRING: {

			variant_string_map[variant_id] = variant_string_map[p_variant.variant_id];
		} break;

		// math types
		case VECTOR2: {

			variant_vector2_map[variant_id] = variant_vector2_map[p_variant.variant_id];
		} break;
		case RECT2: {

			variant_rect2_map[variant_id] = variant_rect2_map[p_variant.variant_id];
		} break;
		case TRANSFORM2D: {

			variant_transform2d_map[variant_id] = variant_transform2d_map[p_variant.variant_id];
		} break;
		case VECTOR3: {

			variant_vector3_map[variant_id] = variant_vector3_map[p_variant.variant_id];
		} break;
		case PLANE: {

			variant_plane_map[variant_id] = variant_plane_map[p_variant.variant_id];
		} break;

		case AABB: {

			variant_aabb_map[variant_id] = variant_aabb_map[p_variant.variant_id];
		} break;
		case QUAT: {

			variant_quat_map[variant_id] = variant_quat_map[p_variant.variant_id];
		} break;
		case BASIS: {

			variant_basis_map[variant_id] = variant_basis_map[p_variant.variant_id];
		} break;
		case TRANSFORM: {

			variant_transform_map[variant_id] = variant_transform_map[p_variant.variant_id];
		} break;

		// misc types
		case COLOR: {

			variant_color_map[variant_id] = variant_color_map[p_variant.variant_id];
		} break;
		case _RID: {

			variant_rid_map[variant_id] = variant_rid_map[p_variant.variant_id];
		} break;
		case OBJECT: {

			std::cerr << "Attempting to reference Object with id:" << variant_id << std::endl;
			// variant_object_map[variant_id] = variant_object_map[p_variant.variant_id];
		} break;
		case NODE_PATH: {

			variant_node_path_map[variant_id] = variant_node_path_map[p_variant.variant_id];
		} break;
		case DICTIONARY: {

			variant_dictionary_map[variant_id] = variant_dictionary_map[p_variant.variant_id];
		} break;
		case ARRAY: {

			variant_array_map[variant_id] = variant_array_map[p_variant.variant_id];
		} break;

		// arrays
		case POOL_BYTE_ARRAY: {

			variant_pool_vector_uint8_map[variant_id] = variant_pool_vector_uint8_map[p_variant.variant_id];
		} break;
		case POOL_INT_ARRAY: {

			variant_pool_vector_uint64_map[variant_id] = variant_pool_vector_uint64_map[p_variant.variant_id];
		} break;
		case POOL_REAL_ARRAY: {

			variant_pool_vector_real_map[variant_id] = variant_pool_vector_real_map[p_variant.variant_id];
		} break;
		case POOL_STRING_ARRAY: {

			variant_pool_vector_string_map[variant_id] = variant_pool_vector_string_map[p_variant.variant_id];
		} break;
		case POOL_VECTOR2_ARRAY: {

			variant_pool_vector_vector2_map[variant_id] = variant_pool_vector_vector2_map[p_variant.variant_id];
		} break;
		case POOL_VECTOR3_ARRAY: {

			variant_pool_vector_vector3_map[variant_id] = variant_pool_vector_vector3_map[p_variant.variant_id];
		} break;
		case POOL_COLOR_ARRAY: {

			variant_pool_vector_color_map[variant_id] = variant_pool_vector_color_map[p_variant.variant_id];
		} break;
		default: {
			CRASH_NOW();
		}
	}
}

void Variant::zero() {
	switch (type) {
		case NIL: break;
		case BOOL: {

			variant_bool_map[variant_id] = false;
		} break;
		case INT: {

			variant_uint64_map[variant_id] = 0u;
		} break;
		case REAL: {

			variant_real_map[variant_id] = 0.0;
		} break;
		case VECTOR2: {

			variant_vector2_map[variant_id] = Vector2();
		} break;
		case RECT2: {

			variant_rect2_map[variant_id] = Rect2();
		} break;
		case VECTOR3: {

			variant_vector3_map[variant_id] = Vector3();
		} break;
		case PLANE: {

			variant_plane_map[variant_id] = Plane();
		} break;
		case QUAT: {

			variant_quat_map[variant_id] = Quat();
		} break;
		case COLOR: {

			variant_color_map[variant_id] = Color();
		} break;
		default: clear();
	}
}

// TODO : Call this before changing types
void Variant::clear() {

	switch (type) {
		case STRING: {

			variant_string_map.erase(variant_id);
		} break;
		case TRANSFORM2D: {

			variant_transform2d_map.erase(variant_id);
		} break;
		case AABB: {

			variant_aabb_map.erase(variant_id);
		} break;
		case BASIS: {

			variant_basis_map.erase(variant_id);
		} break;
		case TRANSFORM: {

			variant_transform_map.erase(variant_id);
		} break;

		// misc types
		case NODE_PATH: {

			variant_node_path_map.erase(variant_id);
		} break;
		case OBJECT: {

			std::cerr << "Attempting to clear Object with id:" << variant_id << std::endl;
			// variant_object_map.erase(variant_id);
		} break;
		case _RID: {

			variant_rid_map.erase(variant_id);
		} break;
		case DICTIONARY: {

			variant_dictionary_map.erase(variant_id);
		} break;
		case ARRAY: {

			variant_array_map.erase(variant_id);
		} break;
		// arrays
		case POOL_BYTE_ARRAY: {

			variant_pool_vector_uint8_map.erase(variant_id);
		} break;
		case POOL_INT_ARRAY: {

			variant_pool_vector_uint64_map.erase(variant_id);
		} break;
		case POOL_REAL_ARRAY: {

			variant_real_map.erase(variant_id);
		} break;
		case POOL_STRING_ARRAY: {

			variant_pool_vector_string_map.erase(variant_id);
		} break;
		case POOL_VECTOR2_ARRAY: {

			variant_pool_vector_vector2_map.erase(variant_id);
		} break;
		case POOL_VECTOR3_ARRAY: {

			variant_pool_vector_vector3_map.erase(variant_id);
		} break;
		case POOL_COLOR_ARRAY: {

			variant_pool_vector_color_map.erase(variant_id);
		} break;
		default: {
			std::cerr << "Nothing to clear" << std::endl;
		}
	}
	type = NIL;
}

Variant::operator signed int() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int();
		}
		default: {
			return 0;
		}
	}
}

Variant::operator unsigned int() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int();
		}
		default: {
			return 0;
		}
	}
}

Variant::operator int64_t() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int64();
		}
		default: {
			return 0;
		}
	}
}

Variant::operator uint64_t() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int();
		}
		default: {
			return 0;
		}
	}
}

Variant::operator signed short() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int();
		}
		default: {
			return 0;
		}
	}
}
Variant::operator unsigned short() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int();
		}
		default: {
			return 0;
		}
	}
}
Variant::operator signed char() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int();
		}
		default: {
			return 0;
		}
	}
}
Variant::operator unsigned char() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_int();
		}
		default: {
			return 0;
		}
	}
}

Variant::operator CharType() const {

	return operator unsigned int();
}

Variant::operator float() const {

	switch (type) {
		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_float();
		}
		default: {
			return 0;
		}
	}
}

Variant::operator double() const {

	switch (type) {

		case NIL: {
			return 0;
		}
		case BOOL: {
			return variant_bool_map[variant_id];
		}
		case INT: {
			return variant_uint64_map[variant_id];
		}
		case REAL: {
			return variant_real_map[variant_id];
		}
		case STRING: {
			return variant_string_map[variant_id].to_double();
		}
		default: {
			return 0;
		}
	}
}

Variant::operator StringName() const {

	if (type == NODE_PATH) {
		return variant_node_path_map[variant_id].get_sname();
	}
	return StringName(operator String());
}

struct VariantStringPair {

	String key;
	String value;

	bool operator<(const VariantStringPair &p) const {

		return key < p.key;
	}
};

Variant::operator String() const {
	List<const void *> stack;

	return stringify(stack);
}

String Variant::stringify(List<const void *> &stack) const {
	switch (type) {

		case NIL: {
			return "VariantType::NIL";
		}
		case BOOL: {
			return variant_bool_map[variant_id] ? "VariantType::BOOL::true" : "VariantType::BOOL::false";
		}
		case INT: {
			return itos(variant_uint64_map[variant_id]);
		}
		case REAL: {
			return rtos(variant_real_map[variant_id]);
		}
		case STRING: {
			return variant_string_map[variant_id];
		}
		case VECTOR2: {
			return "(" + operator Vector2() + ")";
		}
		case RECT2: {
			return "(" + operator Rect2() + ")";
		}
		case TRANSFORM2D: {
			Transform2D mat32 = operator Transform2D();
			return "(" + Variant(mat32.elements[0]).operator String() + ", " + Variant(mat32.elements[1]).operator String() + ", " + Variant(mat32.elements[2]).operator String() + ")";
		} break;
		case VECTOR3: {
			return "(" + operator Vector3() + ")";
		}
		case PLANE: {
			return operator Plane();
		}
		case AABB: {
			return operator ::AABB();
		}
		case QUAT: {
			return "(" + operator Quat() + ")";
		}
		case BASIS: {
			Basis mat3 = operator Basis();
			String mtx("(");
			for (int i = 0; i < 3; i++) {
				if (i != 0) {
					mtx += ", ";
				}
				mtx += "(";
				for (int j = 0; j < 3; j++) {
					if (j != 0) {
						mtx += ", ";
					}
					mtx += Variant(mat3.elements[i][j]).operator String();
				}
				mtx += ")";
			}
			return mtx + ")";
		} break;
		case TRANSFORM: {
			return operator Transform();
		}
		case NODE_PATH: {
			return operator NodePath();
		}
		case COLOR: {
			return String::num(operator Color().r) + "," + String::num(operator Color().g) + "," + String::num(operator Color().b) + "," + String::num(operator Color().a);
		}
		case DICTIONARY: {
			const Dictionary &d = variant_dictionary_map[variant_id];
			if (stack.find(d.id())) {
				return "{...}";
			}
			stack.push_back(d.id());
			String str("{");
			List<Variant> keys;
			d.get_key_list(&keys);
			Vector<VariantStringPair> pairs;
			for (const auto &E : keys) {
				VariantStringPair sp;
				sp.key = E.stringify(stack);
				sp.value = d[E].stringify(stack);
				pairs.push_back(sp);
			}
			pairs.sort();
			for (int i = 0; i < pairs.size(); i++) {
				if (i > 0) {
					str += ", ";
				}
				str += pairs[i].key + ":" + pairs[i].value;
			}
			str += "}";
			return str;
		} break;
		case POOL_VECTOR2_ARRAY: {
			PoolVector<Vector2> vec = operator PoolVector<Vector2>();
			String str("[");
			for (int i = 0; i < vec.size(); i++) {
				if (i > 0) {
					str += ", ";
				}
				str = str + Variant(vec[i]);
			}
			str += "]";
			return str;
		} break;
		case POOL_VECTOR3_ARRAY: {
			PoolVector<Vector3> vec = operator PoolVector<Vector3>();
			String str("[");
			for (int i = 0; i < vec.size(); i++) {
				if (i > 0) {
					str += ", ";
				}
				str = str + Variant(vec[i]);
			}
			str += "]";
			return str;
		} break;
		case POOL_STRING_ARRAY: {
			PoolVector<String> vec = operator PoolVector<String>();
			String str("[");
			for (int i = 0; i < vec.size(); i++) {
				if (i > 0) {
					str += ", ";
				}
				str = str + vec[i];
			}
			str += "]";
			return str;
		} break;
		case POOL_INT_ARRAY: {
			PoolVector<int> vec = operator PoolVector<int>();
			String str("[");
			for (int i = 0; i < vec.size(); i++) {
				if (i > 0) {
					str += ", ";
				}
				str = str + itos(vec[i]);
			}
			str += "]";
			return str;
		} break;
		case POOL_REAL_ARRAY: {
			PoolVector<real_t> vec = operator PoolVector<real_t>();
			String str("[");
			for (int i = 0; i < vec.size(); i++) {
				if (i > 0) {
					str += ", ";
				}
				str = str + rtos(vec[i]);
			}
			str += "]";
			return str;
		} break;
		case ARRAY: {
			Array arr = operator Array();
			if (stack.find(arr.id())) {
				return "[...]";
			}
			stack.push_back(arr.id());
			String str("[");
			for (int i = 0; i < arr.size(); i++) {
				if (i) {
					str += ", ";
				}
				str += arr[i].stringify(stack);
			}
			str += "]";
			return str;

		} break;
		default: {
			return "[" + get_type_name(type) + "]";
		}
	}

	return "";
}

Variant::operator Vector2() const {
	if (type == VECTOR2) {
		return variant_vector2_map[variant_id];
	} else if (type == VECTOR3) {
		const Vector3 &vec = variant_vector3_map[variant_id];
		return Vector2(vec.x, vec.y);
	} else {
		return Vector2();
	}
}

Variant::operator Rect2() const {

	if (type == RECT2) {
		return variant_rect2_map[variant_id];
	} else {
		return Rect2();
	}
}

Variant::operator Vector3() const {

	if (type == VECTOR3) {
		return variant_vector3_map[variant_id];
	} else if (type == VECTOR2) {
		const Vector2 &vec = variant_vector2_map[variant_id];
		return Vector3(vec.x, vec.y, 0.0);
	} else {
		return Vector3();
	}
}

Variant::operator Plane() const {

	if (type == PLANE) {
		return variant_plane_map[variant_id];
	} else {
		return Plane();
	}
}

Variant::operator ::AABB() const {

	if (type == AABB) {
		return variant_aabb_map[variant_id];
	} else {
		return ::AABB();
	}
}

Variant::operator Basis() const {

	if (type == BASIS) {
		return variant_basis_map[variant_id];
	} else if (type == QUAT) {
		return variant_quat_map[variant_id];
	} else if (type == VECTOR3) {
		return variant_vector3_map[variant_id];
	} else if (type == TRANSFORM) {
		return variant_transform_map[variant_id].basis;
	} else {
		return Basis();
	}
}

Variant::operator Quat() const {

	if (type == QUAT) {
		return variant_quat_map[variant_id];
	} else if (type == BASIS) {
		return variant_basis_map[variant_id];
	} else if (type == TRANSFORM) {
		return variant_transform_map[variant_id].basis;
	} else {
		return Quat();
	}
}

Variant::operator Transform() const {

	if (type == TRANSFORM) {
		return variant_transform_map[variant_id];
	} else if (type == BASIS) {
		return Transform(variant_basis_map[variant_id], Vector3());
	} else if (type == QUAT) {
		return Transform(Basis(variant_quat_map[variant_id], Vector3()));
	} else if (type == TRANSFORM2D) {
		const Transform2D &t = variant_transform2d_map[variant_id];
		Transform m;
		m.basis.elements[0][0] = t.elements[0][0];
		m.basis.elements[1][0] = t.elements[0][1];
		m.basis.elements[0][1] = t.elements[1][0];
		m.basis.elements[1][1] = t.elements[1][1];
		m.origin[0] = t.elements[2][0];
		m.origin[1] = t.elements[2][1];
		return m;
	} else
		return Transform();
}

Variant::operator Transform2D() const {

	if (type == TRANSFORM2D) {
		return variant_transform2d_map[variant_id];
	} else if (type == TRANSFORM) {
		const Transform &t = variant_transform_map[variant_id];
		Transform2D m;
		m.elements[0][0] = t.basis.elements[0][0];
		m.elements[0][1] = t.basis.elements[1][0];
		m.elements[1][0] = t.basis.elements[0][1];
		m.elements[1][1] = t.basis.elements[1][1];
		m.elements[2][0] = t.origin[0];
		m.elements[2][1] = t.origin[1];
		return m;
	} else {
		return Transform2D();
	}
}

Variant::operator Color() const {

	if (type == COLOR) {
		return variant_color_map[variant_id];
	} else if (type == STRING) {
		return Color::html(operator String());
	} else if (type == INT) {
		return Color::hex(operator int());
	} else {
		return Color();
	}
}

Variant::operator NodePath() const {

	if (type == NODE_PATH) {
		return variant_node_path_map[variant_id];
	} else if (type == STRING) {
		return NodePath(operator String());
	} else {
		return NodePath();
	}
}

Variant::operator RID() const {

	if (type == _RID) {
		return variant_rid_map[variant_id];
	} else if (type == OBJECT && _get_obj().obj) {
#ifdef DEBUG_ENABLED
		if (ScriptDebugger::get_singleton()) {
			if (!ObjectDB::instance_validate(_get_obj().obj)) {
				ERR_EXPLAIN("Invalid pointer (object was deleted)");
				ERR_FAIL_V(RID());
			};
		};
#endif
		Variant::CallError ce;
		Variant ret = _get_obj().obj->call(CoreStringNames::get_singleton()->get_rid, nullptr, 0, ce);
		if (ce.error == Variant::CallError::CALL_OK && ret.get_type() == Variant::_RID) {
			return ret;
		}
		return RID();
	} else {
		return RID();
	}
}

Variant::operator Object *() const {

	if (type == OBJECT)
		return _get_obj().obj;
	else
		return nullptr;
}
Variant::operator Node *() const {

	if (type == OBJECT)
		return Object::cast_to<Node>(_get_obj().obj);
	else
		return nullptr;
}
Variant::operator Control *() const {

	if (type == OBJECT)
		return Object::cast_to<Control>(_get_obj().obj);
	else
		return nullptr;
}

Variant::operator Dictionary() const {

	if (type == DICTIONARY) {
		return variant_dictionary_map[variant_id];
	} else {
		return Dictionary();
	}
}

template <class DA, class SA>
DA _convert_array(const SA &p_array) {

	DA da;
	da.resize(p_array.size());

	for (int i = 0; i < p_array.size(); i++) {

		da.set(i, Variant(p_array.get(i)));
	}

	return da;
}

template <class DA>
DA _convert_array_from_variant(const Variant &p_variant) {

	switch (p_variant.get_type()) {

		case Variant::ARRAY: {
			return _convert_array<DA, Array>(p_variant.operator Array());
		}
		case Variant::POOL_BYTE_ARRAY: {
			return _convert_array<DA, PoolVector<uint8_t> >(p_variant.operator PoolVector<uint8_t>());
		}
		case Variant::POOL_INT_ARRAY: {
			return _convert_array<DA, PoolVector<int> >(p_variant.operator PoolVector<int>());
		}
		case Variant::POOL_REAL_ARRAY: {
			return _convert_array<DA, PoolVector<real_t> >(p_variant.operator PoolVector<real_t>());
		}
		case Variant::POOL_STRING_ARRAY: {
			return _convert_array<DA, PoolVector<String> >(p_variant.operator PoolVector<String>());
		}
		case Variant::POOL_VECTOR2_ARRAY: {
			return _convert_array<DA, PoolVector<Vector2> >(p_variant.operator PoolVector<Vector2>());
		}
		case Variant::POOL_VECTOR3_ARRAY: {
			return _convert_array<DA, PoolVector<Vector3> >(p_variant.operator PoolVector<Vector3>());
		}
		case Variant::POOL_COLOR_ARRAY: {
			return _convert_array<DA, PoolVector<Color> >(p_variant.operator PoolVector<Color>());
		}
		default: {
			return DA();
		}
	}

	return DA();
}

Variant::operator Array() const {

	if (type == ARRAY) {
		variant_array_map[variant_id];
	} else {
		return _convert_array_from_variant<Array>(*this);
	}
}

Variant::operator PoolVector<uint8_t>() const {

	if (type == POOL_BYTE_ARRAY) {
		return variant_pool_vector_uint8_map[variant_id];
	} else
		return _convert_array_from_variant<PoolVector<uint8_t> >(*this);
}
Variant::operator PoolVector<int>() const {

	if (type == POOL_INT_ARRAY) {
		return variant_pool_vector_int_map[variant_id];
	} else
		return _convert_array_from_variant<PoolVector<int> >(*this);
}
Variant::operator PoolVector<real_t>() const {

	if (type == POOL_REAL_ARRAY) {
		return variant_pool_vector_real_map[variant_id];
	} else
		return _convert_array_from_variant<PoolVector<real_t> >(*this);
}

Variant::operator PoolVector<String>() const {

	if (type == POOL_STRING_ARRAY) {
		return variant_pool_vector_string_map[variant_id];
	} else
		return _convert_array_from_variant<PoolVector<String> >(*this);
}
Variant::operator PoolVector<Vector3>() const {

	if (type == POOL_VECTOR3_ARRAY) {
		return variant_pool_vector_vector3_map[variant_id];
	} else
		return _convert_array_from_variant<PoolVector<Vector3> >(*this);
}
Variant::operator PoolVector<Vector2>() const {

	if (type == POOL_VECTOR2_ARRAY) {
		return variant_pool_vector_vector2_map[variant_id];
	} else {
		return _convert_array_from_variant<PoolVector<Vector2> >(*this);
	}
}

Variant::operator PoolVector<Color>() const {

	if (type == POOL_COLOR_ARRAY) {
		return variant_pool_vector_color_map[variant_id];
	} else
		return _convert_array_from_variant<PoolVector<Color> >(*this);
}

/* helpers */

Variant::operator Vector<RID>() const {

	Array va = operator Array();
	Vector<RID> rids;
	rids.resize(va.size());
	for (int i = 0; i < rids.size(); i++)
		rids[i] = va[i];
	return rids;
}

Variant::operator Vector<Vector2>() const {

	PoolVector<Vector2> from = operator PoolVector<Vector2>();
	Vector<Vector2> to;
	int len = from.size();
	if (len == 0)
		return Vector<Vector2>();
	to.resize(len);
	PoolVector<Vector2>::Read r = from.read();
	Vector2 *w = to.ptrw();
	for (int i = 0; i < len; i++) {

		w[i] = r[i];
	}
	return to;
}

Variant::operator PoolVector<Plane>() const {

	Array va = operator Array();
	PoolVector<Plane> planes;
	int va_size = va.size();
	if (va_size == 0)
		return planes;

	planes.resize(va_size);
	PoolVector<Plane>::Write w = planes.write();

	for (int i = 0; i < va_size; i++)
		w[i] = va[i];

	return planes;
}

Variant::operator PoolVector<Face3>() const {

	PoolVector<Vector3> va = operator PoolVector<Vector3>();
	PoolVector<Face3> faces;
	int va_size = va.size();
	if (va_size == 0)
		return faces;

	faces.resize(va_size / 3);
	PoolVector<Face3>::Write w = faces.write();
	PoolVector<Vector3>::Read r = va.read();

	for (int i = 0; i < va_size; i++)
		w[i / 3].vertex[i % 3] = r[i];

	return faces;
}

Variant::operator Vector<Plane>() const {

	Array va = operator Array();
	Vector<Plane> planes;
	int va_size = va.size();
	if (va_size == 0)
		return planes;

	planes.resize(va_size);

	for (int i = 0; i < va_size; i++)
		planes[i] = va[i];

	return planes;
}

Variant::operator Vector<Variant>() const {

	Array from = operator Array();
	Vector<Variant> to;
	int len = from.size();
	to.resize(len);
	for (int i = 0; i < len; i++) {

		to[i] = from[i];
	}
	return to;
}

Variant::operator Vector<uint8_t>() const {

	PoolVector<uint8_t> from = operator PoolVector<uint8_t>();
	Vector<uint8_t> to;
	int len = from.size();
	to.resize(len);
	for (int i = 0; i < len; i++) {

		to[i] = from[i];
	}
	return to;
}
Variant::operator Vector<int>() const {

	PoolVector<int> from = operator PoolVector<int>();
	Vector<int> to;
	int len = from.size();
	to.resize(len);
	for (int i = 0; i < len; i++) {

		to[i] = from[i];
	}
	return to;
}
Variant::operator Vector<real_t>() const {

	PoolVector<real_t> from = operator PoolVector<real_t>();
	Vector<real_t> to;
	int len = from.size();
	to.resize(len);
	for (int i = 0; i < len; i++) {

		to[i] = from[i];
	}
	return to;
}

Variant::operator Vector<String>() const {

	PoolVector<String> from = operator PoolVector<String>();
	Vector<String> to;
	int len = from.size();
	to.resize(len);
	for (int i = 0; i < len; i++) {

		to[i] = from[i];
	}
	return to;
}
Variant::operator Vector<StringName>() const {

	PoolVector<String> from = operator PoolVector<String>();
	Vector<StringName> to;
	int len = from.size();
	to.resize(len);
	for (int i = 0; i < len; i++) {

		to[i] = from[i];
	}
	return to;
}

Variant::operator Vector<Vector3>() const {

	PoolVector<Vector3> from = operator PoolVector<Vector3>();
	Vector<Vector3> to;
	int len = from.size();
	if (len == 0)
		return Vector<Vector3>();
	to.resize(len);
	PoolVector<Vector3>::Read r = from.read();
	Vector3 *w = to.ptrw();
	for (int i = 0; i < len; i++) {

		w[i] = r[i];
	}
	return to;
}
Variant::operator Vector<Color>() const {

	PoolVector<Color> from = operator PoolVector<Color>();
	Vector<Color> to;
	int len = from.size();
	if (len == 0)
		return Vector<Color>();
	to.resize(len);
	PoolVector<Color>::Read r = from.read();
	Color *w = to.ptrw();
	for (int i = 0; i < len; i++) {

		w[i] = r[i];
	}
	return to;
}

Variant::operator Margin() const {

	return (Margin) operator int();
}
Variant::operator Orientation() const {

	return (Orientation) operator int();
}

Variant::operator IP_Address() const {

	if (type == POOL_REAL_ARRAY || type == POOL_INT_ARRAY || type == POOL_BYTE_ARRAY) {

		PoolVector<int> addr = operator PoolVector<int>();
		if (addr.size() == 4) {
			return IP_Address(addr.get(0), addr.get(1), addr.get(2), addr.get(3));
		}
	}

	return IP_Address(operator String());
}

Variant::Variant(bool p_bool) :
		variant_id(variant_counter++) {
	type = BOOL;
}

Variant::Variant() :
		variant_id(variant_counter++) {
	type = NIL;
}

Variant::Variant(const uint64_t p_int) :
		variant_id(variant_counter++) {
	type = INT;
	variant_uint64_map.insert({ variant_id, p_int });
}

Variant::Variant(const real_t p_double) :
		variant_id(variant_counter++) {
	type = REAL;
	variant_real_map.insert({ variant_id, p_double });
}

Variant::Variant(const StringName &p_string) :
		variant_id(variant_counter++) {
	type = STRING;
	variant_string_map.insert({ variant_id, p_string.operator String() });
}

Variant::Variant(const String &p_string) :
		variant_id(variant_counter++) {
	type = STRING;
	variant_string_map.insert({ variant_id, p_string });
}

Variant::Variant(const char *const p_cstring) :
		variant_id(variant_counter++) {
	type = STRING;
	variant_string_map.insert({ variant_id, String(p_cstring) });
}

Variant::Variant(const CharType *p_wstring) :
		variant_id(variant_counter++) {
	type = STRING;
	variant_string_map.insert({ variant_id, String(p_wstring) });
}

Variant::Variant(const Vector3 &p_vector3) :
		variant_id(variant_counter++) {
	type = VECTOR3;
	variant_vector3_map.insert({ variant_id, p_vector3 });
}

Variant::Variant(const Vector2 &p_vector2) :
		variant_id(variant_counter++) {
	type = VECTOR2;
	variant_vector2_map.insert({ variant_id, p_vector2 });
}

Variant::Variant(const Rect2 &p_rect2) :
		variant_id(variant_counter++) {
	type = RECT2;
	variant_rect2_map.insert({ variant_id, p_rect2 });
}

Variant::Variant(const Plane &p_plane) :
		variant_id(variant_counter++) {
	type = PLANE;
	variant_plane_map.insert({ variant_id, p_plane });
}

Variant::Variant(const ::AABB &p_aabb) :
		variant_id(variant_counter++) {
	type = AABB;
	variant_aabb_map.insert({ variant_id, p_aabb });
}

Variant::Variant(const Basis &p_matrix) :
		variant_id(variant_counter++) {
	type = BASIS;
	variant_basis_map.insert({ variant_id, p_matrix });
}

Variant::Variant(const Quat &p_quat) :
		variant_id(variant_counter++) {
	type = QUAT;
	variant_quat_map.insert({ variant_id, p_quat });
}

Variant::Variant(const Transform &p_transform) :
		variant_id(variant_counter++) {
	type = TRANSFORM;
	variant_transform_map.insert({ variant_id, p_transform });
}

Variant::Variant(const Transform2D &p_transform2d) :
		variant_id(variant_counter++) {
	type = TRANSFORM2D;
	variant_transform2d_map.insert({ variant_id, p_transform2d });
}

Variant::Variant(const Color &p_color) :
		variant_id(variant_counter++) {
	type = COLOR;
	variant_color_map.insert({ variant_id, p_color });
}

Variant::Variant(const NodePath &p_node_path) :
		variant_id(variant_counter++) {
	type = NODE_PATH;
	variant_node_path_map.insert({ variant_id, p_node_path });
}

Variant::Variant(const RefPtr &p_resource) :
		variant_id(variant_counter++) {
	std::cout << "constructing RefPtr id:" << variant_id << std::endl;
}

Variant::Variant(const RID &p_rid) :
		variant_id(variant_counter++) {
	type = _RID;
	variant_rid_map.insert({ variant_id, p_rid });
}

Variant::Variant(const Object *p_object) :
		variant_id(variant_counter++) {
	std::cout << "constructing Object* id:" << variant_id << std::endl;
}

Variant::Variant(const Dictionary &p_dictionary) :
		variant_id(variant_counter++) {
	type = DICTIONARY;
	variant_dictionary_map.insert({ variant_id, p_dictionary });
}

Variant::Variant(const Array &p_array) :
		variant_id(variant_counter++) {
	type = ARRAY;
	variant_array_map.insert({ variant_id, p_array });
}

Variant::Variant(const PoolVector<Plane> &p_array) :
		variant_id(variant_counter++) {
	type = ARRAY;
	variant_pool_vector_plane_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<Plane> &p_array) :
		variant_id(variant_counter++) {
	type = ARRAY;
	variant_vector_plane_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<RID> &p_array) :
		variant_id(variant_counter++) {
	type = ARRAY;
	variant_vector_rid_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<Vector2> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_vector2_map.insert({ variant_id, p_array });
}

Variant::Variant(const PoolVector<uint8_t> &p_raw_array) :
		variant_id(variant_counter++) {
	type = POOL_BYTE_ARRAY;
	variant_pool_vector_uint8_map.insert({ variant_id, p_raw_array });
}

Variant::Variant(const PoolVector<int> &p_int_array) :
		variant_id(variant_counter++) {
	type = POOL_INT_ARRAY;
	variant_pool_vector_int_map.insert({ variant_id, p_int_array });
}

Variant::Variant(const PoolVector<real_t> &p_real_array) :
		variant_id(variant_counter++) {
	type = POOL_REAL_ARRAY;
	variant_pool_vector_real_map.insert({ variant_id, p_real_array });
}

Variant::Variant(const PoolVector<String> &p_string_array) :
		variant_id(variant_counter++) {
	type = POOL_STRING_ARRAY;
	variant_pool_vector_string_map.insert({ variant_id, p_string_array });
}

Variant::Variant(const PoolVector<Vector3> &p_vector3_array) :
		variant_id(variant_counter++) {
	type = POOL_VECTOR3_ARRAY;
	variant_pool_vector_vector3_map.insert({ variant_id, p_vector3_array });
}

Variant::Variant(const PoolVector<Vector2> &p_vector2_array) :
		variant_id(variant_counter++) {
	type = POOL_VECTOR2_ARRAY;
	variant_pool_vector_vector2_map.insert({ variant_id, p_vector2_array });
}

Variant::Variant(const PoolVector<Color> &p_color_array) :
		variant_id(variant_counter++) {
	type = POOL_COLOR_ARRAY;
	variant_pool_vector_color_map.insert({ variant_id, p_color_array });
}

Variant::Variant(const PoolVector<Face3> &p_face_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_pool_vector_face3_map.insert({ variant_id, p_face_array });
}

/* helpers */

Variant::Variant(const Vector<Variant> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_variant_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<uint8_t> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_uint8_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<int> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_int_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<real_t> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_real_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<String> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_string_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<StringName> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_string_name_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<Vector3> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_vector3_map.insert({ variant_id, p_array });
}

Variant::Variant(const Vector<Color> &p_array) :
		variant_id(variant_counter++) {
	type = NIL;
	variant_vector_color_map.insert({ variant_id, p_array });
}

Variant::Variant(const IP_Address &p_address) :
		variant_id(variant_counter++) {
	type = STRING;
	// TODO : What now?
}

Variant::Variant(const Variant &p_variant) :
		variant_id(variant_counter++) {
	type = NIL;
}

Variant::~Variant() {
	if (type != Variant::NIL) { // TODO : Since previous implementation does not delete Vectors...must change this
		clear();
	}
}

Variant &Variant::operator=(const Variant &p_variant) {

	// TODO : Do we really not change the type of this?
	if (this == &p_variant)
		return;

	if (type != p_variant.type) {
		reference(p_variant);
		return;
	}

	switch (p_variant.type) {
		case NIL: {

			// do nothing
		} break;
		case BOOL: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case INT: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case REAL: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case STRING: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;

		case VECTOR2: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case RECT2: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case TRANSFORM2D: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case VECTOR3: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case PLANE: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;

		case AABB: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case QUAT: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case BASIS: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case TRANSFORM: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;

		// misc types
		case COLOR: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case _RID: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case OBJECT: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case NODE_PATH: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case DICTIONARY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;

		// arrays
		case POOL_BYTE_ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case POOL_INT_ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case POOL_REAL_ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case POOL_STRING_ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case POOL_VECTOR2_ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case POOL_VECTOR3_ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		case POOL_COLOR_ARRAY: {

			variant_bool_map.insert({ variant_id, variant_bool_map[p_variant.variant_id] });
		} break;
		default: {
			CRASH_NOW();
		}
	}

	return *this;
}

uint32_t Variant::hash() const {

	switch (type) {
		case NIL: {

			return 0u;
		} break;
		case BOOL: {

			return variant_bool_map[variant_id];
		} break;
		case INT: {

			return variant_bool_map[variant_id];
		} break;
		case REAL: {

			return hash_djb2_one_float(variant_bool_map[variant_id]);
		} break;
		case STRING: {

			return variant_string_map[variant_id].hash();
		} break;

		// math types
		case VECTOR2: {

			uint32_t hash = hash_djb2_one_float(variant_vector2_map[variant_id].x);
			return hash_djb2_one_float(variant_vector2_map[variant_id].y, hash);
		} break;
		case RECT2: {

			uint32_t hash = hash_djb2_one_float(variant_rect2_map[variant_id].position.x);
			hash = hash_djb2_one_float(variant_rect2_map[variant_id].position.y, hash);
			hash = hash_djb2_one_float(variant_rect2_map[variant_id].size.x, hash);
			return hash_djb2_one_float(variant_rect2_map[variant_id].size.y, hash);
		} break;
		case TRANSFORM2D: {

			uint32_t hash = 5831; // TODO : What is this arbitrary value?
			for (int i = 0; i < 3; i++) { // TODO : Refactor and make special class that performs all this conversion...
				for (int j = 0; j < 2; j++) {
					hash = hash_djb2_one_float(variant_transform2d_map[variant_id].elements[i][j], hash);
				}
			}
			return hash;
		} break;
		case VECTOR3: {

			uint32_t hash = hash_djb2_one_float(variant_vector3_map[variant_id].x);
			hash = hash_djb2_one_float(variant_vector3_map[variant_id].y, hash);
			return hash_djb2_one_float(variant_vector3_map[variant_id].z, hash);
		} break;
		case PLANE: {

			uint32_t hash = hash_djb2_one_float(variant_plane_map[variant_id].normal.x);
			hash = hash_djb2_one_float(variant_plane_map[variant_id].normal.y, hash);
			hash = hash_djb2_one_float(variant_plane_map[variant_id].normal.z, hash);
			return hash_djb2_one_float(variant_plane_map[variant_id].d, hash);
		} break;
		case AABB: {

			uint32_t hash = 5831;
			for (int i = 0; i < 3; i++) {
				hash = hash_djb2_one_float(variant_aabb_map[variant_id].position[i], hash);
				hash = hash_djb2_one_float(variant_aabb_map[variant_id].size[i], hash);
			}
			return hash;
		} break;
		case QUAT: {

			uint32_t hash = hash_djb2_one_float(variant_quat_map[variant_id].x);
			hash = hash_djb2_one_float(variant_quat_map[variant_id].y, hash);
			hash = hash_djb2_one_float(variant_quat_map[variant_id].z, hash);
			return hash_djb2_one_float(variant_quat_map[variant_id].w, hash);
		} break;
		case BASIS: {

			uint32_t hash = 5831;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					hash = hash_djb2_one_float(variant_basis_map[variant_id].elements[i][j], hash);
				}
			}
			return hash;
		} break;
		case TRANSFORM: {

			uint32_t hash = 5831;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					hash = hash_djb2_one_float(variant_transform_map[variant_id].basis.elements[i][j], hash);
				}
				hash = hash_djb2_one_float(variant_transform_map[variant_id].origin[i], hash);
			}
			return hash;
		} break;

		// misc types
		case COLOR: {

			uint32_t hash = hash_djb2_one_float(variant_color_map[variant_id].r);
			hash = hash_djb2_one_float(variant_color_map[variant_id].g, hash);
			hash = hash_djb2_one_float(variant_color_map[variant_id].b, hash);
			return hash_djb2_one_float(variant_color_map[variant_id].a, hash);
		} break;
		case _RID: {

			return hash_djb2_one_64(variant_rid_map[variant_id].get_id());
		} break;
		case OBJECT: {

			return hash_djb2_one_64(make_uint64_t(_get_obj().obj));
		} break;
		case NODE_PATH: {

			return variant_node_path_map[variant_id].hash();
		} break;
		case DICTIONARY: {

			return variant_dictionary_map[variant_id].hash();

		} break;
		case ARRAY: {

			return variant_array_map[variant_id].hash();
		} break;
		case POOL_BYTE_ARRAY: {

			const PoolVector<uint8_t> &arr = variant_pool_vector_uint8_map[variant_id];
			int len = arr.size();
			if (likely(len)) {
				PoolVector<uint8_t>::Read r = arr.read();
				return hash_djb2_buffer((uint8_t *)&r[0], len);
			} else {
				return hash_djb2_one_64(0);
			}

		} break;
		case POOL_INT_ARRAY: {

			const PoolVector<int> &arr = variant_pool_vector_int_map[variant_id];
			int len = arr.size();
			if (likely(len)) {
				PoolVector<int>::Read r = arr.read();
				return hash_djb2_buffer((uint8_t *)&r[0], len * sizeof(int));
			} else {
				return hash_djb2_one_64(0);
			}

		} break;
		case POOL_REAL_ARRAY: {

			const PoolVector<real_t> &arr = variant_pool_vector_real_map[variant_id];
			int len = arr.size();

			if (likely(len)) {
				PoolVector<real_t>::Read r = arr.read();
				return hash_djb2_buffer((uint8_t *)&r[0], len * sizeof(real_t));
			} else {
				return hash_djb2_one_float(0.0);
			}

		} break;
		case POOL_STRING_ARRAY: {

			uint32_t hash = 5831;
			const PoolVector<String> &arr = variant_pool_vector_string_map[variant_id];
			int len = arr.size();

			if (likely(len)) {
				PoolVector<String>::Read r = arr.read();

				for (int i = 0; i < len; i++) {
					hash = hash_djb2_one_32(r[i].hash(), hash);
				}
			}

			return hash;
		} break;
		case POOL_VECTOR2_ARRAY: {

			uint32_t hash = 5831;
			const PoolVector<Vector2> &arr = variant_pool_vector_vector2_map[variant_id];
			int len = arr.size();

			if (len) {
				PoolVector<Vector2>::Read r = arr.read();

				for (int i = 0; i < len; i++) {
					hash = hash_djb2_one_float(r[i].x, hash);
					hash = hash_djb2_one_float(r[i].y, hash);
				}
			}

			return hash;
		} break;
		case POOL_VECTOR3_ARRAY: {

			uint32_t hash = 5831;
			const PoolVector<Vector3> &arr = variant_pool_vector_vector3_map[variant_id];
			int len = arr.size();

			if (len) {
				PoolVector<Vector3>::Read r = arr.read();

				for (int i = 0; i < len; i++) {
					hash = hash_djb2_one_float(r[i].x, hash);
					hash = hash_djb2_one_float(r[i].y, hash);
					hash = hash_djb2_one_float(r[i].z, hash);
				}
			}
			return hash;
		} break;
		case POOL_COLOR_ARRAY: {

			uint32_t hash = 5831;
			const PoolVector<Color> &arr = variant_pool_vector_color_map[variant_id];
			int len = arr.size();
			if (len) {
				PoolVector<Color>::Read r = arr.read();

				for (int i = 0; i < len; i++) {
					hash = hash_djb2_one_float(r[i].r, hash);
					hash = hash_djb2_one_float(r[i].g, hash);
					hash = hash_djb2_one_float(r[i].b, hash);
					hash = hash_djb2_one_float(r[i].a, hash);
				}
			}

			return hash;
		} break;
		default: {
		}
	}

	return 0;
}

#define hash_compare_scalar(p_lhs, p_rhs) \
	((p_lhs) == (p_rhs)) || (Math::is_nan(p_lhs) && Math::is_nan(p_rhs))

#define hash_compare_vector2(p_lhs, p_rhs)         \
	(hash_compare_scalar((p_lhs).x, (p_rhs).x)) && \
			(hash_compare_scalar((p_lhs).y, (p_rhs).y))

#define hash_compare_vector3(p_lhs, p_rhs)                 \
	(hash_compare_scalar((p_lhs).x, (p_rhs).x)) &&         \
			(hash_compare_scalar((p_lhs).y, (p_rhs).y)) && \
			(hash_compare_scalar((p_lhs).z, (p_rhs).z))

#define hash_compare_quat(p_lhs, p_rhs)                    \
	(hash_compare_scalar((p_lhs).x, (p_rhs).x)) &&         \
			(hash_compare_scalar((p_lhs).y, (p_rhs).y)) && \
			(hash_compare_scalar((p_lhs).z, (p_rhs).z)) && \
			(hash_compare_scalar((p_lhs).w, (p_rhs).w))

#define hash_compare_color(p_lhs, p_rhs)                   \
	(hash_compare_scalar((p_lhs).r, (p_rhs).r)) &&         \
			(hash_compare_scalar((p_lhs).g, (p_rhs).g)) && \
			(hash_compare_scalar((p_lhs).b, (p_rhs).b)) && \
			(hash_compare_scalar((p_lhs).a, (p_rhs).a))

#define hash_compare_pool_array(l, r, p_type, p_compare_func) \
                                                              \
	if (l.size() != r.size())                                 \
		return false;                                         \
                                                              \
	PoolVector<p_type>::Read lr = l.read();                   \
	PoolVector<p_type>::Read rr = r.read();                   \
                                                              \
	for (int i = 0; i < l.size(); ++i) {                      \
		if (!p_compare_func((lr[i]), (rr[i])))                \
			return false;                                     \
	}                                                         \
                                                              \
	return true

bool Variant::hash_compare(const Variant &p_variant) const {
	if (type != p_variant.type)
		return false;

	switch (type) {
		case REAL: {

			return hash_compare_scalar(variant_real_map[variant_id], variant_real_map[p_variant.variant_id]);
		} break;
		case VECTOR2: {

			const Vector2 &lhs = variant_vector2_map[variant_id];
			const Vector2 &rhs = variant_vector2_map[p_variant.variant_id];
			return hash_compare_vector2(lhs, rhs);
		} break;
		case RECT2: {

			const Rect2 &lhs = variant_rect2_map[variant_id];
			const Rect2 &rhs = variant_rect2_map[p_variant.variant_id];
			return (hash_compare_vector2(lhs.position, rhs.position)) &&
				   (hash_compare_vector2(lhs.size, rhs.size));
		} break;
		case TRANSFORM2D: {

			Transform2D &lhs = variant_transform2d_map[variant_id];
			Transform2D &rhs = variant_transform2d_map[p_variant.variant_id];
			for (int i = 0; i < 3; i++) {
				if (!(hash_compare_vector2(lhs.elements[i], rhs.elements[i]))) {
					return false;
				}
			}
			return true;
		} break;
		case VECTOR3: {

			const Vector3 &lhs = variant_vector3_map[variant_id];
			const Vector3 &rhs = variant_vector3_map[p_variant.variant_id];
			return hash_compare_vector3(lhs, rhs);
		} break;
		case PLANE: {

			const Plane &lhs = variant_plane_map[variant_id];
			const Plane &rhs = variant_plane_map[p_variant.variant_id];
			return (hash_compare_vector3(lhs.normal, rhs.normal)) &&
				   (hash_compare_scalar(lhs.d, rhs.d));
		} break;
		case AABB: {

			const ::AABB &lhs = variant_aabb_map[variant_id];
			const ::AABB &rhs = variant_aabb_map[p_variant.variant_id];
			return (hash_compare_vector3(lhs.position, rhs.position) &&
					(hash_compare_vector3(lhs.size, rhs.size)));
		} break;
		case QUAT: {

			const Quat &lhs = variant_quat_map[variant_id];
			const Quat &rhs = variant_quat_map[p_variant.variant_id];
			return hash_compare_quat(lhs, rhs);
		} break;
		case BASIS: {

			const Basis &lhs = variant_basis_map[variant_id];
			const Basis &rhs = variant_basis_map[p_variant.variant_id];
			for (int i = 0; i < 3; i++) {
				if (!(hash_compare_vector3(lhs.elements[i], rhs.elements[i]))) {
					return false;
				}
			}
			return true;
		} break;
		case TRANSFORM: {

			const Transform &lhs = variant_transform_map[variant_id];
			const Transform &rhs = variant_transform_map[p_variant.variant_id];
			for (int i = 0; i < 3; i++) {
				if (!(hash_compare_vector3(lhs.basis.elements[i], rhs.basis.elements[i])))
					return false;
			}
			return hash_compare_vector3(lhs.origin, rhs.origin);
		} break;

		case COLOR: {

			const Color &lhs = variant_color_map[variant_id];
			const Color &rhs = variant_color_map[p_variant.variant_id];
			return hash_compare_color(lhs, rhs);
		} break;
		case ARRAY: {

			const Array &lhs = variant_array_map[variant_id];
			const Array &rhs = variant_array_map[p_variant.variant_id];
			if (lhs.size() != rhs.size()) {
				return false;
			}
			for (int i = 0; i < lhs.size(); ++i) {
				if (!lhs[i].hash_compare(rhs[i])) {
					return false;
				}
			}
			return true;
		} break;
		case POOL_REAL_ARRAY: {

			hash_compare_pool_array(
					variant_pool_vector_real_map[variant_id],
					variant_pool_vector_real_map[p_variant.variant_id],
					real_t,
					hash_compare_scalar);
		} break;
		case POOL_VECTOR2_ARRAY: {

			hash_compare_pool_array(
					variant_pool_vector_vector2_map[variant_id],
					variant_pool_vector_vector2_map[p_variant.variant_id],
					Vector2,
					hash_compare_vector2);
		} break;
		case POOL_VECTOR3_ARRAY: {

			hash_compare_pool_array(
					variant_pool_vector_vector3_map[variant_id],
					variant_pool_vector_vector3_map[p_variant.variant_id],
					Vector3,
					hash_compare_vector3);
		} break;

		case POOL_COLOR_ARRAY: {

			hash_compare_pool_array(
					variant_pool_vector_color_map[variant_id],
					variant_pool_vector_color_map[p_variant.variant_id],
					Color,
					hash_compare_color);
		} break;
		default:

			bool v;
			Variant r;
			evaluate(OP_EQUAL, *this, p_variant, r, v);
			return r;
	}
	return false;
}

bool Variant::is_ref() const {

	return type == OBJECT && !_get_obj().ref.is_null();
}

Vector<Variant> varray() {

	return Vector<Variant>();
}

Vector<Variant> varray(const Variant &p_arg1) {

	Vector<Variant> v;
	v.push_back(p_arg1);
	return v;
}
Vector<Variant> varray(const Variant &p_arg1, const Variant &p_arg2) {

	Vector<Variant> v;
	v.push_back(p_arg1);
	v.push_back(p_arg2);
	return v;
}
Vector<Variant> varray(const Variant &p_arg1, const Variant &p_arg2, const Variant &p_arg3) {

	Vector<Variant> v;
	v.push_back(p_arg1);
	v.push_back(p_arg2);
	v.push_back(p_arg3);
	return v;
}
Vector<Variant> varray(const Variant &p_arg1, const Variant &p_arg2, const Variant &p_arg3, const Variant &p_arg4) {

	Vector<Variant> v;
	v.push_back(p_arg1);
	v.push_back(p_arg2);
	v.push_back(p_arg3);
	v.push_back(p_arg4);
	return v;
}

Vector<Variant> varray(const Variant &p_arg1, const Variant &p_arg2, const Variant &p_arg3, const Variant &p_arg4, const Variant &p_arg5) {

	Vector<Variant> v;
	v.push_back(p_arg1);
	v.push_back(p_arg2);
	v.push_back(p_arg3);
	v.push_back(p_arg4);
	v.push_back(p_arg5);
	return v;
}

void Variant::static_assign(const Variant &p_variant) {
}

bool Variant::is_shared() const {

	switch (type) {

		case OBJECT: return true;
		case ARRAY: return true;
		case DICTIONARY: return true;
		default: {
		}
	}

	return false;
}

Variant Variant::call(const StringName &p_method, VARIANT_ARG_DECLARE) {
	VARIANT_ARGPTRS;
	int argc = 0;
	for (int i = 0; i < VARIANT_ARG_MAX; i++) {
		if (argptr[i]->get_type() == Variant::NIL)
			break;
		argc++;
	}

	CallError error;

	Variant ret = call(p_method, argptr, argc, error);

	switch (error.error) {

		case CallError::CALL_ERROR_INVALID_ARGUMENT: {

			String err = "Invalid type for argument #" + itos(error.argument) + ", expected '" + Variant::get_type_name(error.expected) + "'.";
			ERR_PRINT(err.utf8().get_data());

		} break;
		case CallError::CALL_ERROR_INVALID_METHOD: {

			String err = "Invalid method '" + p_method + "' for type '" + Variant::get_type_name(type) + "'.";
			ERR_PRINT(err.utf8().get_data());
		} break;
		case CallError::CALL_ERROR_TOO_MANY_ARGUMENTS: {

			String err = "Too many arguments for method '" + p_method + "'";
			ERR_PRINT(err.utf8().get_data());
		} break;
		default: {
		}
	}

	return ret;
}

void Variant::construct_from_string(const String &p_string, Variant &r_value, ObjectConstruct p_obj_construct, void *p_construct_ud) {

	r_value = Variant();
}

String Variant::get_construct_string() const {

	String vars;
	VariantWriter::write_to_string(*this, vars);

	return vars;
}

String Variant::get_call_error_text(Object *p_base, const StringName &p_method, const Variant **p_argptrs, int p_argcount, const Variant::CallError &ce) {

	String err_text;

	if (ce.error == Variant::CallError::CALL_ERROR_INVALID_ARGUMENT) {
		int errorarg = ce.argument;
		if (p_argptrs) {
			err_text = "Cannot convert argument " + itos(errorarg + 1) + " from " + Variant::get_type_name(p_argptrs[errorarg]->get_type()) + " to " + Variant::get_type_name(ce.expected) + ".";
		} else {
			err_text = "Cannot convert argument " + itos(errorarg + 1) + " from [missing argptr, type unknown] to " + Variant::get_type_name(ce.expected) + ".";
		}
	} else if (ce.error == Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS) {
		err_text = "Method expected " + itos(ce.argument) + " arguments, but called with " + itos(p_argcount) + ".";
	} else if (ce.error == Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS) {
		err_text = "Method expected " + itos(ce.argument) + " arguments, but called with " + itos(p_argcount) + ".";
	} else if (ce.error == Variant::CallError::CALL_ERROR_INVALID_METHOD) {
		err_text = "Method not found.";
	} else if (ce.error == Variant::CallError::CALL_ERROR_INSTANCE_IS_NULL) {
		err_text = "Instance is null";
	} else if (ce.error == Variant::CallError::CALL_OK) {
		return "Call OK";
	}

	String class_name = p_base->get_class();
	Ref<Script> script = p_base->get_script();
	if (script.is_valid() && script->get_path().is_resource_file()) {

		class_name += "(" + script->get_path().get_file() + ")";
	}
	return "'" + class_name + "::" + String(p_method) + "': " + err_text;
}

String vformat(const String &p_text, const Variant &p1, const Variant &p2, const Variant &p3, const Variant &p4, const Variant &p5) {

	Array args;
	if (p1.get_type() != Variant::NIL) {

		args.push_back(p1);

		if (p2.get_type() != Variant::NIL) {

			args.push_back(p2);

			if (p3.get_type() != Variant::NIL) {

				args.push_back(p3);

				if (p4.get_type() != Variant::NIL) {

					args.push_back(p4);

					if (p5.get_type() != Variant::NIL) {

						args.push_back(p5);
					}
				}
			}
		}
	}

	bool error = false;
	String fmt = p_text.sprintf(args, &error);

	ERR_FAIL_COND_V(error, String());

	return fmt;
}

Variant::ObjData &Variant::_get_obj() {

	return variant_objdata_map[variant_id];
}

const Variant::ObjData &Variant::_get_obj() const {

	return variant_objdata_map[variant_id];
}