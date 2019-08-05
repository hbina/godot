/*************************************************************************/
/*  visual_server_scene.cpp                                              */
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
#include "visual_server_scene.h"
#include "core/ecs_registry.h"
#include "core/os/os.h"
#include "main/profiler.h"
#include "thirdparty/concurrentqueue/blockingconcurrentqueue.h"
#include "visual_server_global.h"
#include "visual_server_raster.h"
#include <thirdparty/plf/plf_colony.h>

#include <algorithm>
#include <execution>
#include <future>
#include <vector>

#define PARALLEL_RENDER

#ifdef PARALLEL_RENDER
#define STL_PARALLEL std::execution::par
#else
#define STL_PARALLEL std::execution::seq
#endif
int shadow_redraws;
/* CAMERA API */
constexpr int Num_Elements = 64;
template <typename Element>
class FastOctree : public NewOctree {
public:
	struct Node;

	struct ElementArray {
		std::array<Element, Num_Elements> Elements;
		std::array<AABB, Num_Elements> ElementBounds;
		std::array<uint32_t, Num_Elements> ElementsIDs;
		uint8_t count;
	};

	struct Node {
		//only there in leaves
		ElementArray *Storage;

		//plf::colony<Node>::iterator self;

		//bounds of this node (perfect)
		AABB BaseBounds;
		//accurate bounds of the children
		AABB ChildBounds;

		Node *Parent;
		Node *Children[8];

		uint32_t id;

		void SplitNode(FastOctree *Owner) {
			float max_x = BaseBounds.position.x + BaseBounds.size.x;
			float min_x = BaseBounds.position.x - BaseBounds.size.x;

			float max_y = BaseBounds.position.y + BaseBounds.size.y;
			float min_y = BaseBounds.position.y - BaseBounds.size.y;

			float max_z = BaseBounds.position.z + BaseBounds.size.z;
			float min_z = BaseBounds.position.z - BaseBounds.size.z;

			float ox = BaseBounds.size.x / 2.0f;
			float oy = BaseBounds.size.y / 2.0f;
			float oz = BaseBounds.size.z / 2.0f;
			for (int i = 0; i < 8; i++) {
				Children[i] = Owner->CreateNode();
				Children[i]->Parent = this;
				Children[i]->BaseBounds.size = BaseBounds.size / 2.0f;
				Children[i]->BaseBounds.position = BaseBounds.position;
			}

			Children[0]->BaseBounds.position += Vector3(ox, oy, oz);
			Children[1]->BaseBounds.position += Vector3(-ox, oy, oz);
			Children[2]->BaseBounds.position += Vector3(ox, -oy, oz);
			Children[3]->BaseBounds.position += Vector3(-ox, -oy, oz);

			Children[4]->BaseBounds.position += Vector3(ox, oy, -oz);
			Children[5]->BaseBounds.position += Vector3(-ox, oy, -oz);
			Children[6]->BaseBounds.position += Vector3(ox, -oy, -oz);
			Children[7]->BaseBounds.position += Vector3(-ox, -oy, -oz);

			for (int i = 0; i < Storage->count; i++) {
				AABB bounds = Storage->ElementBounds[i];
				Element elem = Storage->Elements[i];
				int best = 0;
				float size = ox;
				float bestscore = 9999999999.f;
				for (int n = 0; n < 8; n++) {
					AABB merged = bounds.merge(Children[n]->BaseBounds);
					float scale = merged.get_longest_axis_size();
					if (scale < bestscore) {
						best = n;
						bestscore = scale;
					}
					//if (Children[n]->BaseBounds.has_point(bounds.position)) {
					//	Children[n]->Storage->Add(elem, bounds);
					//	break;
					//}
				}

				Owner->AddElementToArray_id(Storage->ElementsIDs[i], Children[best], elem, bounds);
			}
			for (int n = 0; n < 8; n++) {

				Children[n]->RecalcElementBounds();
			}
			delete Storage;
			Storage = nullptr;
		};

		void RecalcElementBounds() {
			AABB NewBounds = BaseBounds;
			NewBounds.size = Vector3(0.0f, 0.0f, 0.0f);

			if (Storage) {
				for (int i = 0; i < Storage->count; i++) {
					AABB bounds = Storage->ElementBounds[i];
					NewBounds.merge_with(bounds);
				}
			} else {
				for (int n = 0; n < 8; n++) {

					Children[n]->RecalcElementBounds();
					NewBounds.merge_with(Children[n]->ChildBounds);
				}
			}

			ChildBounds = NewBounds;
		}

		uint32_t AddElement(const Element &element, const AABB &bounds, FastOctree *Owner) {
			//is leaf
			uint32_t id = -1;
			if (Storage) {
				//has space
				if (Storage->count < Num_Elements) {
					id = Owner->AddElementToArray(this, element, bounds);
					//Storage->Add(element, bounds);
					ChildBounds.merge_with(bounds);
					return id;

				}
				//no space, split it
				else {
					SplitNode(Owner);
					//add to child
					const float slack = BaseBounds.size.x / 20.0;
					for (int n = 0; n < 8; n++) {
						if (Children[n]->BaseBounds.has_point(bounds.position)) {
							id = Children[n]->AddElement(element, bounds, Owner);
							ChildBounds.merge_with(Children[n]->ChildBounds);
							return id;
						}
					}
					//fallback
					id = Children[0]->AddElement(element, bounds, Owner);
					ChildBounds.merge_with(Children[0]->ChildBounds);
					return id;
				}
			} else {
				//add to child
				for (int n = 0; n < 8; n++) {
					const float slack = BaseBounds.size.x / 20.0;
					if (Children[n]->BaseBounds.has_point(bounds.position)) {
						id = Children[n]->AddElement(element, bounds, Owner);
						ChildBounds.merge_with(Children[n]->ChildBounds);
						return id;
					}
				}
				//fallback
				id = Children[0]->AddElement(element, bounds, Owner);
				ChildBounds.merge_with(Children[0]->ChildBounds);
				return id;
			}

			return id;
		};

		template <typename F>
		void cull_convex(const Vector<Plane> &p_convex, F &&functor) {

			if (ChildBounds.intersects_convex_shape(&p_convex[0], p_convex.size())) {

				if (Storage) {
					for (int i = 0; i < Storage->count; i++) {
						const AABB &bnd = Storage->ElementBounds[i];
						if (bnd.intersects_convex_shape(&p_convex[0], p_convex.size())) {

							//auto &ic = get_component<InstanceComponent>(Storage->Elements[i]);
							functor(Storage->Elements[i]);
						}
					}

				} else {

					for (auto n = 0; n < 8; n++) {
						Children[n]->cull_convex(p_convex, functor);
					}
				}
			}
		};
		template <typename F>
		void cull_aabb(const AABB &bound, F &&functor) {

			if (ChildBounds.intersects(bound)) {

				if (Storage) {
					for (int i = 0; i < Storage->count; i++) {
						const AABB &bnd = Storage->ElementBounds[i];
						if (bnd.intersects(bound)) {

							//auto &ic = get_component<InstanceComponent>(Storage->Elements[i]);
							functor(Storage->Elements[i]);
						}
					}

				} else {

					for (auto n = 0; n < 8; n++) {
						Children[n]->cull_aabb(bound, functor);
					}
				}
			}
		};
	};

	struct NodeRef {
		Node *ref;
	};

	FastOctree(Vector3 InitialBounds) {
		auto e0 = OctreeRegistry.create();
		OctreeRegistry.destroy(e0);
		RootNode = CreateNode();
		RootNode->BaseBounds.set_size(InitialBounds);
	}

	uint32_t AddElement(const Element &element, const AABB &bounds) {
		return RootNode->AddElement(element, bounds, this);
	}

	Node *CreateNode() {

		auto it = new Node(); //NodePool.insert(Node{});
		it->Storage = new ElementArray();
		it->Parent = nullptr;
		uint32_t nodeID = OctreeRegistry.create();
		it->id = nodeID;
		OctreeRegistry.assign<NodeRef>(nodeID, it);
		for (int i = 0; i < 8; i++) {
			it->Children[i] = nullptr;
		}
		return it;
	}
	template <typename F>
	void cull_convex(const Vector<Plane> &p_convex, F &&functor) {
		RootNode->cull_convex(p_convex, functor);
	}
	template <typename F>
	void cull_aabb(const AABB &bound, F &&functor) {
		RootNode->cull_aabb(bound, functor);
	}
	struct ElementReference {
		Node *owner;
		uint16_t idx;
	};
	uint32_t AddElementToArray(Node *node, const Element &newe, const AABB &newbounds) {
		ElementArray *ElemList = node->Storage;
		ElemList->Elements[ElemList->count] = newe;
		ElemList->ElementBounds[ElemList->count] = newbounds;
		uint32_t id = OctreeRegistry.create();

		ElemList->ElementsIDs[ElemList->count] = id;

		OctreeRegistry.assign<ElementReference>(id, node, ElemList->count);

		ElemList->count++;
		return id;
	}
	void AddElementToArray_id(uint32_t id, Node *node, const Element &newe, const AABB &newbounds) {
		ElementArray *ElemList = node->Storage;
		ElemList->Elements[ElemList->count] = newe;
		ElemList->ElementBounds[ElemList->count] = newbounds;
		//uint32_t id = OctreeRegistry.create();

		ElemList->ElementsIDs[ElemList->count] = id;

		OctreeRegistry.assign_or_replace<ElementReference>(id, node, ElemList->count);

		ElemList->count++;
	}
	void RemoveElementFromArray(Node *node, const Element &_remove) {
		ElementArray *ElemList = node->Storage;
		for (int i = 0; i < count; i++) {
			if (ElemList->Elements[i] == _remove) {
				ElemList->Elements[i] = ElemList->Elements[ElemList->count - 1];
				ElemList->ElementBounds[i] = ElemList->ElementBounds[ElemList->count - 1];
				ElemList->ElementsIDs[i] = ElemList->ElementsIDs[ElemList->count - 1];

				OctreeRegistry.assign_or_replace<ElementReference>(ElemList->ElementsIDs[i], node, i);

				ElemList->count--;
				return;
			}
		}
	}
	void RemoveElementFromNode(Node *node, uint16_t idx) {
		ElementArray *ElemList = node->Storage;
		int i = idx;

		ElemList->Elements[i] = ElemList->Elements[ElemList->count - 1];
		ElemList->ElementBounds[i] = ElemList->ElementBounds[ElemList->count - 1];
		ElemList->ElementsIDs[i] = ElemList->ElementsIDs[ElemList->count - 1];

		OctreeRegistry.assign_or_replace<ElementReference>(ElemList->ElementsIDs[i], node, (uint16_t)i);

		ElemList->count--;
	}

	void RemoveOctreeElement(uint32_t id) {
		if (OctreeRegistry.valid(id)) {

			ElementReference &ref = OctreeRegistry.get<ElementReference>(id);
			RemoveElementFromNode(ref.owner, ref.idx);

			OctreeRegistry.destroy(id);
		}
	}

	void MoveElement(uint32_t id, AABB newAABB) {
		AUTO_PROFILE;
		if (OctreeRegistry.valid(id)) {
			ElementReference &ref = OctreeRegistry.get<ElementReference>(id);
			Element original = ref.owner->Storage->Elements[ref.idx];

			RemoveElementFromNode(ref.owner, ref.idx);
			uint32_t newid = AddElement(original, newAABB);

			ElementReference &newref = OctreeRegistry.get<ElementReference>(newid);

			OctreeRegistry.assign_or_replace<ElementReference>(id, newref);

			newref.owner->Storage->ElementsIDs[newref.idx] = id;

			OctreeRegistry.destroy(newid);
		}
	}

private:
	Node *RootNode;
	//plf::colony<Node> NodePool;
	entt::registry OctreeRegistry;
};

struct InstanceComponent {
	VisualServerScene::Instance *instance;
	RID self_ID;
	//VS::InstanceType base_type;
	//bool is_geo() const { return base_type & VS::InstanceType::INSTANCE_GEOMETRY_MASK; };
};
struct ScenarioLink {
	EntityID scenario_id;
	VisualServerScene::Scenario *owner{ nullptr };
};
struct Dirty {
	//aabb stuff
	bool update_aabb;
	bool update_materials;
	Dirty() {
		update_aabb = false;
		update_materials = false;
	}
};
struct GeometryComponent {
	VisualServerScene::InstanceGeometryData *Data{ nullptr };
	std::vector<uint32_t> AffectingLights;
	uint8_t lighting_dirty : 1;
	uint8_t can_cast_shadows : 1;
	uint8_t material_is_animated : 1;
	uint8_t reflection_dirty : 1;
	uint8_t gi_probes_dirty : 1;

	GeometryComponent() :
			Data(nullptr),
			lighting_dirty(false),
			reflection_dirty(true),
			can_cast_shadows(false),
			material_is_animated(false),
			gi_probes_dirty(true) {}

	void add_light(uint32_t light, bool check_existence = false) {
		if (check_existence) {
		}
		AffectingLights.push_back(light);
	};
	void remove_light(uint32_t light) {
		for (int i = 0; i < AffectingLights.size(); i++) {
			if (AffectingLights[i] == light) {
				AffectingLights[i] = AffectingLights.back();
				AffectingLights.pop_back();
				return;
			}
		}
	};
};

struct LightComponent {
	//VisualServerScene::InstanceLightData *Data;
	RID light_instance;
	bool shadow_dirty;
	VisualServerScene::Instance *baked_light;
	uint64_t last_version;
	LightComponent() {
		//Data = nullptr;
		shadow_dirty = false;
		baked_light = nullptr;
		last_version = 0;
	}

	//LightComponent(VisualServerScene::InstanceLightData *_Data) {
	//	shadow_dirty = true;
	//	baked_light = nullptr;
	//	Data = _Data;
	//	last_version = 0;
	//}
};
struct ReflectionProbeComponent {
	VisualServerScene::Instance *owner;
	RID instance;
	bool reflection_dirty{ true };
	int render_step{ -1 };

	VisualServerScene::InstanceReflectionProbeData *Data{ nullptr };
};
struct GIProbeComponent {
	VisualServerScene::Instance *owner;
	VisualServerScene::InstanceGIProbeData *Data{ nullptr };
};
struct LightmapCaptureComponent {
	VisualServerScene::Instance *owner;
	VisualServerScene::InstanceLightmapCaptureData *Data{ nullptr };
};

template <typename T>
struct MarkUpdate {
};
struct DirectionalLight {
};
struct Visible {
};
struct CullAABB {
	AABB aabb;
};
struct InstanceBoundsComponent {

	AABB aabb;
	AABB transformed_aabb;
	AABB custom_aabb; // <Zylann> would using aabb directly with a bool be better?
	float extra_margin{ 0.0f };
	bool use_custom_aabb{ false };

	//InstanceBoundsComponent() {
	//	use_custom_aabb = false;
	//}
};
struct ShadowWorkItem {
	//shadowtransform
	bool bUpdateTransform{ false };
	RID tf_p_light_instance;
	CameraMatrix tf_p_projection;
	Transform tf_p_transform;
	float tf_p_far;
	float tf_p_split;
	int tf_p_pass;
	float tf_p_bias_scale;
	//render
	bool bRender{ false };
	RID r_p_light;
	RID r_p_shadow_atlas;
	int r_p_pass;
	std::vector<RasterizerScene::InstanceBase *> r_cullresult;

	void light_instance_set_shadow_transform(RID p_light_instance, const CameraMatrix &p_projection, const Transform &p_transform, float p_far, float p_split, int p_pass, float p_bias_scale = 1.0) {
		bUpdateTransform = true;
		tf_p_light_instance = p_light_instance;
		tf_p_projection = p_projection;
		tf_p_transform = p_transform;
		tf_p_far = p_far;
		tf_p_split = p_split;
		tf_p_pass = p_pass;
		tf_p_bias_scale = p_bias_scale;
	}

	void render_shadow(RID p_light, RID p_shadow_atlas, int p_pass, std::vector<RasterizerScene::InstanceBase *> &cullresult) {
		bRender = true;
		r_p_light = p_light;
		r_p_shadow_atlas = p_shadow_atlas;
		r_p_pass = p_pass;
		r_cullresult = std::move(cullresult);
	}
};

moodycamel::ConcurrentQueue<ShadowWorkItem> ShadowWorkQueue;
std::future<void> async_lightmap_captures;
bool bAsyncLightmapsCalculating = false;
void JoinAsyncLigthmaps() {
	//if (bAsyncLightmapsCalculating) {
	//	async_lightmap_captures.get();
	//}
	//bAsyncLightmapsCalculating = false;
}

VisualServerScene::InstanceGeometryData *get_instance_geometry(RID id) {

	if (VSG::ecs->registry.valid(id.eid) && VSG::ecs->registry.has<GeometryComponent>(id.eid)) {
		return VSG::ecs->registry.get<GeometryComponent>(id.eid).Data;
	}
	return nullptr;
}

template <typename T>
bool has_component(EntityID id) {
	return VSG::ecs->registry.valid(id) && VSG::ecs->registry.has<T>(id);
}
template <typename T>
bool has_component(RID id) {
	return has_component<T>(id.eid);
}
template <typename T>
T &get_component(EntityID id) {

	CRASH_COND(!VSG::ecs->registry.valid(id));
	CRASH_COND(!VSG::ecs->registry.has<T>(id));

	return VSG::ecs->registry.get<T>(id);
}
template <typename T>
T &get_component(RID id) {

	return get_component<T>(id.eid);
}
template <typename T>
T &add_component(EntityID id) {

	CRASH_COND(!VSG::ecs->registry.valid(id));

	return VSG::ecs->registry.assign_or_replace<T>(id);
}

template <typename T>
T &add_component(RID id) {

	return add_component<T>(id.eid);
}
template <typename T>
void clear_component(EntityID id) {
	if (VSG::ecs->registry.valid(id) && VSG::ecs->registry.has<T>(id)) {
		VSG::ecs->registry.remove<T>(id);
	}
}

template <typename T>
void clear_component(RID id) {
	clear_component<T>(id.eid);
}

void set_dirty(RID id, bool p_update_aabb, bool p_update_materials) {

	auto &reg = VSG::ecs->registry;
	if (!has_component<Dirty>(id.eid)) {
		reg.assign_or_replace<Dirty>(id.eid, Dirty());
	}

	if (p_update_aabb)
		get_component<Dirty>(id.eid).update_aabb = true;
	if (p_update_materials)
		get_component<Dirty>(id.eid).update_materials = true;
}

moodycamel::ConcurrentQueue<VisualServerScene::Instance *> lightmap_update_queue;
template <typename T, typename QTraits, typename F>
void dequeue_concurrent_queue(moodycamel::ConcurrentQueue<T, QTraits> &queue, F &&functor) {
	constexpr size_t blocksize = QTraits::BLOCK_SIZE;
	T dequeued[blocksize];
	//SCOPE_PROFILE(InstancesCull);
	while (true) {
		size_t num = queue.try_dequeue_bulk(dequeued, blocksize);
		if (num <= 0)
			break;
		else {
			for (int i = 0; i < num; i++) {
				functor(dequeued[i]);
			}
		}
	}
}

template <typename T, typename QTraits, typename F>
void parallel_dequeue_concurrent_queue(moodycamel::ConcurrentQueue<T, QTraits> &queue, F &&functor) {
	constexpr size_t taskloop[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	std::for_each(STL_PARALLEL, &taskloop[0], &taskloop[7], [&queue, &functor](auto i) {
		SCOPE_PROFILE(parallel_dequeue);
		constexpr size_t blocksize = QTraits::BLOCK_SIZE;
		T dequeued[blocksize];

		while (true) {
			size_t num = queue.try_dequeue_bulk(dequeued, blocksize);
			if (num <= 0)
				break;
			else {
				for (int i = 0; i < num; i++) {
					functor(dequeued[i]);
				}
			}
		}
	});
}
template <typename T, typename QTraits, typename F>
void parallel_dequeue_concurrent_queue_unbatched(moodycamel::ConcurrentQueue<T, QTraits> &queue, F &&functor) {
	constexpr size_t taskloop[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	std::for_each(STL_PARALLEL, &taskloop[0], &taskloop[7], [&queue, &functor](auto i) {
		SCOPE_PROFILE(parallel_dequeue);
		constexpr size_t blocksize = QTraits::BLOCK_SIZE;
		T dequeued;

		while (queue.try_dequeue(dequeued)) {

			functor(dequeued);
		}
	});
}
template <typename Cont, typename F>
void parallel_for(Cont &container, F &&functor) {

	std::for_each(STL_PARALLEL, container.begin(), container.end(), functor);
}
FastOctree<EntityID> *get_geoctree(VisualServerScene::Scenario *sc) {
	return static_cast<FastOctree<EntityID> *>(sc->geometry_octree);
}
FastOctree<EntityID> *get_otheroctree(VisualServerScene::Scenario *sc) {
	return static_cast<FastOctree<EntityID> *>(sc->other_octree);
}

RID VisualServerScene::camera_create() {
	auto eid = VSG::ecs->registry.create();
	VSG::ecs->registry.assign<Camera>(eid, Camera());
	RID newid;
	newid.eid = eid;
	return newid;
	//Camera *camera = memnew(Camera);
	//return camera_owner.make_rid(camera);
}

void VisualServerScene::camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far) {

	if (VSG::ecs->registry.valid(p_camera.eid) && VSG::ecs->registry.has<Camera>(p_camera.eid)) {
		Camera &camera = VSG::ecs->registry.get<Camera>(p_camera.eid);

		camera.type = Camera::PERSPECTIVE;
		camera.fov = p_fovy_degrees;
		camera.znear = p_z_near;
		camera.zfar = p_z_far;
	} else {
		ERR_FAIL_COND(true);
	}

	/*
	Camera camera = //camera_owner.get(p_camera);
	ERR_FAIL_COND(!camera);
	camera->type = Camera::PERSPECTIVE;
	camera->fov = p_fovy_degrees;
	camera->znear = p_z_near;
	camera->zfar = p_z_far;
	*/
}

void VisualServerScene::camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far) {

	if (VSG::ecs->registry.valid(p_camera.eid) && VSG::ecs->registry.has<Camera>(p_camera.eid)) {
		Camera &camera = VSG::ecs->registry.get<Camera>(p_camera.eid);

		camera.type = Camera::ORTHOGONAL;
		camera.size = p_size;
		camera.znear = p_z_near;
		camera.zfar = p_z_far;
	} else {
		ERR_FAIL_COND(true);
	}
	/*
	Camera *camera = camera_owner.get(p_camera);
	ERR_FAIL_COND(!camera);
	camera->type = Camera::ORTHOGONAL;
	camera->size = p_size;
	camera->znear = p_z_near;
	camera->zfar = p_z_far;
	*/
}

void VisualServerScene::camera_set_transform(RID p_camera, const Transform &p_transform) {
	if (VSG::ecs->registry.valid(p_camera.eid) && VSG::ecs->registry.has<Camera>(p_camera.eid)) {
		Camera &camera = VSG::ecs->registry.get<Camera>(p_camera.eid);

		camera.transform = p_transform.orthonormalized();
	} else {
		ERR_FAIL_COND(true);
	}
	//Camera *camera = camera_owner.get(p_camera);
	//ERR_FAIL_COND(!camera);
	//camera->transform = p_transform.orthonormalized();
}

void VisualServerScene::camera_set_cull_mask(RID p_camera, uint32_t p_layers) {

	if (VSG::ecs->registry.valid(p_camera.eid) && VSG::ecs->registry.has<Camera>(p_camera.eid)) {
		Camera &camera = VSG::ecs->registry.get<Camera>(p_camera.eid);

		camera.visible_layers = p_layers;
	} else {
		ERR_FAIL_COND(true);
	}
	/*
	Camera *camera = camera_owner.get(p_camera);
	ERR_FAIL_COND(!camera);

	camera->visible_layers = p_layers;*/
}

void VisualServerScene::camera_set_environment(RID p_camera, RID p_env) {
	if (VSG::ecs->registry.valid(p_camera.eid) && VSG::ecs->registry.has<Camera>(p_camera.eid)) {
		Camera &camera = VSG::ecs->registry.get<Camera>(p_camera.eid);

		camera.env = p_env;
	} else {
		ERR_FAIL_COND(true);
	}
	/*Camera *camera = camera_owner.get(p_camera);
	ERR_FAIL_COND(!camera);
	camera->env = p_env;
	*/
}

void VisualServerScene::camera_set_use_vertical_aspect(RID p_camera, bool p_enable) {

	if (VSG::ecs->registry.valid(p_camera.eid) && VSG::ecs->registry.has<Camera>(p_camera.eid)) {
		Camera &camera = VSG::ecs->registry.get<Camera>(p_camera.eid);

		camera.vaspect = p_enable;
	}

	/*Camera *camera = camera_owner.get(p_camera);
	ERR_FAIL_COND(!camera);
	camera->vaspect = p_enable;*/
}

bool VisualServerScene::owns_camera(RID p_camera) {
	return VSG::ecs->registry.valid(p_camera.eid) && VSG::ecs->registry.has<Camera>(p_camera.eid);
}

/* SCENARIO API */

void *VisualServerScene::_instance_pair(void *p_self, OctreeElementID, Instance *p_A, int, OctreeElementID, Instance *p_B, int) {

	VisualServerScene *self = (VisualServerScene *)p_self;
	Instance *A = p_A;
	Instance *B = p_B;

	//instance indices are designed so greater always contains lesser
	if (A->base_type > B->base_type) {
		SWAP(A, B); //lesser always first
	}

	if (has_component<ReflectionProbeComponent>(B->self) && has_component<GeometryComponent>(A->self)) {

		InstanceReflectionProbeData *reflection_probe = static_cast<InstanceReflectionProbeData *>(B->base_data);
		InstanceGeometryData *geom = get_instance_geometry(A->self); //static_cast<InstanceGeometryData *>(A->base_data);

		InstanceReflectionProbeData::PairInfo pinfo;
		pinfo.geometry = A;
		pinfo.L = geom->reflection_probes.push_back(B);

		List<InstanceReflectionProbeData::PairInfo>::Element *E = reflection_probe->geometries.push_back(pinfo);

		get_component<GeometryComponent>(A->self).reflection_dirty = true;

		return E; //this element should make freeing faster
	} else if (has_component<LightmapCaptureComponent>(B->self) && has_component<GeometryComponent>(A->self)) {

		InstanceLightmapCaptureData *lightmap_capture = static_cast<InstanceLightmapCaptureData *>(B->base_data);
		InstanceGeometryData *geom = get_instance_geometry(A->self); //static_cast<InstanceGeometryData *>(A->base_data);

		InstanceLightmapCaptureData::PairInfo pinfo;
		pinfo.geometry = A;
		pinfo.L = geom->lightmap_captures.push_back(B);

		List<InstanceLightmapCaptureData::PairInfo>::Element *E = lightmap_capture->geometries.push_back(pinfo);
		((VisualServerScene *)p_self)->_instance_queue_update(A, false, false); //need to update capture

		return E; //this element should make freeing faster
	} else if (has_component<GIProbeComponent>(B->self) && has_component<GeometryComponent>(A->self)) {

		InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(B->base_data);
		InstanceGeometryData *geom = get_instance_geometry(A->self); //static_cast<InstanceGeometryData *>(A->base_data);

		InstanceGIProbeData::PairInfo pinfo;
		pinfo.geometry = A;
		pinfo.L = geom->gi_probes.push_back(B);

		List<InstanceGIProbeData::PairInfo>::Element *E = gi_probe->geometries.push_back(pinfo);

		get_component<GeometryComponent>(A->self).gi_probes_dirty = true;

		return E; //this element should make freeing faster

	} else if (has_component<GIProbeComponent>(B->self) && has_component<LightComponent>(A->self)) {

		InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(B->base_data);
		return gi_probe->lights.insert(A);
	}

	return NULL;
}
void VisualServerScene::_instance_unpair(void *p_self, OctreeElementID, Instance *p_A, int, OctreeElementID, Instance *p_B, int, void *udata) {

	VisualServerScene *self = (VisualServerScene *)p_self;
	Instance *A = p_A;
	Instance *B = p_B;

	//instance indices are designed so greater always contains lesser
	if (A->base_type > B->base_type) {
		SWAP(A, B); //lesser always first
	}

	if (has_component<ReflectionProbeComponent>(B->self) && has_component<GeometryComponent>(A->self)) {

		InstanceReflectionProbeData *reflection_probe = static_cast<InstanceReflectionProbeData *>(B->base_data);
		InstanceGeometryData *geom = get_instance_geometry(A->self); //static_cast<InstanceGeometryData *>(A->base_data);

		List<InstanceReflectionProbeData::PairInfo>::Element *E = reinterpret_cast<List<InstanceReflectionProbeData::PairInfo>::Element *>(udata);

		geom->reflection_probes.erase(E->get().L);
		reflection_probe->geometries.erase(E);

		get_component<GeometryComponent>(A->self).reflection_dirty = true;
	} else if (has_component<LightmapCaptureComponent>(B->self) && has_component<GeometryComponent>(A->self)) {

		InstanceLightmapCaptureData *lightmap_capture = static_cast<InstanceLightmapCaptureData *>(B->base_data);
		InstanceGeometryData *geom = get_instance_geometry(A->self); //static_cast<InstanceGeometryData *>(A->base_data);

		List<InstanceLightmapCaptureData::PairInfo>::Element *E = reinterpret_cast<List<InstanceLightmapCaptureData::PairInfo>::Element *>(udata);

		geom->lightmap_captures.erase(E->get().L);
		lightmap_capture->geometries.erase(E);
		((VisualServerScene *)p_self)->_instance_queue_update(A, false, false); //need to update capture

	} else if (has_component<GIProbeComponent>(B->self) && has_component<GeometryComponent>(A->self)) {

		InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(B->base_data);
		InstanceGeometryData *geom = get_instance_geometry(A->self); //static_cast<InstanceGeometryData *>(A->base_data);

		List<InstanceGIProbeData::PairInfo>::Element *E = reinterpret_cast<List<InstanceGIProbeData::PairInfo>::Element *>(udata);

		geom->gi_probes.erase(E->get().L);
		gi_probe->geometries.erase(E);

		get_component<GeometryComponent>(A->self).gi_probes_dirty = true;

	} else if (has_component<GIProbeComponent>(B->self) && has_component<LightComponent>(A->self)) {

		InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(B->base_data);
		Set<Instance *>::Element *E = reinterpret_cast<Set<Instance *>::Element *>(udata);

		gi_probe->lights.erase(E);
	}
}

template <typename F>
void scenario_cull_convex_instance(VisualServerScene::Scenario *sc, const Vector<Plane> &p_convex, F &&functor, uint32_t p_mask = 0xFFFFFFFF) {
	AUTO_PROFILE
	static_cast<FastOctree<EntityID> *>(sc->geometry_octree)->cull_convex(p_convex, [&](auto eid) {
		auto &ic = get_component<InstanceComponent>(eid);
		functor(ic.instance);
	});
	static_cast<FastOctree<EntityID> *>(sc->other_octree)->cull_convex(p_convex, [&](auto eid) {
		auto &ic = get_component<InstanceComponent>(eid);
		functor(ic.instance);
	});
}
template <typename F>
void scenario_cull_convex_entities(VisualServerScene::Scenario *sc, const Vector<Plane> &p_convex, F &&functor) {
	AUTO_PROFILE
	static_cast<FastOctree<EntityID> *>(sc->geometry_octree)->cull_convex(p_convex, functor);
	static_cast<FastOctree<EntityID> *>(sc->other_octree)->cull_convex(p_convex, functor);
}
template <typename F>
void scenario_cull_convex_geo(VisualServerScene::Scenario *sc, const Vector<Plane> &p_convex, F &&functor) {
	AUTO_PROFILE
	static_cast<FastOctree<EntityID> *>(sc->geometry_octree)->cull_convex(p_convex, functor);
}

template <typename F>
void scenario_cull_box_geo(VisualServerScene::Scenario *sc, const AABB &box, F &&functor) {
	AUTO_PROFILE
	static_cast<FastOctree<EntityID> *>(sc->geometry_octree)->cull_aabb(box, functor);
}

RID VisualServerScene::scenario_create() {

	Scenario *scenario = memnew(Scenario);
	ERR_FAIL_COND_V(!scenario, RID());
	RID scenario_rid = scenario_owner.make_rid(scenario);
	scenario->self = scenario_rid;

	scenario->octree.set_pair_callback(_instance_pair, this);
	scenario->octree.set_unpair_callback(_instance_unpair, this);

	scenario->geometry_octree = new FastOctree<EntityID>(Vector3(8000.0f, 8000.0f, 8000.0f));
	scenario->other_octree = new FastOctree<EntityID>(Vector3(8000.0f, 8000.0f, 8000.0f));

	scenario->reflection_probe_shadow_atlas = VSG::scene_render->shadow_atlas_create();
	VSG::scene_render->shadow_atlas_set_size(scenario->reflection_probe_shadow_atlas, 1024); //make enough shadows for close distance, don't bother with rest
	VSG::scene_render->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 0, 4);
	VSG::scene_render->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 1, 4);
	VSG::scene_render->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 2, 4);
	VSG::scene_render->shadow_atlas_set_quadrant_subdivision(scenario->reflection_probe_shadow_atlas, 3, 8);
	scenario->reflection_atlas = VSG::scene_render->reflection_atlas_create();

	return scenario_rid;
}

void VisualServerScene::scenario_set_debug(RID p_scenario, VS::ScenarioDebugMode p_debug_mode) {

	Scenario *scenario = scenario_owner.get(p_scenario);
	ERR_FAIL_COND(!scenario);
	scenario->debug = p_debug_mode;
}

void VisualServerScene::scenario_set_environment(RID p_scenario, RID p_environment) {

	Scenario *scenario = scenario_owner.get(p_scenario);
	ERR_FAIL_COND(!scenario);
	scenario->environment = p_environment;
}

void VisualServerScene::scenario_set_fallback_environment(RID p_scenario, RID p_environment) {

	Scenario *scenario = scenario_owner.get(p_scenario);
	ERR_FAIL_COND(!scenario);
	scenario->fallback_environment = p_environment;
}

void VisualServerScene::scenario_set_reflection_atlas_size(RID p_scenario, int p_size, int p_subdiv) {

	Scenario *scenario = scenario_owner.get(p_scenario);
	ERR_FAIL_COND(!scenario);
	VSG::scene_render->reflection_atlas_set_size(scenario->reflection_atlas, p_size);
	VSG::scene_render->reflection_atlas_set_subdivision(scenario->reflection_atlas, p_subdiv);
}

/* INSTANCING API */

void VisualServerScene::_instance_queue_update(Instance *p_instance, bool p_update_aabb, bool p_update_materials) {

	set_dirty(p_instance->self, p_update_aabb, p_update_materials);
}

// from can be mesh, light,  area and portal so far.
RID VisualServerScene::instance_create() {

	Instance *instance = memnew(Instance);
	ERR_FAIL_COND_V(!instance, RID());

	RID instance_rid = instance_owner.make_rid(instance);
	instance_rid.eid = VSG::ecs->registry.create();
	instance->self = instance_rid;
	VSG::ecs->registry.assign_or_replace<InstanceComponent>(instance_rid.eid, instance, instance_rid);
	VSG::ecs->registry.assign_or_replace<Visible>(instance_rid.eid);
	VSG::ecs->registry.assign_or_replace<InstanceBoundsComponent>(instance_rid.eid, InstanceBoundsComponent());
	return instance_rid;
}

void VisualServerScene::instance_set_base(RID p_instance, RID p_base) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	Scenario *scenario = instance->scenario;

	if (instance->base_type != VS::INSTANCE_NONE) {
		//free anything related to that base

		VSG::storage->instance_remove_dependency(instance->base, instance);

		if (instance->base_type == VS::INSTANCE_GI_PROBE) {
			//if gi probe is baking, wait until done baking, else race condition may happen when removing it
			//from octree
			InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(instance->base_data);

			//make sure probes are done baking
			while (!probe_bake_list.empty()) {
				OS::get_singleton()->delay_usec(1);
			}
			//make sure this one is done baking

			while (gi_probe->dynamic.updating_stage == GI_UPDATE_STAGE_LIGHTING) {
				//wait until bake is done if it's baking
				OS::get_singleton()->delay_usec(1);
			}
		}

		if (scenario && instance->octree_id) {
			scenario->octree.erase(instance->octree_id); //make dependencies generated by the octree go away

			instance->octree_id = 0;
			if (has_component<GeometryComponent>(instance->self)) {
				get_geoctree(scenario)->RemoveOctreeElement(instance->octree_index);
			} else {
				get_otheroctree(scenario)->RemoveOctreeElement(instance->octree_index);
			}
			instance->octree_index = 0;
		}

		switch (instance->base_type) {
			case VS::INSTANCE_LIGHT: {

				if (has_component<DirectionalLight>(instance->self)) {
					clear_component<DirectionalLight>(instance->self);
				}

				VSG::scene_render->free(get_component<LightComponent>(instance->self).light_instance);

				clear_component<LightComponent>(instance->self);

			} break;
			case VS::INSTANCE_REFLECTION_PROBE: {

				ReflectionProbeComponent &cmp = get_component<ReflectionProbeComponent>(instance->self);

				InstanceReflectionProbeData *reflection_probe = static_cast<InstanceReflectionProbeData *>(instance->base_data);
				VSG::scene_render->free(cmp.instance);
				//if (reflection_probe->update_list.in_list()) {
				//	reflection_probe_render_list.remove(&reflection_probe->update_list);
				//}
				clear_component<MarkUpdate<ReflectionProbeComponent> >(instance->self);

				clear_component<ReflectionProbeComponent>(instance->self);

			} break;
			case VS::INSTANCE_LIGHTMAP_CAPTURE: {

				InstanceLightmapCaptureData *lightmap_capture = static_cast<InstanceLightmapCaptureData *>(instance->base_data);
				//erase dependencies, since no longer a lightmap
				clear_component<LightmapCaptureComponent>(instance->self);
				while (lightmap_capture->users.front()) {
					instance_set_use_lightmap(lightmap_capture->users.front()->get()->self, RID(), RID());
				}
			} break;
			case VS::INSTANCE_GI_PROBE: {

				InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(instance->base_data);

				//if (gi_probe->update_element.in_list()) {
				//	gi_probe_update_list.remove(&gi_probe->update_element);
				//}
				if (gi_probe->dynamic.probe_data.is_valid()) {
					VSG::storage->free(gi_probe->dynamic.probe_data);
				}

				if (instance->lightmap_capture) {
					Instance *capture = (Instance *)instance->lightmap_capture;
					InstanceLightmapCaptureData *lightmap_capture = static_cast<InstanceLightmapCaptureData *>(capture->base_data);
					lightmap_capture->users.erase(instance);
					instance->lightmap_capture = NULL;
					instance->lightmap = RID();
				}
				clear_component<MarkUpdate<GIProbeComponent> >(instance->self);
				clear_component<GIProbeComponent>(instance->self);

				VSG::scene_render->free(gi_probe->probe_instance);

			} break;
			default: {}
		}

		if (instance->base_data) {
			memdelete(instance->base_data);
			instance->base_data = NULL;
		}

		instance->blend_values.clear();

		for (int i = 0; i < instance->materials.size(); i++) {
			if (instance->materials[i].is_valid()) {
				VSG::storage->material_remove_instance_owner(instance->materials[i], instance);
			}
		}
		instance->materials.clear();
	}

	instance->base_type = VS::INSTANCE_NONE;
	instance->base = RID();

	if (p_base.is_valid()) {

		instance->base_type = VSG::storage->get_base_type(p_base);
		ERR_FAIL_COND(instance->base_type == VS::INSTANCE_NONE);

		switch (instance->base_type) {
			case VS::INSTANCE_LIGHT: {

				//InstanceLightData *light = memnew(InstanceLightData);

				if (scenario && VSG::storage->light_get_type(p_base) == VS::LIGHT_DIRECTIONAL) {
					//light->D = scenario->directional_lights.push_back(instance);
					add_component<DirectionalLight>(instance->self);
					//VSG::ecs->registry.assign_or_replace<DirectionalLight>(instance->self.eid);
				}
				auto instance_rid = VSG::scene_render->light_instance_create(p_base);
				//light->instance = VSG::scene_render->light_instance_create(p_base);

				VSG::ecs->registry.assign_or_replace<LightComponent>(instance->self.eid);
				get_component<LightComponent>(instance->self.eid).light_instance = instance_rid;
				instance->base_data = nullptr;
			} break;
			case VS::INSTANCE_MESH:
			case VS::INSTANCE_MULTIMESH:
			case VS::INSTANCE_IMMEDIATE:
			case VS::INSTANCE_PARTICLES: {

				InstanceGeometryData *geom = memnew(InstanceGeometryData);
				add_component<GeometryComponent>(instance->self.eid);
				get_component<GeometryComponent>(instance->self.eid).Data = geom;

			} break;
			case VS::INSTANCE_REFLECTION_PROBE: {

				InstanceReflectionProbeData *reflection_probe = memnew(InstanceReflectionProbeData);

				add_component<ReflectionProbeComponent>(instance->self);
				ReflectionProbeComponent &cmp = get_component<ReflectionProbeComponent>(instance->self);
				cmp.owner = instance;
				reflection_probe->owner = instance;
				instance->base_data = reflection_probe;
				cmp.Data = reflection_probe;
				cmp.instance = VSG::scene_render->reflection_probe_instance_create(p_base);

			} break;
			case VS::INSTANCE_LIGHTMAP_CAPTURE: {

				InstanceLightmapCaptureData *lightmap_capture = memnew(InstanceLightmapCaptureData);
				instance->base_data = lightmap_capture;
				add_component<LightmapCaptureComponent>(instance->self);
				LightmapCaptureComponent &cmp = get_component<LightmapCaptureComponent>(instance->self);
				cmp.Data = lightmap_capture;
				cmp.owner = instance;
				//lightmap_capture->instance = VSG::scene_render->lightmap_capture_instance_create(p_base);
			} break;
			case VS::INSTANCE_GI_PROBE: {

				InstanceGIProbeData *gi_probe = memnew(InstanceGIProbeData);
				instance->base_data = gi_probe;
				gi_probe->owner = instance;

				//if (scenario && !gi_probe->update_element.in_list()) {
				//	gi_probe_update_list.add(&gi_probe->update_element);
				//}
				add_component<MarkUpdate<GIProbeComponent> >(instance->self);
				add_component<GIProbeComponent>(instance->self);
				GIProbeComponent &cmp = get_component<GIProbeComponent>(instance->self);
				cmp.Data = gi_probe;
				cmp.owner = instance;

				gi_probe->probe_instance = VSG::scene_render->gi_probe_instance_create();

			} break;
			default: {}
		}

		VSG::storage->instance_add_dependency(p_base, instance);

		instance->base = p_base;

		if (scenario)
			_instance_queue_update(instance, true, true);
	}
}
void VisualServerScene::Scenario::insert_instance(RID instance) {

	if (has_component<ScenarioLink>(instance)) {
		ScenarioLink &link = get_component<ScenarioLink>(instance);
		if (link.owner != this && link.owner->entity_list.valid(link.scenario_id)) {
			link.owner->entity_list.destroy(link.scenario_id);
		}
	}
	add_component<ScenarioLink>(instance);
	ScenarioLink &link = get_component<ScenarioLink>(instance);

	link.scenario_id = entity_list.create();

	entity_list.assign_or_replace<InstanceComponent>(link.scenario_id, get_component<InstanceComponent>(instance));

	link.owner = this;
}

void VisualServerScene::Scenario::remove_instance(RID instance) {
	if (has_component<ScenarioLink>(instance)) {

		ScenarioLink &link = get_component<ScenarioLink>(instance);
		if (link.owner == this && link.owner->entity_list.valid(link.scenario_id)) {

			link.owner->entity_list.destroy(link.scenario_id);
		}
	}
}

void VisualServerScene::instance_set_scenario(RID p_instance, RID p_scenario) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	if (instance->scenario) {

		instance->scenario->remove_instance(p_instance);
		//instance->scenario->instances.remove(&instance->scenario_item);

		if (instance->octree_id) {
			instance->scenario->octree.erase(instance->octree_id); //make dependencies generated by the octree go away
			instance->octree_id = 0;
		}
		if (has_component<GeometryComponent>(instance->self)) {
			get_geoctree(instance->scenario)->RemoveOctreeElement(instance->octree_index);
		} else {
			get_otheroctree(instance->scenario)->RemoveOctreeElement(instance->octree_index);
		}
		//get_geoctree(instance->scenario)->RemoveOctreeElement(instance->octree_index);
		instance->octree_index = 0;

		switch (instance->base_type) {

			case VS::INSTANCE_LIGHT: {

				//InstanceLightData *light = static_cast<InstanceLightData *>(instance->base_data);

				//if (light->D) {
				//	instance->scenario->directional_lights.erase(light->D);
				//	light->D = NULL;
				//}
			} break;
			case VS::INSTANCE_REFLECTION_PROBE: {

				//InstanceReflectionProbeData *reflection_probe = static_cast<InstanceReflectionProbeData *>(instance->base_data);
				ReflectionProbeComponent &cmp = get_component<ReflectionProbeComponent>(instance->self);
				VSG::scene_render->reflection_probe_release_atlas_index(cmp.instance);
			} break;
			case VS::INSTANCE_GI_PROBE: {

				clear_component<MarkUpdate<GIProbeComponent> >(instance->self);
				//InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(instance->base_data);
				//
				//if (gi_probe->update_element.in_list()) {
				//	gi_probe_update_list.remove(&gi_probe->update_element);
				//}
			} break;
			default: {}
		}

		instance->scenario = NULL;
	}

	if (p_scenario.is_valid()) {

		Scenario *scenario = scenario_owner.get(p_scenario);
		ERR_FAIL_COND(!scenario);

		instance->scenario = scenario;

		instance->scenario->insert_instance(p_instance);
		//		scenario->instances.add(&instance->scenario_item);

		switch (instance->base_type) {

			case VS::INSTANCE_LIGHT: {

				//InstanceLightData *light = static_cast<InstanceLightData *>(instance->base_data);

				//if (VSG::storage->light_get_type(instance->base) == VS::LIGHT_DIRECTIONAL) {
				//	light->D = scenario->directional_lights.push_back(instance);
				//}
			} break;
			case VS::INSTANCE_GI_PROBE: {
				add_component<MarkUpdate<GIProbeComponent> >(instance->self);
				//InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(instance->base_data);
				//if (!gi_probe->update_element.in_list()) {
				//	gi_probe_update_list.add(&gi_probe->update_element);
				//}
			} break;
			default: {}
		}

		_instance_queue_update(instance, true, true);
	}
}
void VisualServerScene::instance_set_layer_mask(RID p_instance, uint32_t p_mask) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	instance->layer_mask = p_mask;
}
void VisualServerScene::instance_set_transform(RID p_instance, const Transform &p_transform) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	if (instance->transform == p_transform)
		return; //must be checked to avoid worst evil

	instance->transform = p_transform;
	_instance_queue_update(instance, true);
}
void VisualServerScene::instance_attach_object_instance_id(RID p_instance, ObjectID p_ID) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	instance->object_ID = p_ID;
}
void VisualServerScene::instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	if (!has_component<Dirty>(p_instance)) {
		_update_dirty_instance(instance);
	}

	ERR_FAIL_INDEX(p_shape, instance->blend_values.size());
	instance->blend_values.write[p_shape] = p_weight;
}

void VisualServerScene::instance_set_surface_material(RID p_instance, int p_surface, RID p_material) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	if (instance->base_type == VS::INSTANCE_MESH) {
		//may not have been updated yet
		instance->materials.resize(VSG::storage->mesh_get_surface_count(instance->base));
	}

	ERR_FAIL_INDEX(p_surface, instance->materials.size());

	if (instance->materials[p_surface].is_valid()) {
		VSG::storage->material_remove_instance_owner(instance->materials[p_surface], instance);
	}
	instance->materials.write[p_surface] = p_material;
	instance->base_changed(false, true);

	if (instance->materials[p_surface].is_valid()) {
		VSG::storage->material_add_instance_owner(instance->materials[p_surface], instance);
	}
}

void VisualServerScene::instance_set_visible(RID p_instance, bool p_visible) {

	//Instance *instance = instance_owner.get(p_instance);
	//ERR_FAIL_COND(!instance);
	//
	//bool visible = has_component<Visible>(p_instance);
	//
	//if (visible == p_visible)
	//	return;

	if (p_visible) {

		add_component<Visible>(p_instance);
	} else {
		clear_component<Visible>(p_instance);
	}
	if (has_component<ScenarioLink>(p_instance)) {
		ScenarioLink &link = get_component<ScenarioLink>(p_instance);

		bool bVisible = link.owner->entity_list.has<Visible>(link.scenario_id);
		if (bVisible != p_visible) {

			if (p_visible) {
				link.owner->entity_list.assign_or_replace<Visible>(link.scenario_id);
			} else {
				link.owner->entity_list.remove<Visible>(link.scenario_id);
			}
		}
	}
	//switch (instance->base_type) {
	//	case VS::INSTANCE_LIGHT: {
	//		if (VSG::storage->light_get_type(instance->base) != VS::LIGHT_DIRECTIONAL && instance->octree_id && instance->scenario) {
	//			instance->scenario->octree.set_pairable(instance->octree_id, p_visible, 1 << VS::INSTANCE_LIGHT, p_visible ? VS::INSTANCE_GEOMETRY_MASK : 0);
	//		}
	//
	//	} break;
	//	case VS::INSTANCE_REFLECTION_PROBE: {
	//		if (instance->octree_id && instance->scenario) {
	//			instance->scenario->octree.set_pairable(instance->octree_id, p_visible, 1 << VS::INSTANCE_REFLECTION_PROBE, p_visible ? VS::INSTANCE_GEOMETRY_MASK : 0);
	//		}
	//
	//	} break;
	//	case VS::INSTANCE_LIGHTMAP_CAPTURE: {
	//		if (instance->octree_id && instance->scenario) {
	//			instance->scenario->octree.set_pairable(instance->octree_id, p_visible, 1 << VS::INSTANCE_LIGHTMAP_CAPTURE, p_visible ? VS::INSTANCE_GEOMETRY_MASK : 0);
	//		}
	//
	//	} break;
	//	case VS::INSTANCE_GI_PROBE: {
	//		if (instance->octree_id && instance->scenario) {
	//			instance->scenario->octree.set_pairable(instance->octree_id, p_visible, 1 << VS::INSTANCE_GI_PROBE, p_visible ? (VS::INSTANCE_GEOMETRY_MASK | (1 << VS::INSTANCE_LIGHT)) : 0);
	//		}
	//
	//	} break;
	//	default: {}
	//}
}
inline bool is_geometry_instance(VisualServer::InstanceType p_type) {
	return p_type == VS::INSTANCE_MESH || p_type == VS::INSTANCE_MULTIMESH || p_type == VS::INSTANCE_PARTICLES || p_type == VS::INSTANCE_IMMEDIATE;
}

void VisualServerScene::instance_set_use_lightmap(RID p_instance, RID p_lightmap_instance, RID p_lightmap) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	if (instance->lightmap_capture) {
		InstanceLightmapCaptureData *lightmap_capture = static_cast<InstanceLightmapCaptureData *>(((Instance *)instance->lightmap_capture)->base_data);
		lightmap_capture->users.erase(instance);
		instance->lightmap = RID();
		instance->lightmap_capture = NULL;
	}

	if (p_lightmap_instance.is_valid()) {
		Instance *lightmap_instance = instance_owner.get(p_lightmap_instance);
		ERR_FAIL_COND(!lightmap_instance);
		ERR_FAIL_COND(lightmap_instance->base_type != VS::INSTANCE_LIGHTMAP_CAPTURE);
		instance->lightmap_capture = lightmap_instance;

		InstanceLightmapCaptureData *lightmap_capture = static_cast<InstanceLightmapCaptureData *>(((Instance *)instance->lightmap_capture)->base_data);
		lightmap_capture->users.insert(instance);
		instance->lightmap = p_lightmap;
	}
}

void VisualServerScene::instance_set_custom_aabb(RID p_instance, AABB p_aabb) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);
	ERR_FAIL_COND(!is_geometry_instance(instance->base_type));

	InstanceBoundsComponent &bounds = get_component<InstanceBoundsComponent>(p_instance);

	if (p_aabb != AABB()) {

		// Set custom AABB
		bounds.custom_aabb = p_aabb;
		bounds.use_custom_aabb = true;

	} else {

		bounds.use_custom_aabb = false;
	}

	if (instance->scenario)
		_instance_queue_update(instance, true, false);
}

void VisualServerScene::instance_attach_skeleton(RID p_instance, RID p_skeleton) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	if (instance->skeleton == p_skeleton)
		return;

	if (instance->skeleton.is_valid()) {
		VSG::storage->instance_remove_skeleton(instance->skeleton, instance);
	}

	instance->skeleton = p_skeleton;

	if (instance->skeleton.is_valid()) {
		VSG::storage->instance_add_skeleton(instance->skeleton, instance);
	}

	_instance_queue_update(instance, true);
}

void VisualServerScene::instance_set_exterior(RID p_instance, bool p_enabled) {
}

void VisualServerScene::instance_set_extra_visibility_margin(RID p_instance, real_t p_margin) {
	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	InstanceBoundsComponent &bounds = get_component<InstanceBoundsComponent>(p_instance);
	bounds.extra_margin = p_margin;
	_instance_queue_update(instance, true, false);
}

Vector<ObjectID> VisualServerScene::instances_cull_aabb(const AABB &p_aabb, RID p_scenario) const {

	Vector<ObjectID> instances;
	Scenario *scenario = scenario_owner.get(p_scenario);
	ERR_FAIL_COND_V(!scenario, instances);

	const_cast<VisualServerScene *>(this)->update_dirty_instances(); // check dirty instances before culling
	JoinAsyncLigthmaps();
	int culled = 0;
	Instance *cull[1024];
	culled = scenario->octree.cull_aabb(p_aabb, cull, 1024);

	for (int i = 0; i < culled; i++) {

		Instance *instance = cull[i];
		ERR_CONTINUE(!instance);
		if (instance->object_ID == 0)
			continue;

		instances.push_back(instance->object_ID);
	}

	return instances;
}
Vector<ObjectID> VisualServerScene::instances_cull_ray(const Vector3 &p_from, const Vector3 &p_to, RID p_scenario) const {

	Vector<ObjectID> instances;
	Scenario *scenario = scenario_owner.get(p_scenario);
	ERR_FAIL_COND_V(!scenario, instances);
	const_cast<VisualServerScene *>(this)->update_dirty_instances(); // check dirty instances before culling
	JoinAsyncLigthmaps();
	int culled = 0;
	Instance *cull[1024];
	culled = scenario->octree.cull_segment(p_from, p_from + p_to * 10000, cull, 1024);

	for (int i = 0; i < culled; i++) {
		Instance *instance = cull[i];
		ERR_CONTINUE(!instance);
		if (instance->object_ID == 0)
			continue;

		instances.push_back(instance->object_ID);
	}

	return instances;
}
Vector<ObjectID> VisualServerScene::instances_cull_convex(const Vector<Plane> &p_convex, RID p_scenario) const {

	Vector<ObjectID> instances;
	Scenario *scenario = scenario_owner.get(p_scenario);
	ERR_FAIL_COND_V(!scenario, instances);
	const_cast<VisualServerScene *>(this)->update_dirty_instances(); // check dirty instances before culling
	JoinAsyncLigthmaps();
	int culled = 0;
	Instance *cull[1024];

	culled = scenario->octree.cull_convex(p_convex, cull, 1024);

	for (int i = 0; i < culled; i++) {

		Instance *instance = cull[i];
		ERR_CONTINUE(!instance);
		if (instance->object_ID == 0)
			continue;

		instances.push_back(instance->object_ID);
	}

	return instances;
}

void VisualServerScene::instance_geometry_set_flag(RID p_instance, VS::InstanceFlags p_flags, bool p_enabled) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	switch (p_flags) {

		case VS::INSTANCE_FLAG_USE_BAKED_LIGHT: {

			instance->baked_light = p_enabled;

		} break;
		case VS::INSTANCE_FLAG_DRAW_NEXT_FRAME_IF_VISIBLE: {

			instance->redraw_if_visible = p_enabled;

		} break;
		default: {}
	}
}
void VisualServerScene::instance_geometry_set_cast_shadows_setting(RID p_instance, VS::ShadowCastingSetting p_shadow_casting_setting) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	instance->cast_shadows = p_shadow_casting_setting;
	instance->base_changed(false, true); // to actually compute if shadows are visible or not
}
void VisualServerScene::instance_geometry_set_material_override(RID p_instance, RID p_material) {

	Instance *instance = instance_owner.get(p_instance);
	ERR_FAIL_COND(!instance);

	if (instance->material_override.is_valid()) {
		VSG::storage->material_remove_instance_owner(instance->material_override, instance);
	}
	instance->material_override = p_material;
	instance->base_changed(false, true);

	if (instance->material_override.is_valid()) {
		VSG::storage->material_add_instance_owner(instance->material_override, instance);
	}
}

void VisualServerScene::instance_geometry_set_draw_range(RID p_instance, float p_min, float p_max, float p_min_margin, float p_max_margin) {
}
void VisualServerScene::instance_geometry_set_as_instance_lod(RID p_instance, RID p_as_lod_of_instance) {
}

void VisualServerScene::_update_instance(Instance *p_instance) {

	p_instance->version++;
	InstanceBoundsComponent &bounds = get_component<InstanceBoundsComponent>(p_instance->self);
	if (has_component<LightComponent>(p_instance->self)) {

		LightComponent &light_comp = get_component<LightComponent>(p_instance->self);
		//InstanceLightData *light = light_comp.Data;

		VSG::scene_render->light_instance_set_transform(light_comp.light_instance, p_instance->transform);
		light_comp.shadow_dirty = true;
	}

	const bool visible = has_component<Visible>(p_instance->self);
	if (has_component<ReflectionProbeComponent>(p_instance->self)) {
		ReflectionProbeComponent &cmp = get_component<ReflectionProbeComponent>(p_instance->self);
		//InstanceReflectionProbeData *reflection_probe = static_cast<InstanceReflectionProbeData *>(p_instance->base_data);
		//
		VSG::scene_render->reflection_probe_instance_set_transform(cmp.instance, p_instance->transform);
		cmp.reflection_dirty = true;
	}

	if (p_instance->base_type == VS::INSTANCE_PARTICLES) {

		VSG::storage->particles_set_emission_transform(p_instance->base, p_instance->transform);
	}

	if (bounds.aabb.has_no_surface()) {
		return;
	}

	if (has_component<GeometryComponent>(p_instance->self)) {

		InstanceGeometryData *geom = get_instance_geometry(p_instance->self);
		//make sure lights are updated if it casts shadow

		if (!p_instance->lightmap_capture && geom->lightmap_captures.size()) {

			//affected by lightmap captures, must update capture info!
			lightmap_update_queue.enqueue(p_instance);
			//_update_instance_lightmap_captures(p_instance);
		} else {
			if (!p_instance->lightmap_capture_data.empty()) {
				p_instance->lightmap_capture_data.resize(0); //not in use, clear capture data
			}
		}
	}

	p_instance->mirror = p_instance->transform.basis.determinant() < 0.0;

	AABB new_aabb;
	AABB old_aabb;
	new_aabb = p_instance->transform.xform(bounds.aabb);
	old_aabb = bounds.transformed_aabb;

	bool sameAABB = (new_aabb == bounds.transformed_aabb);
	bounds.transformed_aabb = new_aabb;

	//trigger dirty flags
	if (has_component<GeometryComponent>(p_instance->self)) {

		auto &GeoComp = get_component<GeometryComponent>(p_instance->self);
		if (!sameAABB || p_instance->skeleton.is_valid()) {
			GeoComp.lighting_dirty = true;
			GeoComp.reflection_dirty = true;
			GeoComp.gi_probes_dirty = true;

			if (GeoComp.can_cast_shadows) {

				for (auto e : GeoComp.AffectingLights) {
					if (has_component<LightComponent>(e)) {

						get_component<LightComponent>(e).shadow_dirty = true;
					}
				}
			}
			GeoComp.AffectingLights.clear();
		}
	}

	if (!p_instance->scenario) {

		return;
	}

	auto &rg = p_instance->scenario->entity_list;

	{
		//make sure to dirty all the meshes that change
		SCOPE_PROFILE(LightSetDirty)
		if (has_component<LightComponent>(p_instance->self) && !sameAABB) {

			//auto &group = p_instance->scenario->entity_list.group<CullAABB, InstanceComponent, Visible>();
			//std::for_each(group.begin(), group.end(),
			AABB all_box = old_aabb;
			all_box.merge_with(new_aabb);
			scenario_cull_box_geo(p_instance->scenario, all_box,
					[this, &old_aabb, &new_aabb](auto eid) {
						auto &bounds = get_component<InstanceBoundsComponent>(eid);
						const bool intersectsOld = bounds.transformed_aabb.intersects(old_aabb);
						const bool intersectsNew = bounds.transformed_aabb.intersects(new_aabb);

						if ((intersectsOld && !intersectsNew) || (!intersectsOld && intersectsNew)) {

							get_component<GeometryComponent>(eid).lighting_dirty = true;
						}
					});
		} /*else if (has_component<ReflectionProbeComponent>(p_instance->self) && !sameAABB) {
				 auto &group = p_instance->scenario->entity_list.group<CullAABB, InstanceComponent, Visible>();
				 std::for_each(group.begin(), group.end(),
					   [this, &old_aabb, &new_aabb, &group](auto e) {
						   if (has_component<GeometryComponent>(group.get<InstanceComponent>(e).self_ID)) {

							   const bool intersectsOld = group.get<CullAABB>(e).aabb.intersects(old_aabb);
							   const bool intersectsNew = group.get<CullAABB>(e).aabb.intersects(new_aabb);

							   if ((intersectsOld && !intersectsNew) || (!intersectsOld && intersectsNew)) {

								   get_component<GeometryComponent>(group.get<InstanceComponent>(e).self_ID).reflection_dirty = true;
							   }
						   }
					   });
		}*/
	}

	ScenarioLink &sl = get_component<ScenarioLink>(p_instance->self);

	rg.assign_or_replace<CullAABB>(sl.scenario_id, CullAABB{ new_aabb });
	auto oct = has_component<GeometryComponent>(p_instance->self) ? get_geoctree(p_instance->scenario) : get_otheroctree(p_instance->scenario);
	if (p_instance->octree_id == 0) {

		uint32_t base_type = 1 << p_instance->base_type;
		uint32_t pairable_mask = 0;
		bool pairable = false;

		if (p_instance->base_type == VS::INSTANCE_LIGHT || p_instance->base_type == VS::INSTANCE_REFLECTION_PROBE || p_instance->base_type == VS::INSTANCE_LIGHTMAP_CAPTURE) {

			pairable_mask = visible ? VS::INSTANCE_GEOMETRY_MASK : 0;
			pairable = true;
		}

		if (p_instance->base_type == VS::INSTANCE_GI_PROBE) {
			//lights and geometries
			pairable_mask = visible ? VS::INSTANCE_GEOMETRY_MASK | (1 << VS::INSTANCE_LIGHT) : 0;
			pairable = true;
		}

		// not inside octree
		p_instance->octree_id = p_instance->scenario->octree.create(p_instance, new_aabb, 0, pairable, base_type, pairable_mask);

		p_instance->octree_index = oct->AddElement(p_instance->self.eid, new_aabb);

	} else {

		if (sameAABB)
			return;

		p_instance->scenario->octree.move(p_instance->octree_id, new_aabb);
		oct->MoveElement(p_instance->octree_index, new_aabb);
	}
}

void VisualServerScene::_update_instance_aabb(Instance *p_instance) {
	AABB new_aabb;

	ERR_FAIL_COND(p_instance->base_type != VS::INSTANCE_NONE && !p_instance->base.is_valid());

	InstanceBoundsComponent &bounds = get_component<InstanceBoundsComponent>(p_instance->self);

	switch (p_instance->base_type) {
		case VisualServer::INSTANCE_NONE: {

			// do nothing
		} break;
		case VisualServer::INSTANCE_MESH: {

			if (bounds.use_custom_aabb)
				new_aabb = bounds.custom_aabb;
			else
				new_aabb = VSG::storage->mesh_get_aabb(p_instance->base, p_instance->skeleton);

		} break;

		case VisualServer::INSTANCE_MULTIMESH: {

			if (bounds.use_custom_aabb)
				new_aabb = bounds.custom_aabb;
			else
				new_aabb = VSG::storage->multimesh_get_aabb(p_instance->base);

		} break;
		case VisualServer::INSTANCE_IMMEDIATE: {

			if (bounds.use_custom_aabb)
				new_aabb = bounds.custom_aabb;
			else
				new_aabb = VSG::storage->immediate_get_aabb(p_instance->base);

		} break;
		case VisualServer::INSTANCE_PARTICLES: {

			if (bounds.use_custom_aabb)
				new_aabb = bounds.custom_aabb;
			else
				new_aabb = VSG::storage->particles_get_aabb(p_instance->base);

		} break;
		case VisualServer::INSTANCE_LIGHT: {

			new_aabb = VSG::storage->light_get_aabb(p_instance->base);

		} break;
		case VisualServer::INSTANCE_REFLECTION_PROBE: {

			new_aabb = VSG::storage->reflection_probe_get_aabb(p_instance->base);

		} break;
		case VisualServer::INSTANCE_GI_PROBE: {

			new_aabb = VSG::storage->gi_probe_get_bounds(p_instance->base);

		} break;
		case VisualServer::INSTANCE_LIGHTMAP_CAPTURE: {

			new_aabb = VSG::storage->lightmap_capture_get_bounds(p_instance->base);

		} break;
		default: {}
	}

	// <Zylann> This is why I didn't re-use Instance::aabb to implement custom AABBs
	if (bounds.extra_margin)
		new_aabb.grow_by(bounds.extra_margin);

	bounds.aabb = new_aabb;
}

_FORCE_INLINE_ static void _light_capture_sample_octree(const RasterizerStorage::LightmapCaptureOctree *p_octree, int p_cell_subdiv, const Vector3 &p_pos, const Vector3 &p_dir, float p_level, Vector3 &r_color, float &r_alpha) {

	static const Vector3 aniso_normal[6] = {
		Vector3(-1, 0, 0),
		Vector3(1, 0, 0),
		Vector3(0, -1, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, -1),
		Vector3(0, 0, 1)
	};

	int size = 1 << (p_cell_subdiv - 1);

	int clamp_v = size - 1;
	//first of all, clamp
	Vector3 pos;
	pos.x = CLAMP(p_pos.x, 0, clamp_v);
	pos.y = CLAMP(p_pos.y, 0, clamp_v);
	pos.z = CLAMP(p_pos.z, 0, clamp_v);

	float level = (p_cell_subdiv - 1) - p_level;

	int target_level;
	float level_filter;
	if (level <= 0.0) {
		level_filter = 0;
		target_level = 0;
	} else {
		target_level = Math::ceil(level);
		level_filter = target_level - level;
	}

	Vector3 color[2][8];
	float alpha[2][8];
	zeromem(alpha, sizeof(float) * 2 * 8);

	//find cell at given level first

	for (int c = 0; c < 2; c++) {

		int current_level = MAX(0, target_level - c);
		int level_cell_size = (1 << (p_cell_subdiv - 1)) >> current_level;

		for (int n = 0; n < 8; n++) {

			int x = int(pos.x);
			int y = int(pos.y);
			int z = int(pos.z);

			if (n & 1)
				x += level_cell_size;
			if (n & 2)
				y += level_cell_size;
			if (n & 4)
				z += level_cell_size;

			int ofs_x = 0;
			int ofs_y = 0;
			int ofs_z = 0;

			x = CLAMP(x, 0, clamp_v);
			y = CLAMP(y, 0, clamp_v);
			z = CLAMP(z, 0, clamp_v);

			int half = size / 2;
			uint32_t cell = 0;
			for (int i = 0; i < current_level; i++) {

				const RasterizerStorage::LightmapCaptureOctree *bc = &p_octree[cell];

				int child = 0;
				if (x >= ofs_x + half) {
					child |= 1;
					ofs_x += half;
				}
				if (y >= ofs_y + half) {
					child |= 2;
					ofs_y += half;
				}
				if (z >= ofs_z + half) {
					child |= 4;
					ofs_z += half;
				}

				cell = bc->children[child];
				if (cell == RasterizerStorage::LightmapCaptureOctree::CHILD_EMPTY)
					break;

				half >>= 1;
			}

			if (cell == RasterizerStorage::LightmapCaptureOctree::CHILD_EMPTY) {
				alpha[c][n] = 0;
			} else {
				alpha[c][n] = p_octree[cell].alpha;

				for (int i = 0; i < 6; i++) {
					//anisotropic read light
					float amount = p_dir.dot(aniso_normal[i]);
					if (amount < 0)
						amount = 0;
					color[c][n].x += p_octree[cell].light[i][0] / 1024.0 * amount;
					color[c][n].y += p_octree[cell].light[i][1] / 1024.0 * amount;
					color[c][n].z += p_octree[cell].light[i][2] / 1024.0 * amount;
				}
			}

			//print_line("\tlev " + itos(c) + " - " + itos(n) + " alpha: " + rtos(cells[test_cell].alpha) + " col: " + color[c][n]);
		}
	}

	float target_level_size = size >> target_level;
	Vector3 pos_fract[2];

	pos_fract[0].x = Math::fmod(pos.x, target_level_size) / target_level_size;
	pos_fract[0].y = Math::fmod(pos.y, target_level_size) / target_level_size;
	pos_fract[0].z = Math::fmod(pos.z, target_level_size) / target_level_size;

	target_level_size = size >> MAX(0, target_level - 1);

	pos_fract[1].x = Math::fmod(pos.x, target_level_size) / target_level_size;
	pos_fract[1].y = Math::fmod(pos.y, target_level_size) / target_level_size;
	pos_fract[1].z = Math::fmod(pos.z, target_level_size) / target_level_size;

	float alpha_interp[2];
	Vector3 color_interp[2];

	for (int i = 0; i < 2; i++) {

		Vector3 color_x00 = color[i][0].linear_interpolate(color[i][1], pos_fract[i].x);
		Vector3 color_xy0 = color[i][2].linear_interpolate(color[i][3], pos_fract[i].x);
		Vector3 blend_z0 = color_x00.linear_interpolate(color_xy0, pos_fract[i].y);

		Vector3 color_x0z = color[i][4].linear_interpolate(color[i][5], pos_fract[i].x);
		Vector3 color_xyz = color[i][6].linear_interpolate(color[i][7], pos_fract[i].x);
		Vector3 blend_z1 = color_x0z.linear_interpolate(color_xyz, pos_fract[i].y);

		color_interp[i] = blend_z0.linear_interpolate(blend_z1, pos_fract[i].z);

		float alpha_x00 = Math::lerp(alpha[i][0], alpha[i][1], pos_fract[i].x);
		float alpha_xy0 = Math::lerp(alpha[i][2], alpha[i][3], pos_fract[i].x);
		float alpha_z0 = Math::lerp(alpha_x00, alpha_xy0, pos_fract[i].y);

		float alpha_x0z = Math::lerp(alpha[i][4], alpha[i][5], pos_fract[i].x);
		float alpha_xyz = Math::lerp(alpha[i][6], alpha[i][7], pos_fract[i].x);
		float alpha_z1 = Math::lerp(alpha_x0z, alpha_xyz, pos_fract[i].y);

		alpha_interp[i] = Math::lerp(alpha_z0, alpha_z1, pos_fract[i].z);
	}

	r_color = color_interp[0].linear_interpolate(color_interp[1], level_filter);
	r_alpha = Math::lerp(alpha_interp[0], alpha_interp[1], level_filter);

	//print_line("pos: " + p_posf + " level " + rtos(p_level) + " down to " + itos(target_level) + "." + rtos(level_filter) + " color " + r_color + " alpha " + rtos(r_alpha));
}

_FORCE_INLINE_ static Color _light_capture_voxel_cone_trace(const RasterizerStorage::LightmapCaptureOctree *p_octree, const Vector3 &p_pos, const Vector3 &p_dir, float p_aperture, int p_cell_subdiv) {

	float bias = 0.0; //no need for bias here
	float max_distance = (Vector3(1, 1, 1) * (1 << (p_cell_subdiv - 1))).length();

	float dist = bias;
	float alpha = 0.0;
	Vector3 color;

	Vector3 scolor;
	float salpha;

	while (dist < max_distance && alpha < 0.95) {
		float diameter = MAX(1.0, 2.0 * p_aperture * dist);
		_light_capture_sample_octree(p_octree, p_cell_subdiv, p_pos + dist * p_dir, p_dir, log2(diameter), scolor, salpha);
		float a = (1.0 - alpha);
		color += scolor * a;
		alpha += a * salpha;
		dist += diameter * 0.5;
	}

	return Color(color.x, color.y, color.z, alpha);
}

void VisualServerScene::_update_instance_lightmap_captures(Instance *p_instance) {
	AUTO_PROFILE;
	InstanceGeometryData *geom = get_instance_geometry(p_instance->self); //static_cast<InstanceGeometryData *>(p_instance->base_data);

	static const Vector3 cone_traces[12] = {
		Vector3(0, 0, 1),
		Vector3(0.866025, 0, 0.5),
		Vector3(0.267617, 0.823639, 0.5),
		Vector3(-0.700629, 0.509037, 0.5),
		Vector3(-0.700629, -0.509037, 0.5),
		Vector3(0.267617, -0.823639, 0.5),
		Vector3(0, 0, -1),
		Vector3(0.866025, 0, -0.5),
		Vector3(0.267617, 0.823639, -0.5),
		Vector3(-0.700629, 0.509037, -0.5),
		Vector3(-0.700629, -0.509037, -0.5),
		Vector3(0.267617, -0.823639, -0.5)
	};

	float cone_aperture = 0.577; // tan(angle) 60 degrees

	if (p_instance->lightmap_capture_data.empty()) {
		p_instance->lightmap_capture_data.resize(12);
	}

	//print_line("update captures for pos: " + p_instance->transform.origin);

	zeromem(p_instance->lightmap_capture_data.ptrw(), 12 * sizeof(Color));
	//this could use some sort of blending..
	TracyPlot("Lightmap Capture updates", (int64_t)geom->lightmap_captures.size());
	for (List<Instance *>::Element *E = geom->lightmap_captures.front(); E; E = E->next()) {
		const PoolVector<RasterizerStorage::LightmapCaptureOctree> *octree = VSG::storage->lightmap_capture_get_octree_ptr(E->get()->base);
		//print_line("octree size: " + itos(octree->size()));
		if (octree->size() == 0)
			continue;
		Transform to_cell_xform = VSG::storage->lightmap_capture_get_octree_cell_transform(E->get()->base);
		int cell_subdiv = VSG::storage->lightmap_capture_get_octree_cell_subdiv(E->get()->base);
		to_cell_xform = to_cell_xform * E->get()->transform.affine_inverse();

		PoolVector<RasterizerStorage::LightmapCaptureOctree>::Read octree_r = octree->read();

		Vector3 pos = to_cell_xform.xform(p_instance->transform.origin);

		for (int i = 0; i < 12; i++) {

			Vector3 dir = to_cell_xform.basis.xform(cone_traces[i]).normalized();
			Color capture = _light_capture_voxel_cone_trace(octree_r.ptr(), pos, dir, cone_aperture, cell_subdiv);
			p_instance->lightmap_capture_data.write[i] += capture;
		}
	}
}

bool VisualServerScene::_light_instance_update_shadow(Instance *p_instance, const Transform p_cam_transform, const CameraMatrix &p_cam_projection, bool p_cam_orthogonal, RID p_shadow_atlas, Scenario *p_scenario) {

	SCOPE_PROFILE(update_shadow);

	LightComponent &light_cmp = get_component<LightComponent>(p_instance->self);
	RID light_rid = light_cmp.light_instance;
	//InstanceLightData *light = get_component<LightComponent>(p_instance->self).Data;

	Transform light_transform = p_instance->transform;
	light_transform.orthonormalize(); //scale does not count on lights

	bool animated_material_found = false;

	int cull_count = 0;

	switch (VSG::storage->light_get_type(p_instance->base)) {

		case VS::LIGHT_DIRECTIONAL: {

			float max_distance = p_cam_projection.get_z_far();
			float shadow_max = VSG::storage->light_get_param(p_instance->base, VS::LIGHT_PARAM_SHADOW_MAX_DISTANCE);
			if (shadow_max > 0 && !p_cam_orthogonal) { //its impractical (and leads to unwanted behaviors) to set max distance in orthogonal camera
				max_distance = MIN(shadow_max, max_distance);
			}
			max_distance = MAX(max_distance, p_cam_projection.get_z_near() + 0.001);
			float min_distance = MIN(p_cam_projection.get_z_near(), max_distance);

			VS::LightDirectionalShadowDepthRangeMode depth_range_mode = VSG::storage->light_directional_get_shadow_depth_range_mode(p_instance->base);

			if (depth_range_mode == VS::LIGHT_DIRECTIONAL_SHADOW_DEPTH_RANGE_OPTIMIZED) {
				//optimize min/max
				Vector<Plane> planes = p_cam_projection.get_projection_planes(p_cam_transform);
				int cull_count = 0; // = p_scenario->octree.cull_convex(planes, instance_shadow_cull_result, MAX_INSTANCE_CULL, VS::INSTANCE_GEOMETRY_MASK);
				Plane base(p_cam_transform.origin, -p_cam_transform.basis.get_axis(2));

				//int cull_count = 0;

				bool found_items = false;
				float z_max = -1e20;
				float z_min = 1e20;
				//std::vector<Instance*> CullResult;
				//CullResult.reserve(1000);
				//p_scenario->octree.cull_convex_lambda(planes, [&](Instance *instance) {
				scenario_cull_convex_geo(p_scenario, planes, [&](EntityID id) {
					const bool bIsVisible = true; //has_component<Visible>(instance->self);
					const bool bIsMesh = true; //has_component<GeometryComponent>(instance->self);
					const bool bIsShadowcaster = bIsMesh && get_component<GeometryComponent>(id).can_cast_shadows;

					if (bIsVisible && bIsMesh && bIsShadowcaster) {

						if (get_component<GeometryComponent>(id).material_is_animated) {
							animated_material_found = true;
						}

						float max, min;
						get_component<InstanceBoundsComponent>(id).transformed_aabb.project_range_in_plane(base, min, max);

						z_max = MAX(z_max, max);

						z_min = MIN(z_min, min);

						found_items = true;
						//instance_shadow_cull_result[cull_count] = instance;
						//CullResult.push_back(instance);
						cull_count++;
					}
				});
				//		VS::INSTANCE_GEOMETRY_MASK);

				if (found_items) {
					min_distance = MAX(min_distance, z_min);
					max_distance = MIN(max_distance, z_max);
				}
			}

			float range = max_distance - min_distance;

			int splits = 0;
			switch (VSG::storage->light_directional_get_shadow_mode(p_instance->base)) {
				case VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL: splits = 1; break;
				case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS: splits = 2; break;
				case VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS: splits = 4; break;
			}

			float distances[5];

			distances[0] = min_distance;
			for (int i = 0; i < splits; i++) {
				distances[i + 1] = min_distance + VSG::storage->light_get_param(p_instance->base, VS::LightParam(VS::LIGHT_PARAM_SHADOW_SPLIT_1_OFFSET + i)) * range;
			};

			distances[splits] = max_distance;

			float texture_size = VSG::scene_render->get_directional_light_shadow_size(light_rid);

			bool overlap = VSG::storage->light_directional_get_blend_splits(p_instance->base);

			float first_radius = 0.0;

			for (int i = 0; i < splits; i++) {

				// setup a camera matrix for that range!
				CameraMatrix camera_matrix;

				float aspect = p_cam_projection.get_aspect();

				if (p_cam_orthogonal) {

					float w, h;
					p_cam_projection.get_viewport_size(w, h);
					camera_matrix.set_orthogonal(w, aspect, distances[(i == 0 || !overlap) ? i : i - 1], distances[i + 1], false);
				} else {

					float fov = p_cam_projection.get_fov();
					camera_matrix.set_perspective(fov, aspect, distances[(i == 0 || !overlap) ? i : i - 1], distances[i + 1], false);
				}

				//obtain the frustum endpoints

				Vector3 endpoints[8]; // frustum plane endpoints
				bool res = camera_matrix.get_endpoints(p_cam_transform, endpoints);
				ERR_CONTINUE(!res);

				// obtain the light frustm ranges (given endpoints)

				Transform transform = light_transform; //discard scale and stabilize light

				Vector3 x_vec = transform.basis.get_axis(Vector3::AXIS_X).normalized();
				Vector3 y_vec = transform.basis.get_axis(Vector3::AXIS_Y).normalized();
				Vector3 z_vec = transform.basis.get_axis(Vector3::AXIS_Z).normalized();
				//z_vec points agsint the camera, like in default opengl

				float x_min = 0.f, x_max = 0.f;
				float y_min = 0.f, y_max = 0.f;
				float z_min = 0.f, z_max = 0.f;

				// FIXME: z_max_cam is defined, computed, but not used below when setting up
				// ortho_camera. Commented out for now to fix warnings but should be investigated.
				float x_min_cam = 0.f, x_max_cam = 0.f;
				float y_min_cam = 0.f, y_max_cam = 0.f;
				float z_min_cam = 0.f;
				//float z_max_cam = 0.f;

				float bias_scale = 1.0;

				//used for culling

				for (int j = 0; j < 8; j++) {

					float d_x = x_vec.dot(endpoints[j]);
					float d_y = y_vec.dot(endpoints[j]);
					float d_z = z_vec.dot(endpoints[j]);

					if (j == 0 || d_x < x_min)
						x_min = d_x;
					if (j == 0 || d_x > x_max)
						x_max = d_x;

					if (j == 0 || d_y < y_min)
						y_min = d_y;
					if (j == 0 || d_y > y_max)
						y_max = d_y;

					if (j == 0 || d_z < z_min)
						z_min = d_z;
					if (j == 0 || d_z > z_max)
						z_max = d_z;
				}

				{
					//camera viewport stuff

					Vector3 center;

					for (int j = 0; j < 8; j++) {

						center += endpoints[j];
					}
					center /= 8.0;

					//center=x_vec*(x_max-x_min)*0.5 + y_vec*(y_max-y_min)*0.5 + z_vec*(z_max-z_min)*0.5;

					float radius = 0;

					for (int j = 0; j < 8; j++) {

						float d = center.distance_to(endpoints[j]);
						if (d > radius)
							radius = d;
					}

					radius *= texture_size / (texture_size - 2.0); //add a texel by each side

					if (i == 0) {
						first_radius = radius;
					} else {
						bias_scale = radius / first_radius;
					}

					x_max_cam = x_vec.dot(center) + radius;
					x_min_cam = x_vec.dot(center) - radius;
					y_max_cam = y_vec.dot(center) + radius;
					y_min_cam = y_vec.dot(center) - radius;
					//z_max_cam = z_vec.dot(center) + radius;
					z_min_cam = z_vec.dot(center) - radius;

					if (depth_range_mode == VS::LIGHT_DIRECTIONAL_SHADOW_DEPTH_RANGE_STABLE) {
						//this trick here is what stabilizes the shadow (make potential jaggies to not move)
						//at the cost of some wasted resolution. Still the quality increase is very well worth it

						float unit = radius * 2.0 / texture_size;

						x_max_cam = Math::stepify(x_max_cam, unit);
						x_min_cam = Math::stepify(x_min_cam, unit);
						y_max_cam = Math::stepify(y_max_cam, unit);
						y_min_cam = Math::stepify(y_min_cam, unit);
					}
				}

				//now that we now all ranges, we can proceed to make the light frustum planes, for culling octree

				Vector<Plane> light_frustum_planes;
				light_frustum_planes.resize(6);

				//right/left
				light_frustum_planes.write[0] = Plane(x_vec, x_max);
				light_frustum_planes.write[1] = Plane(-x_vec, -x_min);
				//top/bottom
				light_frustum_planes.write[2] = Plane(y_vec, y_max);
				light_frustum_planes.write[3] = Plane(-y_vec, -y_min);
				//near/far
				light_frustum_planes.write[4] = Plane(z_vec, z_max + 1e6);
				light_frustum_planes.write[5] = Plane(-z_vec, -z_min); // z_min is ok, since casters further than far-light plane are not needed

				std::vector<RasterizerScene::InstanceBase *> CullResult;
				CullResult.reserve(1000);

				//int cull_count = p_scenario->octree.cull_convex(light_frustum_planes, instance_shadow_cull_result, MAX_INSTANCE_CULL, VS::INSTANCE_GEOMETRY_MASK);
				Plane near_plane(light_transform.origin, -light_transform.basis.get_axis(2));
				//p_scenario->octree.cull_convex_lambda(light_frustum_planes, [&](Instance *instance) {
				scenario_cull_convex_geo(p_scenario, light_frustum_planes, [&](EntityID id) {
					//scenario_cull_convex_instance(p_scenario, light_frustum_planes, [&](Instance *instance) {
					InstanceComponent &instcmp = get_component<InstanceComponent>(id);
					Instance *instance = instcmp.instance;
					const bool bIsVisible = true; //has_component<Visible>(id);
					const bool bIsMesh = true; //
					has_component<GeometryComponent>(id);
					const bool bIsShadowcaster = bIsMesh && get_component<GeometryComponent>(id).can_cast_shadows;

					if (bIsVisible && bIsMesh && bIsShadowcaster) {

						float min, max;
						//Instance *instance = instance_shadow_cull_result[j];

						get_component<InstanceBoundsComponent>(id).transformed_aabb.project_range_in_plane(Plane(z_vec, 0), min, max);
						instance->depth = near_plane.distance_to(instance->transform.origin);
						instance->depth_layer = 0;
						if (max > z_max)
							z_max = max;
						CullResult.push_back(instance);
						//instance_shadow_cull_result[cull_count] = instance;
						cull_count++;
					}
				}); //,
				//		VS::INSTANCE_GEOMETRY_MASK);
				// a pre pass will need to be needed to determine the actual z-near to be used

				ShadowWorkItem WorkItem;
				{

					CameraMatrix ortho_camera;
					real_t half_x = (x_max_cam - x_min_cam) * 0.5;
					real_t half_y = (y_max_cam - y_min_cam) * 0.5;

					ortho_camera.set_orthogonal(-half_x, half_x, -half_y, half_y, 0, (z_max - z_min_cam));

					Transform ortho_transform;
					ortho_transform.basis = transform.basis;
					ortho_transform.origin = x_vec * (x_min_cam + half_x) + y_vec * (y_min_cam + half_y) + z_vec * z_max;

					WorkItem.light_instance_set_shadow_transform(light_rid, ortho_camera, ortho_transform, 0, distances[i + 1], i, bias_scale);
					//VSG::scene_render->light_instance_set_shadow_transform(light->instance, ortho_camera, ortho_transform, 0, distances[i + 1], i, bias_scale);
				}
				WorkItem.render_shadow(light_rid, p_shadow_atlas, i, CullResult);
				ShadowWorkQueue.enqueue(WorkItem);
				//VSG::scene_render->render_shadow(light->instance, p_shadow_atlas, i, (RasterizerScene::InstanceBase **)instance_shadow_cull_result, cull_count);
			}

		} break;
		case VS::LIGHT_OMNI: {

			VS::LightOmniShadowMode shadow_mode = VSG::storage->light_omni_get_shadow_mode(p_instance->base);
			float radius = VSG::storage->light_get_param(p_instance->base, VS::LIGHT_PARAM_RANGE);

			std::vector<AABB> geo_aabbs;
			std::vector<Instance *> geo_instances;
			geo_aabbs.reserve(200);
			geo_instances.reserve(200);

			AABB light_aabb;
			light_aabb.position = (light_transform.get_origin());
			light_aabb.size = Vector3{ radius, radius, radius };

			scenario_cull_box_geo(p_scenario, light_aabb, [&](EntityID id) {
				InstanceComponent &instcmp = get_component<InstanceComponent>(id);
				InstanceBoundsComponent &instbounds = get_component<InstanceBoundsComponent>(id);
				Instance *instance = instcmp.instance;
				const bool bIsVisible = true;
				const bool bIsMesh = true;

				const bool bIsShadowcaster = bIsMesh && get_component<GeometryComponent>(id).can_cast_shadows;

				if (bIsVisible && bIsMesh && bIsShadowcaster) {

					if (get_component<GeometryComponent>(id).material_is_animated) {
						animated_material_found = true;
					}

					geo_instances.push_back(instance);
					geo_aabbs.push_back(instbounds.aabb);
				}
			});

			switch (shadow_mode) {

				case VS::LIGHT_OMNI_SHADOW_DUAL_PARABOLOID: {
					std::vector<RasterizerScene::InstanceBase *> CullResult;
					CullResult.reserve(200);
					for (int i = 0; i < 2; i++) {

						//using this one ensures that raster deferred will have it
						float z = i == 0 ? -1 : 1;
						//Vector<Plane> planes;
						//planes.resize(5);
						std::array<Plane, 5> planes;
						planes[0] = light_transform.xform(Plane(Vector3(0, 0, z), radius));
						planes[1] = light_transform.xform(Plane(Vector3(1, 0, z).normalized(), radius));
						planes[2] = light_transform.xform(Plane(Vector3(-1, 0, z).normalized(), radius));
						planes[3] = light_transform.xform(Plane(Vector3(0, 1, z).normalized(), radius));
						planes[4] = light_transform.xform(Plane(Vector3(0, -1, z).normalized(), radius));

						Plane near_plane(light_transform.origin, light_transform.basis.get_axis(2) * z);

						for (int b = 0; b < geo_aabbs.size(); b++) {
							if (geo_aabbs[b].inside_convex_shape(planes.data(), 5)) {
								CullResult.push_back(geo_instances[b]);
							}
						}

						ShadowWorkItem WorkItem;
						WorkItem.light_instance_set_shadow_transform(light_rid, CameraMatrix(), light_transform, radius, 0, i);
						TracyPlot("Omnishadow cull", (int64_t)CullResult.size());
						WorkItem.render_shadow(light_rid, p_shadow_atlas, i, CullResult);
						ShadowWorkQueue.enqueue(WorkItem);
						CullResult.clear();
					}
				} break;
				case VS::LIGHT_OMNI_SHADOW_CUBE: {

					CameraMatrix cm;
					cm.set_perspective(90, 1, 0.01, radius);
					std::vector<RasterizerScene::InstanceBase *> CullResult;
					CullResult.reserve(200);
					for (int i = 0; i < 6; i++) {

						//using this one ensures that raster deferred will have it

						static const Vector3 view_normals[6] = {
							Vector3(-1, 0, 0),
							Vector3(+1, 0, 0),
							Vector3(0, -1, 0),
							Vector3(0, +1, 0),
							Vector3(0, 0, -1),
							Vector3(0, 0, +1)
						};
						static const Vector3 view_up[6] = {
							Vector3(0, -1, 0),
							Vector3(0, -1, 0),
							Vector3(0, 0, -1),
							Vector3(0, 0, +1),
							Vector3(0, -1, 0),
							Vector3(0, -1, 0)
						};

						Transform xform = light_transform * Transform().looking_at(view_normals[i], view_up[i]);

						Vector<Plane> planes = cm.get_projection_planes(xform);
						Plane near_plane(xform.origin, -xform.basis.get_axis(2));

						for (int b = 0; b < geo_aabbs.size(); b++) {
							if (geo_aabbs[b].inside_convex_shape(&planes[0], 6)) {
								CullResult.push_back(geo_instances[b]);
							}
						}

						TracyPlot("OmniCube cull", (int64_t)CullResult.size());
						ShadowWorkItem WorkItem;
						WorkItem.light_instance_set_shadow_transform(light_rid, cm, xform, radius, 0, i);

						WorkItem.render_shadow(light_rid, p_shadow_atlas, i, CullResult);
						ShadowWorkQueue.enqueue(WorkItem);
						CullResult.clear();
					}

					//restore the regular DP matrix
					VSG::scene_render->light_instance_set_shadow_transform(light_rid, CameraMatrix(), light_transform, radius, 0, 0);
					ShadowWorkItem WorkItem;
					WorkItem.light_instance_set_shadow_transform(light_rid, CameraMatrix(), light_transform, radius, 0, 0);
					//VSG::scene_render->light_instance_set_shadow_transform(light->instance, ortho_camera, ortho_transform, 0, distances[i + 1], i, bias_scale);

					//WorkItem.render_shadow(light->instance, p_shadow_atlas, i, CullResult);
					ShadowWorkQueue.enqueue(WorkItem);

				} break;
			}
		} break;
		case VS::LIGHT_SPOT: {

			float radius = VSG::storage->light_get_param(p_instance->base, VS::LIGHT_PARAM_RANGE);
			float angle = VSG::storage->light_get_param(p_instance->base, VS::LIGHT_PARAM_SPOT_ANGLE);

			CameraMatrix cm;
			cm.set_perspective(angle * 2.0, 1.0, 0.01, radius);

			Vector<Plane> planes = cm.get_projection_planes(light_transform);
			Plane near_plane(light_transform.origin, -light_transform.basis.get_axis(2));
			//int cull_count = 0;
			std::vector<RasterizerScene::InstanceBase *> CullResult;
			CullResult.reserve(1000);
			//p_scenario->octree.cull_convex_lambda(planes, [&](Instance *instance) {
			scenario_cull_convex_geo(p_scenario, planes, [&](EntityID id) {
				InstanceComponent &instcmp = get_component<InstanceComponent>(id);
				Instance *instance = instcmp.instance;
				const bool bIsVisible = true; //has_component<Visible>(instance->self);
				const bool bIsMesh = true;
				const bool bIsShadowcaster = bIsMesh && get_component<GeometryComponent>(id).can_cast_shadows;

				if (bIsVisible && bIsMesh && bIsShadowcaster) {
					if (get_component<GeometryComponent>(id).material_is_animated) {
						animated_material_found = true;
					}
					instance->depth = near_plane.distance_to(instance->transform.origin);
					instance->depth_layer = 0;

					//instance_shadow_cull_result[cull_count]=instance;
					instance_shadow_cull_result[cull_count] = instance;
					CullResult.push_back(instance);
					cull_count++;
				}
			}); //,
			//		VS::INSTANCE_GEOMETRY_MASK);
			TracyPlot("Spotshadow cull", (int64_t)CullResult.size());
			//VSG::scene_render->light_instance_set_shadow_transform(light->instance, cm, light_transform, radius, 0, 0);
			//VSG::scene_render->render_shadow(light->instance, p_shadow_atlas, 0, (RasterizerScene::InstanceBase **)instance_shadow_cull_result, cull_count);
			ShadowWorkItem WorkItem;
			WorkItem.light_instance_set_shadow_transform(light_rid, cm, light_transform, radius, 0, 0);
			//VSG::scene_render->light_instance_set_shadow_transform(light->instance, ortho_camera, ortho_transform, 0, distances[i + 1], i, bias_scale);

			WorkItem.render_shadow(light_rid, p_shadow_atlas, 0, CullResult);
			ShadowWorkQueue.enqueue(WorkItem);
		} break;
	}

	return animated_material_found;
}

void VisualServerScene::render_camera(RID p_camera, RID p_scenario, Size2 p_viewport_size, RID p_shadow_atlas) {
	AUTO_PROFILE;
// render to mono camera
#ifndef _3D_DISABLED

	Camera *camera = &VSG::ecs->registry.get<Camera>(p_camera.eid); //camera_owner.getornull(p_camera);
	ERR_FAIL_COND(!camera);

	/* STEP 1 - SETUP CAMERA */
	CameraMatrix camera_matrix;
	bool ortho = false;

	switch (camera->type) {
		case Camera::ORTHOGONAL: {

			camera_matrix.set_orthogonal(
					camera->size,
					p_viewport_size.width / (float)p_viewport_size.height,
					camera->znear,
					camera->zfar,
					camera->vaspect);
			ortho = true;
		} break;
		case Camera::PERSPECTIVE: {

			camera_matrix.set_perspective(
					camera->fov,
					p_viewport_size.width / (float)p_viewport_size.height,
					camera->znear,
					camera->zfar,
					camera->vaspect);
			ortho = false;

		} break;
	}

	_prepare_scene(camera->transform, camera_matrix, ortho, camera->env, camera->visible_layers, p_scenario, p_shadow_atlas, RID());

	_render_scene(camera->transform, camera_matrix, ortho, camera->env, p_scenario, p_shadow_atlas, RID(), -1);
#endif
}

void VisualServerScene::render_camera(Ref<ARVRInterface> &p_interface, ARVRInterface::Eyes p_eye, RID p_camera, RID p_scenario, Size2 p_viewport_size, RID p_shadow_atlas) {
	// render for AR/VR interface

	Camera *camera = &VSG::ecs->registry.get<Camera>(p_camera.eid); //camera_owner.getornull(p_camera);
	ERR_FAIL_COND(!camera);

	/* SETUP CAMERA, we are ignoring type and FOV here */
	float aspect = p_viewport_size.width / (float)p_viewport_size.height;
	CameraMatrix camera_matrix = p_interface->get_projection_for_eye(p_eye, aspect, camera->znear, camera->zfar);

	// We also ignore our camera position, it will have been positioned with a slightly old tracking position.
	// Instead we take our origin point and have our ar/vr interface add fresh tracking data! Whoohoo!
	Transform world_origin = ARVRServer::get_singleton()->get_world_origin();
	Transform cam_transform = p_interface->get_transform_for_eye(p_eye, world_origin);

	// For stereo render we only prepare for our left eye and then reuse the outcome for our right eye
	if (p_eye == ARVRInterface::EYE_LEFT) {
		///@TODO possibly move responsibility for this into our ARVRServer or ARVRInterface?

		// Center our transform, we assume basis is equal.
		Transform mono_transform = cam_transform;
		Transform right_transform = p_interface->get_transform_for_eye(ARVRInterface::EYE_RIGHT, world_origin);
		mono_transform.origin += right_transform.origin;
		mono_transform.origin *= 0.5;

		// We need to combine our projection frustums for culling.
		// Ideally we should use our clipping planes for this and combine them,
		// however our shadow map logic uses our projection matrix.
		// Note: as our left and right frustums should be mirrored, we don't need our right projection matrix.

		// - get some base values we need
		float eye_dist = (mono_transform.origin - cam_transform.origin).length();
		float z_near = camera_matrix.get_z_near(); // get our near plane
		float z_far = camera_matrix.get_z_far(); // get our far plane
		float width = (2.0 * z_near) / camera_matrix.matrix[0][0];
		float x_shift = width * camera_matrix.matrix[2][0];
		float height = (2.0 * z_near) / camera_matrix.matrix[1][1];
		float y_shift = height * camera_matrix.matrix[2][1];

		// printf("Eye_dist = %f, Near = %f, Far = %f, Width = %f, Shift = %f\n", eye_dist, z_near, z_far, width, x_shift);

		// - calculate our near plane size (horizontal only, right_near is mirrored)
		float left_near = -eye_dist - ((width - x_shift) * 0.5);

		// - calculate our far plane size (horizontal only, right_far is mirrored)
		float left_far = -eye_dist - (z_far * (width - x_shift) * 0.5 / z_near);
		float left_far_right_eye = eye_dist - (z_far * (width + x_shift) * 0.5 / z_near);
		if (left_far > left_far_right_eye) {
			// on displays smaller then double our iod, the right eye far frustrum can overtake the left eyes.
			left_far = left_far_right_eye;
		}

		// - figure out required z-shift
		float slope = (left_far - left_near) / (z_far - z_near);
		float z_shift = (left_near / slope) - z_near;

		// - figure out new vertical near plane size (this will be slightly oversized thanks to our z-shift)
		float top_near = (height - y_shift) * 0.5;
		top_near += (top_near / z_near) * z_shift;
		float bottom_near = -(height + y_shift) * 0.5;
		bottom_near += (bottom_near / z_near) * z_shift;

		// printf("Left_near = %f, Left_far = %f, Top_near = %f, Bottom_near = %f, Z_shift = %f\n", left_near, left_far, top_near, bottom_near, z_shift);

		// - generate our frustum
		CameraMatrix combined_matrix;
		combined_matrix.set_frustum(left_near, -left_near, bottom_near, top_near, z_near + z_shift, z_far + z_shift);

		// and finally move our camera back
		Transform apply_z_shift;
		apply_z_shift.origin = Vector3(0.0, 0.0, z_shift); // z negative is forward so this moves it backwards
		mono_transform *= apply_z_shift;

		// now prepare our scene with our adjusted transform projection matrix
		_prepare_scene(mono_transform, combined_matrix, false, camera->env, camera->visible_layers, p_scenario, p_shadow_atlas, RID());
	} else if (p_eye == ARVRInterface::EYE_MONO) {
		// For mono render, prepare as per usual
		_prepare_scene(cam_transform, camera_matrix, false, camera->env, camera->visible_layers, p_scenario, p_shadow_atlas, RID());
	}

	// And render our scene...
	_render_scene(cam_transform, camera_matrix, false, camera->env, p_scenario, p_shadow_atlas, RID(), -1);
};

struct ShadowUpdateWork {
	void light_instance_update_shadow(VisualServerScene::Instance *p_instance, const Transform p_cam_transform, const CameraMatrix &p_cam_projection,
			bool p_cam_orthogonal, RID p_shadow_atlas, VisualServerScene::Scenario *p_scenario) {

		_p_instance = p_instance;
		_p_cam_transform = p_cam_transform;
		_p_cam_projection = p_cam_projection;
		_p_cam_orthogonal = p_cam_orthogonal;
		_p_shadow_atlas = p_shadow_atlas;
		_p_scenario = p_scenario;
		light = p_instance->self.eid;
		//light = entt::registry<EntityID>::entity_type ;
	}

	VisualServerScene::Instance *_p_instance;
	Transform _p_cam_transform;
	CameraMatrix _p_cam_projection;
	bool _p_cam_orthogonal;
	RID _p_shadow_atlas;
	VisualServerScene::Scenario *_p_scenario;
	EntityID light;
	//VisualServerScene::InstanceLightData * light;
};

struct EntityIDQueueTraits : public moodycamel::ConcurrentQueueDefaultTraits {
	static const size_t BLOCK_SIZE = 256;
};

moodycamel::ConcurrentQueue<EntityID, EntityIDQueueTraits> FrustrumInstances;

moodycamel::ConcurrentQueue<EntityID> reflection_probe_instances;
moodycamel::ConcurrentQueue<EntityID> gi_probe_instances;
moodycamel::ConcurrentQueue<VisualServerScene::Instance *, EntityIDQueueTraits> geometry_instances;
void VisualServerScene::_prepare_scene(const Transform p_cam_transform, const CameraMatrix &p_cam_projection, bool p_cam_orthogonal, RID p_force_environment, uint32_t p_visible_layers, RID p_scenario, RID p_shadow_atlas, RID p_reflection_probe) {

	AUTO_PROFILE;

	// Note, in stereo rendering:
	// - p_cam_transform will be a transform in the middle of our two eyes
	// - p_cam_projection is a wider frustrum that encompasses both eyes

	Scenario *scenario = scenario_owner.getornull(p_scenario);

	render_pass++;
	uint32_t camera_layer_mask = p_visible_layers;

	VSG::scene_render->set_scene_pass(render_pass);

	//rasterizer->set_camera(camera->transform, camera_matrix,ortho);

	Vector<Plane> planes = p_cam_projection.get_projection_planes(p_cam_transform);

	Plane near_plane(p_cam_transform.origin, -p_cam_transform.basis.get_axis(2).normalized());
	float z_far = p_cam_projection.get_z_far();

	/* STEP 2 - CULL */

	light_cull_count = 0;

	reflection_probe_cull_count = 0;
	shadow_redraws = 0;
	//light_samplers_culled=0;

	/*
	print_line("OT: "+rtos( (OS::get_singleton()->get_ticks_usec()-t)/1000.0));
	print_line("OTO: "+itos(p_scenario->octree.get_octant_count()));
	print_line("OTE: "+itos(p_scenario->octree.get_elem_count()));
	print_line("OTP: "+itos(p_scenario->octree.get_pair_count()));
	*/

	/* STEP 3 - PROCESS PORTALS, VALIDATE ROOMS */
	//removed, will replace with culling

	/* STEP 4 - REMOVE FURTHER CULLED OBJECTS, ADD LIGHTS */

	//cull_lights

	{
		SCOPE_PROFILE(OctreeCreation);

		//FastOctree<EntityID> *octree = scenario->octree//new FastOctree<EntityID>(Vector3(8000.0f, 8000.0f, 8000.0f));
		//
		////for loop
		//auto &group = scenario->entity_list.group<CullAABB, InstanceComponent, Visible>();
		//std::for_each(group.begin(), group.end(),
		//		[octree, &group](auto e) {
		//			octree->AddElement(group.get<InstanceComponent>(e).self_ID.eid, group.get<CullAABB>(e).aabb);
		//		});

		//if (scenario->geometry_octree) {
		//	delete scenario->geometry_octree;
		//}
		//scenario->geometry_octree = octree;
	}

	auto &reg = VSG::ecs->registry;

	static std::vector<RID> view_lights;
	static std::vector<RID> view_lights_ids;
	static std::vector<AABB> view_lights_bounds;
	view_lights.clear();
	view_lights_bounds.clear();
	view_lights_ids.clear();
	scenario->entity_list.view<InstanceComponent>().each([&](auto e, InstanceComponent &ins) {
		if (has_component<LightComponent>(ins.self_ID)) {
			InstanceBoundsComponent &bound_comp = get_component<InstanceBoundsComponent>(ins.self_ID);
			LightComponent &light_comp = get_component<LightComponent>(ins.self_ID);

			view_lights.push_back(light_comp.light_instance);
			view_lights_ids.push_back(ins.self_ID);
			//_update_instance_aabb(ins.instance);

			//AABB lightbounds = bound_comp.
			//if (bounds.extra_margin)
			//	new_aabb.grow_by(bounds.extra_margin);
			//
			//bounds.aabb = new_aabb;

			view_lights_bounds.push_back(bound_comp.transformed_aabb);
		}
	});

	auto light_view = reg.group<>(entt::get<InstanceBoundsComponent, LightComponent, Visible, InstanceComponent>, entt::exclude<DirectionalLight>);
	for (auto entity : light_view) {

		InstanceBoundsComponent &bound_comp = light_view.get<InstanceBoundsComponent>(entity);
		LightComponent &light_comp = light_view.get<LightComponent>(entity);
		InstanceComponent &inst_comp = light_view.get<InstanceComponent>(entity);

		RID light_instance = light_comp.light_instance;
		Instance *ins = inst_comp.instance;

		//view_lights.push_back(light_instance);
		//view_lights_bounds.push_back(bound_comp.transformed_aabb);

		bool is_in_frustrum = bound_comp.transformed_aabb.intersects_convex_shape(&planes[0], 6);

		if (light_cull_count < MAX_LIGHTS_CULLED && is_in_frustrum) {

			//if (!light->geometries.empty()) {
			//do not add this light if no geometry is affected by it..
			light_cull_result[light_cull_count] = ins;
			light_instance_cull_result[light_cull_count] = light_instance;
			if (p_shadow_atlas.is_valid() && VSG::storage->light_has_shadow(ins->base)) {
				VSG::scene_render->light_instance_mark_visible(light_instance); //mark it visible for shadow allocation later
			}

			light_cull_count++;
			//}
		}
	}

	//printf("numlight %i", light_cull_count);

	RID *directional_light_ptr = &light_instance_cull_result[light_cull_count];
	directional_light_count = 0;

	std::vector<ShadowUpdateWork> UpdateWork;
	UpdateWork.reserve(10);
	// directional lights
	{

		int directional_shadow_count = 0;
		//reg.group<>(entt::get<InstanceBoundsComponent, LightComponent, Visible, InstanceComponent>);
		auto directional_lights = reg.group<>(entt::get<DirectionalLight, LightComponent, InstanceComponent, Visible>);
		Instance **lights_with_shadow = (Instance **)alloca(sizeof(Instance *) * directional_lights.size());
		for (EntityID lightID : directional_lights) {
			//for (List<Instance *>::Element *E = scenario->directional_lights.front(); E; E = E->next()) {

			InstanceComponent &ic = directional_lights.get<InstanceComponent>(lightID);

			if (light_cull_count + directional_light_count >= MAX_LIGHTS_CULLED) {
				break;
			}
			//we already match visibility

			//if (!ic.instance->visible)
			//	continue;

			//InstanceLightData *light = directional_lights.get<LightComponent>(lightID).Data;

			//check shadow..

			//if (light) {
			if (p_shadow_atlas.is_valid() && VSG::storage->light_has_shadow(ic.self_ID)) {
				lights_with_shadow[directional_shadow_count++] = ic.instance;
			}
			//add to list
			directional_light_ptr[directional_light_count++] = ic.self_ID;
			//}
		}

		VSG::scene_render->set_directional_shadow_count(directional_shadow_count);

		for (int i = 0; i < directional_shadow_count; i++) {
			ShadowUpdateWork work;
			work.light_instance_update_shadow(lights_with_shadow[i], p_cam_transform, p_cam_projection, p_cam_orthogonal, p_shadow_atlas, scenario);
			UpdateWork.push_back(work);
			//_light_instance_update_shadow(lights_with_shadow[i], p_cam_transform, p_cam_projection, p_cam_orthogonal, p_shadow_atlas, scenario);
		}
	}

	ShadowWorkItem QItem;
	int64_t shadowcasters = 0;
	{ //setup shadow maps

		for (int i = 0; i < light_cull_count; i++) {

			Instance *ins = light_cull_result[i];

			if (!p_shadow_atlas.is_valid() || !VSG::storage->light_has_shadow(ins->base))
				continue;
			LightComponent &light_comp = get_component<LightComponent>(ins->self);
			//InstanceLightData *light = light_comp.Data;
			RID light_instance = light_comp.light_instance;
			float coverage = 0.f;

			{ //compute coverage

				Transform cam_xf = p_cam_transform;
				float zn = p_cam_projection.get_z_near();
				Plane p(cam_xf.origin + cam_xf.basis.get_axis(2) * -zn, -cam_xf.basis.get_axis(2)); //camera near plane

				float vp_w, vp_h; //near plane size in screen coordinates
				p_cam_projection.get_viewport_size(vp_w, vp_h);

				switch (VSG::storage->light_get_type(ins->base)) {

					case VS::LIGHT_OMNI: {

						float radius = VSG::storage->light_get_param(ins->base, VS::LIGHT_PARAM_RANGE);

						//get two points parallel to near plane
						Vector3 points[2] = {
							ins->transform.origin,
							ins->transform.origin + cam_xf.basis.get_axis(0) * radius
						};

						if (!p_cam_orthogonal) {
							//if using perspetive, map them to near plane
							for (int j = 0; j < 2; j++) {
								if (p.distance_to(points[j]) < 0) {
									points[j].z = -zn; //small hack to keep size constant when hitting the screen
								}

								p.intersects_segment(cam_xf.origin, points[j], &points[j]); //map to plane
							}
						}

						float screen_diameter = points[0].distance_to(points[1]) * 2;
						coverage = screen_diameter / (vp_w + vp_h);
					} break;
					case VS::LIGHT_SPOT: {

						float radius = VSG::storage->light_get_param(ins->base, VS::LIGHT_PARAM_RANGE);
						float angle = VSG::storage->light_get_param(ins->base, VS::LIGHT_PARAM_SPOT_ANGLE);

						float w = radius * Math::sin(Math::deg2rad(angle));
						float d = radius * Math::cos(Math::deg2rad(angle));

						Vector3 base = ins->transform.origin - ins->transform.basis.get_axis(2).normalized() * d;

						Vector3 points[2] = {
							base,
							base + cam_xf.basis.get_axis(0) * w
						};

						if (!p_cam_orthogonal) {
							//if using perspetive, map them to near plane
							for (int j = 0; j < 2; j++) {
								if (p.distance_to(points[j]) < 0) {
									points[j].z = -zn; //small hack to keep size constant when hitting the screen
								}

								p.intersects_segment(cam_xf.origin, points[j], &points[j]); //map to plane
							}
						}

						float screen_diameter = points[0].distance_to(points[1]) * 2;
						coverage = screen_diameter / (vp_w + vp_h);

					} break;
					default: {
						ERR_PRINT("Invalid Light Type");
					}
				}
			}

			if (light_comp.shadow_dirty) {
				shadow_redraws++;
				light_comp.last_version++;
				light_comp.shadow_dirty = false;
			}

			bool redraw = VSG::scene_render->shadow_atlas_update_light(p_shadow_atlas, light_instance, coverage, light_comp.last_version);

			if (redraw) {

				//shadowcasters++;
				//must redraw!
				ShadowUpdateWork work;
				work.light_instance_update_shadow(ins, p_cam_transform, p_cam_projection, p_cam_orthogonal, p_shadow_atlas, scenario);
				//work.light = light->;
				UpdateWork.push_back(work);

				//_light_instance_update_shadow(ins, p_cam_transform, p_cam_projection, p_cam_orthogonal, p_shadow_atlas, scenario);
			}
		};
	}

	/* STEP 5 - PROCESS LIGHTS */

#ifdef PARALLEL_RENDER
	auto handle = std::async(std::launch::async,
			[&]() {
#endif
				{
					SCOPE_PROFILE(ShadowAsyncWork);

					parallel_for(UpdateWork, [this](auto work) {
						bool bShadowDirty = _light_instance_update_shadow(work._p_instance, work._p_cam_transform, work._p_cam_projection, work._p_cam_orthogonal, work._p_shadow_atlas, work._p_scenario);

						get_component<LightComponent>(work.light).shadow_dirty = false; // bShadowDirty;
					});
				}

#ifdef PARALLEL_RENDER
			});
#endif
	instance_cull_count = 0;
	{
		{ SCOPE_PROFILE(MainFrustrumCull);

	scenario_cull_convex_entities(scenario, planes, [](auto eid) {
		FrustrumInstances.enqueue(eid);
	});
}

{

	JoinAsyncLigthmaps();
	instance_cull_count = 0;
	{
		SCOPE_PROFILE(ProcessInstances);
		parallel_dequeue_concurrent_queue(FrustrumInstances, [&, this](auto eid) {
			InstanceComponent &instcmp = get_component<InstanceComponent>(eid);
			Instance *ins = instcmp.instance; //instance_cull_result[i];

			bool keep = false;

			if ((camera_layer_mask & ins->layer_mask) == 0) {

				//failure
			} else if (has_component<ReflectionProbeComponent>(eid)) {
				ReflectionProbeComponent &reflection_cmp = get_component<ReflectionProbeComponent>(ins->self);
				InstanceReflectionProbeData *reflection_probe = static_cast<InstanceReflectionProbeData *>(ins->base_data);

				if (p_reflection_probe != reflection_cmp.instance) {
					//avoid entering The Matrix

					if (!reflection_probe->geometries.empty()) {
						//do not add this light if no geometry is affected by it..

						//reflection_probe_instances.enqueue(reflection_probe);
						reflection_probe_instances.enqueue(ins->self.eid);
					}
				}

			} else if (has_component<GIProbeComponent>(eid)) {

				//InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(ins->base_data);

				if (!has_component<MarkUpdate<GIProbeComponent> >(ins->self)) {
					gi_probe_instances.enqueue(ins->self.eid);
				}

				//if (!gi_probe->update_element.in_list()) {
				//	gi_probe_instances.enqueue(gi_probe);
				//	//gi_probe_update_list.add(&gi_probe->update_element);
				//}

			} else if (has_component<GeometryComponent>(eid) && ins->cast_shadows != VS::SHADOW_CASTING_SETTING_SHADOWS_ONLY) {
				GeometryComponent &geocomp = get_component<GeometryComponent>(eid);
				InstanceBoundsComponent &bounds = get_component<InstanceBoundsComponent>(eid);
				//get_component<GeometryComponent>(ins->self).

				keep = true;

				InstanceGeometryData *geom = get_instance_geometry(ins->self);

				if (ins->redraw_if_visible) {
					//VisualServerRaster::redraw_request();
				}

				if (ins->base_type == VS::INSTANCE_PARTICLES) {
					//particles visible? process them
					if (VSG::storage->particles_is_inactive(ins->base)) {
						//but if nothing is going on, don't do it.
						keep = false;
					} else {
						//VSG::storage->particles_request_process(ins->base);
						//particles visible? request redraw
						//VisualServerRaster::redraw_request();
					}
				}

				if (geocomp.lighting_dirty) {
					SCOPE_PROFILE(RefreshLight)
					int l = 0;

					struct LightInfo {
						Vector3 pos;
						RID light;
					};
					std::array<LightInfo, 36> candidate_lights;
					geocomp.AffectingLights.clear();

					for (int i = 0; i < view_lights_bounds.size(); i++) {

						if (bounds.transformed_aabb.intersects(view_lights_bounds[i])) {

							candidate_lights[l].pos = view_lights_bounds[i].position;
							candidate_lights[l].light = view_lights[i];
							geocomp.AffectingLights.push_back(view_lights_ids[i].eid);
							l++;
							//ins->lights[l++] =
						}
					}

					Vector3 targetpos = bounds.transformed_aabb.position;
					if (l > 16) {
						std::sort(&candidate_lights[0], &candidate_lights[16], [&targetpos](const LightInfo &A, const LightInfo &B) {
							const float distA = targetpos.distance_squared_to(A.pos);
							const float distB = targetpos.distance_squared_to(B.pos);
							return distA < distB;
						});

						l = 16;
					}

					for (int i = 0; i < 16 && i < l; i++) {
						ins->lights[i] = candidate_lights[i].light;
					}

					ins->nlights = l;

					geocomp.lighting_dirty = false;
				}

				if (geocomp.reflection_dirty) {
					SCOPE_PROFILE(RefreshReflection)
					int l = 0;
					//only called when reflection probe AABB enter/exit this geometry
					ins->reflection_probe_instances.resize(geom->reflection_probes.size());

					for (List<Instance *>::Element *E = geom->reflection_probes.front(); E; E = E->next()) {

						ReflectionProbeComponent &reflection_cmp = get_component<ReflectionProbeComponent>(E->get()->self);
						//InstanceReflectionProbeData *reflection_probe = static_cast<InstanceReflectionProbeData *>(E->get()->base_data);

						ins->reflection_probe_instances.write[l++] = reflection_cmp.instance;
					}

					geocomp.reflection_dirty = false;
				}

				if (geocomp.gi_probes_dirty) {
					SCOPE_PROFILE(RefreshGIProbes)
					int l = 0;
					//only called when reflection probe AABB enter/exit this geometry
					ins->gi_probe_instances.resize(geom->gi_probes.size());

					for (List<Instance *>::Element *E = geom->gi_probes.front(); E; E = E->next()) {

						InstanceGIProbeData *gi_probe = static_cast<InstanceGIProbeData *>(E->get()->base_data);

						ins->gi_probe_instances.write[l++] = gi_probe->probe_instance;
					}

					geocomp.gi_probes_dirty = false;
				}

				ins->depth = near_plane.distance_to(ins->transform.origin);
				ins->depth_layer = CLAMP(int(ins->depth * 16 / z_far), 0, 15);
			}
			if (keep) {
				geometry_instances.enqueue(ins);
			}
		});
	}

	{

		SCOPE_PROFILE(dequeue_gi_probes);
		dequeue_concurrent_queue(gi_probe_instances, [this](EntityID gi_probe) {
			add_component<MarkUpdate<GIProbeComponent> >(gi_probe);
			//gi_probe_update_list.add(&gi_probe->update_element);
		});
	}
	{

		SCOPE_PROFILE(dequeue_reflection_probes);
		dequeue_concurrent_queue(reflection_probe_instances, [this](EntityID id) {
			ReflectionProbeComponent &reflection_cmp = get_component<ReflectionProbeComponent>(id);
			InstanceReflectionProbeData *reflection_probe = reflection_cmp.Data;

			if (reflection_probe_cull_count < MAX_REFLECTION_PROBES_CULLED) {
				if (reflection_cmp.reflection_dirty || VSG::scene_render->reflection_probe_instance_needs_redraw(reflection_cmp.instance)) {
					add_component<MarkUpdate<ReflectionProbeComponent> >(id);

					reflection_cmp.reflection_dirty = false;
				}

				if (VSG::scene_render->reflection_probe_instance_has_reflection(reflection_cmp.instance)) {
					reflection_probe_instance_cull_result[reflection_probe_cull_count] = reflection_cmp.instance;
					reflection_probe_cull_count++;
				}
			}
		});
	}
	{

		SCOPE_PROFILE(dequeue_geometry);
		dequeue_concurrent_queue(geometry_instances, [this](auto *ins) {
			if (ins->base_type == VS::INSTANCE_PARTICLES) {
				VSG::storage->particles_request_process(ins->base);
			}

			instance_cull_result[instance_cull_count] = ins;
			instance_cull_count++;
			ins->last_render_pass = render_pass;
		});
	}
}
}
{
	SCOPE_PROFILE(dequeue_shadows)

	int shadowmeshes = 0;
	while (ShadowWorkQueue.try_dequeue(QItem)) {
		SCOPE_PROFILE(ShadowRender_2)
		if (QItem.bUpdateTransform) {
			SCOPE_PROFILE(ShadowTransform)
			VSG::scene_render->light_instance_set_shadow_transform(QItem.tf_p_light_instance, QItem.tf_p_projection, QItem.tf_p_transform, QItem.tf_p_far, QItem.tf_p_split, QItem.tf_p_pass, QItem.tf_p_bias_scale);
		}
		if (QItem.bRender) {
			SCOPE_PROFILE(ShadowPass)
			shadowcasters++;
			shadowmeshes += QItem.r_cullresult.size();
			VSG::scene_render->render_shadow(QItem.r_p_light, QItem.r_p_shadow_atlas, QItem.r_p_pass, &QItem.r_cullresult[0], QItem.r_cullresult.size());
		}
	}
#ifdef PARALLEL_RENDER

	handle.get();
	//its possible that there is still work left
	while (ShadowWorkQueue.try_dequeue(QItem)) {
		SCOPE_PROFILE(ShadowRender_2)
		if (QItem.bUpdateTransform) {
			SCOPE_PROFILE(ShadowTransform)
			VSG::scene_render->light_instance_set_shadow_transform(QItem.tf_p_light_instance, QItem.tf_p_projection, QItem.tf_p_transform, QItem.tf_p_far, QItem.tf_p_split, QItem.tf_p_pass, QItem.tf_p_bias_scale);
		}
		if (QItem.bRender) {
			SCOPE_PROFILE(ShadowPass)
			shadowcasters++;
			shadowmeshes += QItem.r_cullresult.size();
			VSG::scene_render->render_shadow(QItem.r_p_light, QItem.r_p_shadow_atlas, QItem.r_p_pass, &QItem.r_cullresult[0], QItem.r_cullresult.size());
		}
	}
#endif

	TracyPlot("Shadow Casters", (int64_t)shadowcasters);
	TracyPlot("Shadow Instances", (int64_t)shadowmeshes);
	TracyPlot("Render Instances", (int64_t)instance_cull_count);
	TracyPlot("Shadow Redraws", (int64_t)shadow_redraws);
}

UpdateWork.clear();
}

void VisualServerScene::_render_scene(const Transform p_cam_transform, const CameraMatrix &p_cam_projection, bool p_cam_orthogonal, RID p_force_environment, RID p_scenario, RID p_shadow_atlas, RID p_reflection_probe, int p_reflection_probe_pass) {
	AUTO_PROFILE;
	Scenario *scenario = scenario_owner.getornull(p_scenario);

	/* ENVIRONMENT */

	RID environment;
	if (p_force_environment.is_valid()) //camera has more environment priority
		environment = p_force_environment;
	else if (scenario->environment.is_valid())
		environment = scenario->environment;
	else
		environment = scenario->fallback_environment;

	/* PROCESS GEOMETRY AND DRAW SCENE */

	VSG::scene_render->render_scene(p_cam_transform, p_cam_projection, p_cam_orthogonal, (RasterizerScene::InstanceBase **)instance_cull_result, instance_cull_count, light_instance_cull_result, light_cull_count + directional_light_count, reflection_probe_instance_cull_result, reflection_probe_cull_count, environment, p_shadow_atlas, scenario->reflection_atlas, p_reflection_probe, p_reflection_probe_pass);
}

void VisualServerScene::render_empty_scene(RID p_scenario, RID p_shadow_atlas) {

#ifndef _3D_DISABLED

	Scenario *scenario = scenario_owner.getornull(p_scenario);

	RID environment;
	if (scenario->environment.is_valid())
		environment = scenario->environment;
	else
		environment = scenario->fallback_environment;
	VSG::scene_render->render_scene(Transform(), CameraMatrix(), true, NULL, 0, NULL, 0, NULL, 0, environment, p_shadow_atlas, scenario->reflection_atlas, RID(), 0);
#endif
}

bool VisualServerScene::_render_reflection_probe_step(Instance *p_instance, int p_step) {

	ReflectionProbeComponent &reflection_cmp = get_component<ReflectionProbeComponent>(p_instance->self);
	InstanceReflectionProbeData *reflection_probe = reflection_cmp.Data; //static_cast<InstanceReflectionProbeData *>(p_instance->base_data);
	Scenario *scenario = p_instance->scenario;
	ERR_FAIL_COND_V(!scenario, true);

	VisualServerRaster::redraw_request(); //update, so it updates in editor

	if (p_step == 0) {

		if (!VSG::scene_render->reflection_probe_instance_begin_render(reflection_cmp.instance, scenario->reflection_atlas)) {
			return true; //sorry, all full :(
		}
	}

	if (p_step >= 0 && p_step < 6) {

		static const Vector3 view_normals[6] = {
			Vector3(-1, 0, 0),
			Vector3(+1, 0, 0),
			Vector3(0, -1, 0),
			Vector3(0, +1, 0),
			Vector3(0, 0, -1),
			Vector3(0, 0, +1)
		};

		Vector3 extents = VSG::storage->reflection_probe_get_extents(p_instance->base);
		Vector3 origin_offset = VSG::storage->reflection_probe_get_origin_offset(p_instance->base);
		float max_distance = VSG::storage->reflection_probe_get_origin_max_distance(p_instance->base);

		Vector3 edge = view_normals[p_step] * extents;
		float distance = ABS(view_normals[p_step].dot(edge) - view_normals[p_step].dot(origin_offset)); //distance from origin offset to actual view distance limit

		max_distance = MAX(max_distance, distance);

		//render cubemap side
		CameraMatrix cm;
		cm.set_perspective(90, 1, 0.01, max_distance);

		static const Vector3 view_up[6] = {
			Vector3(0, -1, 0),
			Vector3(0, -1, 0),
			Vector3(0, 0, -1),
			Vector3(0, 0, +1),
			Vector3(0, -1, 0),
			Vector3(0, -1, 0)
		};

		Transform local_view;
		local_view.set_look_at(origin_offset, origin_offset + view_normals[p_step], view_up[p_step]);

		Transform xform = p_instance->transform * local_view;

		RID shadow_atlas;

		if (VSG::storage->reflection_probe_renders_shadows(p_instance->base)) {

			shadow_atlas = scenario->reflection_probe_shadow_atlas;
		}

		_prepare_scene(xform, cm, false, RID(), VSG::storage->reflection_probe_get_cull_mask(p_instance->base), p_instance->scenario->self, shadow_atlas, reflection_cmp.instance);
		_render_scene(xform, cm, false, RID(), p_instance->scenario->self, shadow_atlas, reflection_cmp.instance, p_step);

	} else {
		//do roughness postprocess step until it believes it's done
		return VSG::scene_render->reflection_probe_instance_postprocess_step(reflection_cmp.instance);
	}

	return false;
}

void VisualServerScene::_gi_probe_fill_local_data(int p_idx, int p_level, int p_x, int p_y, int p_z, const GIProbeDataCell *p_cell, const GIProbeDataHeader *p_header, InstanceGIProbeData::LocalData *p_local_data, Vector<uint32_t> *prev_cell) {

	if ((uint32_t)p_level == p_header->cell_subdiv - 1) {

		Vector3 emission;
		emission.x = (p_cell[p_idx].emission >> 24) / 255.0;
		emission.y = ((p_cell[p_idx].emission >> 16) & 0xFF) / 255.0;
		emission.z = ((p_cell[p_idx].emission >> 8) & 0xFF) / 255.0;
		float l = (p_cell[p_idx].emission & 0xFF) / 255.0;
		l *= 8.0;

		emission *= l;

		p_local_data[p_idx].energy[0] = uint16_t(emission.x * 1024); //go from 0 to 1024 for light
		p_local_data[p_idx].energy[1] = uint16_t(emission.y * 1024); //go from 0 to 1024 for light
		p_local_data[p_idx].energy[2] = uint16_t(emission.z * 1024); //go from 0 to 1024 for light
	} else {

		p_local_data[p_idx].energy[0] = 0;
		p_local_data[p_idx].energy[1] = 0;
		p_local_data[p_idx].energy[2] = 0;

		int half = (1 << (p_header->cell_subdiv - 1)) >> (p_level + 1);

		for (int i = 0; i < 8; i++) {

			uint32_t child = p_cell[p_idx].children[i];

			if (child == 0xFFFFFFFF)
				continue;

			int x = p_x;
			int y = p_y;
			int z = p_z;

			if (i & 1)
				x += half;
			if (i & 2)
				y += half;
			if (i & 4)
				z += half;

			_gi_probe_fill_local_data(child, p_level + 1, x, y, z, p_cell, p_header, p_local_data, prev_cell);
		}
	}

	//position for each part of the mipmaped texture
	p_local_data[p_idx].pos[0] = p_x >> (p_header->cell_subdiv - p_level - 1);
	p_local_data[p_idx].pos[1] = p_y >> (p_header->cell_subdiv - p_level - 1);
	p_local_data[p_idx].pos[2] = p_z >> (p_header->cell_subdiv - p_level - 1);

	prev_cell[p_level].push_back(p_idx);
}

void VisualServerScene::_gi_probe_bake_threads(void *self) {

	VisualServerScene *vss = (VisualServerScene *)self;
	vss->_gi_probe_bake_thread();
}

void VisualServerScene::_setup_gi_probe(Instance *p_instance) {

	InstanceGIProbeData *probe = static_cast<InstanceGIProbeData *>(p_instance->base_data);

	if (probe->dynamic.probe_data.is_valid()) {
		VSG::storage->free(probe->dynamic.probe_data);
		probe->dynamic.probe_data = RID();
	}

	probe->dynamic.light_data = VSG::storage->gi_probe_get_dynamic_data(p_instance->base);

	if (probe->dynamic.light_data.size() == 0)
		return;
	//using dynamic data
	PoolVector<int>::Read r = probe->dynamic.light_data.read();

	const GIProbeDataHeader *header = (GIProbeDataHeader *)r.ptr();

	probe->dynamic.local_data.resize(header->cell_count);

	int cell_count = probe->dynamic.local_data.size();
	PoolVector<InstanceGIProbeData::LocalData>::Write ldw = probe->dynamic.local_data.write();
	const GIProbeDataCell *cells = (GIProbeDataCell *)&r[16];

	probe->dynamic.level_cell_lists.resize(header->cell_subdiv);

	_gi_probe_fill_local_data(0, 0, 0, 0, 0, cells, header, ldw.ptr(), probe->dynamic.level_cell_lists.ptrw());

	bool compress = VSG::storage->gi_probe_is_compressed(p_instance->base);

	probe->dynamic.compression = compress ? VSG::storage->gi_probe_get_dynamic_data_get_preferred_compression() : RasterizerStorage::GI_PROBE_UNCOMPRESSED;

	probe->dynamic.probe_data = VSG::storage->gi_probe_dynamic_data_create(header->width, header->height, header->depth, probe->dynamic.compression);

	probe->dynamic.bake_dynamic_range = VSG::storage->gi_probe_get_dynamic_range(p_instance->base);

	probe->dynamic.mipmaps_3d.clear();
	probe->dynamic.propagate = VSG::storage->gi_probe_get_propagation(p_instance->base);

	probe->dynamic.grid_size[0] = header->width;
	probe->dynamic.grid_size[1] = header->height;
	probe->dynamic.grid_size[2] = header->depth;

	int size_limit = 1;
	int size_divisor = 1;

	if (probe->dynamic.compression == RasterizerStorage::GI_PROBE_S3TC) {
		size_limit = 4;
		size_divisor = 4;
	}
	for (int i = 0; i < (int)header->cell_subdiv; i++) {

		int x = header->width >> i;
		int y = header->height >> i;
		int z = header->depth >> i;

		//create and clear mipmap
		PoolVector<uint8_t> mipmap;
		int size = x * y * z * 4;
		size /= size_divisor;
		mipmap.resize(size);
		PoolVector<uint8_t>::Write w = mipmap.write();
		zeromem(w.ptr(), size);
		w = PoolVector<uint8_t>::Write();

		probe->dynamic.mipmaps_3d.push_back(mipmap);

		if (x <= size_limit || y <= size_limit || z <= size_limit)
			break;
	}

	probe->dynamic.updating_stage = GI_UPDATE_STAGE_CHECK;
	probe->invalid = false;
	probe->dynamic.enabled = true;

	Transform cell_to_xform = VSG::storage->gi_probe_get_to_cell_xform(p_instance->base);
	AABB bounds = VSG::storage->gi_probe_get_bounds(p_instance->base);
	float cell_size = VSG::storage->gi_probe_get_cell_size(p_instance->base);

	probe->dynamic.light_to_cell_xform = cell_to_xform * p_instance->transform.affine_inverse();

	VSG::scene_render->gi_probe_instance_set_light_data(probe->probe_instance, p_instance->base, probe->dynamic.probe_data);
	VSG::scene_render->gi_probe_instance_set_transform_to_data(probe->probe_instance, probe->dynamic.light_to_cell_xform);

	VSG::scene_render->gi_probe_instance_set_bounds(probe->probe_instance, bounds.size / cell_size);

	probe->base_version = VSG::storage->gi_probe_get_version(p_instance->base);

	//if compression is S3TC, fill it up
	if (probe->dynamic.compression == RasterizerStorage::GI_PROBE_S3TC) {

		//create all blocks
		Vector<Map<uint32_t, InstanceGIProbeData::CompBlockS3TC> > comp_blocks;
		int mipmap_count = probe->dynamic.mipmaps_3d.size();
		comp_blocks.resize(mipmap_count);

		for (int i = 0; i < cell_count; i++) {

			const GIProbeDataCell &c = cells[i];
			const InstanceGIProbeData::LocalData &ld = ldw[i];
			int level = c.level_alpha >> 16;
			int mipmap = header->cell_subdiv - level - 1;
			if (mipmap >= mipmap_count)
				continue; //uninteresting

			int blockx = (ld.pos[0] >> 2);
			int blocky = (ld.pos[1] >> 2);
			int blockz = (ld.pos[2]); //compression is x/y only

			int blockw = (header->width >> mipmap) >> 2;
			int blockh = (header->height >> mipmap) >> 2;

			//print_line("cell "+itos(i)+" level "+itos(level)+"mipmap: "+itos(mipmap)+" pos: "+Vector3(blockx,blocky,blockz)+" size "+Vector2(blockw,blockh));

			uint32_t key = blockz * blockw * blockh + blocky * blockw + blockx;

			Map<uint32_t, InstanceGIProbeData::CompBlockS3TC> &cmap = comp_blocks.write[mipmap];

			if (!cmap.has(key)) {

				InstanceGIProbeData::CompBlockS3TC k;
				k.offset = key; //use offset as counter first
				k.source_count = 0;
				cmap[key] = k;
			}

			InstanceGIProbeData::CompBlockS3TC &k = cmap[key];
			ERR_CONTINUE(k.source_count == 16);
			k.sources[k.source_count++] = i;
		}

		//fix the blocks, precomputing what is needed
		probe->dynamic.mipmaps_s3tc.resize(mipmap_count);

		for (int i = 0; i < mipmap_count; i++) {
			//print_line("S3TC level: " + itos(i) + " blocks: " + itos(comp_blocks[i].size()));
			probe->dynamic.mipmaps_s3tc.write[i].resize(comp_blocks[i].size());
			PoolVector<InstanceGIProbeData::CompBlockS3TC>::Write w = probe->dynamic.mipmaps_s3tc.write[i].write();
			int block_idx = 0;

			for (Map<uint32_t, InstanceGIProbeData::CompBlockS3TC>::Element *E = comp_blocks[i].front(); E; E = E->next()) {

				InstanceGIProbeData::CompBlockS3TC k = E->get();

				//PRECOMPUTE ALPHA
				int max_alpha = -100000;
				int min_alpha = k.source_count == 16 ? 100000 : 0; //if the block is not completely full, minimum is always 0, (and those blocks will map to 1, which will be zero)

				uint8_t alpha_block[4][4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

				for (uint32_t j = 0; j < k.source_count; j++) {

					int alpha = (cells[k.sources[j]].level_alpha >> 8) & 0xFF;
					if (alpha < min_alpha)
						min_alpha = alpha;
					if (alpha > max_alpha)
						max_alpha = alpha;
					//fill up alpha block
					alpha_block[ldw[k.sources[j]].pos[0] % 4][ldw[k.sources[j]].pos[1] % 4] = alpha;
				}

				//use the first mode (8 adjustable levels)
				k.alpha[0] = max_alpha;
				k.alpha[1] = min_alpha;

				uint64_t alpha_bits = 0;

				if (max_alpha != min_alpha) {

					int idx = 0;

					for (int y = 0; y < 4; y++) {
						for (int x = 0; x < 4; x++) {

							//subtract minimum
							uint32_t a = uint32_t(alpha_block[x][y]) - min_alpha;
							//convert range to 3 bits
							a = int((a * 7.0 / (max_alpha - min_alpha)) + 0.5);
							a = CLAMP(a, 0, 7); //just to be sure
							a = 7 - a; //because range is inverted in this mode
							if (a == 0) {
								//do none, remain
							} else if (a == 7) {
								a = 1;
							} else {
								a = a + 1;
							}

							alpha_bits |= uint64_t(a) << (idx * 3);
							idx++;
						}
					}
				}

				k.alpha[2] = (alpha_bits >> 0) & 0xFF;
				k.alpha[3] = (alpha_bits >> 8) & 0xFF;
				k.alpha[4] = (alpha_bits >> 16) & 0xFF;
				k.alpha[5] = (alpha_bits >> 24) & 0xFF;
				k.alpha[6] = (alpha_bits >> 32) & 0xFF;
				k.alpha[7] = (alpha_bits >> 40) & 0xFF;

				w[block_idx++] = k;
			}
		}
	}
}

void VisualServerScene::_gi_probe_bake_thread() {

	while (true) {

		probe_bake_sem->wait();
		if (probe_bake_thread_exit) {
			break;
		}

		Instance *to_bake = NULL;

		probe_bake_mutex->lock();

		if (!probe_bake_list.empty()) {
			to_bake = probe_bake_list.front()->get();
			probe_bake_list.pop_front();
		}
		probe_bake_mutex->unlock();

		if (!to_bake)
			continue;

		_bake_gi_probe(to_bake);
	}
}

uint32_t VisualServerScene::_gi_bake_find_cell(const GIProbeDataCell *cells, int x, int y, int z, int p_cell_subdiv) {

	uint32_t cell = 0;

	int ofs_x = 0;
	int ofs_y = 0;
	int ofs_z = 0;
	int size = 1 << (p_cell_subdiv - 1);
	int half = size / 2;

	if (x < 0 || x >= size)
		return -1;
	if (y < 0 || y >= size)
		return -1;
	if (z < 0 || z >= size)
		return -1;

	for (int i = 0; i < p_cell_subdiv - 1; i++) {

		const GIProbeDataCell *bc = &cells[cell];

		int child = 0;
		if (x >= ofs_x + half) {
			child |= 1;
			ofs_x += half;
		}
		if (y >= ofs_y + half) {
			child |= 2;
			ofs_y += half;
		}
		if (z >= ofs_z + half) {
			child |= 4;
			ofs_z += half;
		}

		cell = bc->children[child];
		if (cell == 0xFFFFFFFF)
			return 0xFFFFFFFF;

		half >>= 1;
	}

	return cell;
}

static float _get_normal_advance(const Vector3 &p_normal) {

	Vector3 normal = p_normal;
	Vector3 unorm = normal.abs();

	if ((unorm.x >= unorm.y) && (unorm.x >= unorm.z)) {
		// x code
		unorm = normal.x > 0.0 ? Vector3(1.0, 0.0, 0.0) : Vector3(-1.0, 0.0, 0.0);
	} else if ((unorm.y > unorm.x) && (unorm.y >= unorm.z)) {
		// y code
		unorm = normal.y > 0.0 ? Vector3(0.0, 1.0, 0.0) : Vector3(0.0, -1.0, 0.0);
	} else if ((unorm.z > unorm.x) && (unorm.z > unorm.y)) {
		// z code
		unorm = normal.z > 0.0 ? Vector3(0.0, 0.0, 1.0) : Vector3(0.0, 0.0, -1.0);
	} else {
		// oh-no we messed up code
		// has to be
		unorm = Vector3(1.0, 0.0, 0.0);
	}

	return 1.0 / normal.dot(unorm);
}

void VisualServerScene::_bake_gi_probe_light(const GIProbeDataHeader *header, const GIProbeDataCell *cells, InstanceGIProbeData::LocalData *local_data, const uint32_t *leaves, int p_leaf_count, const InstanceGIProbeData::LightCache &light_cache, int p_sign) {

	int light_r = int(light_cache.color.r * light_cache.energy * 1024.0) * p_sign;
	int light_g = int(light_cache.color.g * light_cache.energy * 1024.0) * p_sign;
	int light_b = int(light_cache.color.b * light_cache.energy * 1024.0) * p_sign;

	float limits[3] = { float(header->width), float(header->height), float(header->depth) };
	Plane clip[3];
	int clip_planes = 0;

	switch (light_cache.type) {

		case VS::LIGHT_DIRECTIONAL: {

			float max_len = Vector3(limits[0], limits[1], limits[2]).length() * 1.1;

			Vector3 light_axis = -light_cache.transform.basis.get_axis(2).normalized();

			for (int i = 0; i < 3; i++) {

				if (ABS(light_axis[i]) < CMP_EPSILON)
					continue;
				clip[clip_planes].normal[i] = 1.0;

				if (light_axis[i] < 0) {

					clip[clip_planes].d = limits[i] + 1;
				} else {
					clip[clip_planes].d -= 1.0;
				}

				clip_planes++;
			}

			float distance_adv = _get_normal_advance(light_axis);

			int success_count = 0;

			// uint64_t us = OS::get_singleton()->get_ticks_usec();

			for (int i = 0; i < p_leaf_count; i++) {

				uint32_t idx = leaves[i];

				const GIProbeDataCell *cell = &cells[idx];
				InstanceGIProbeData::LocalData *light = &local_data[idx];

				Vector3 to(light->pos[0] + 0.5, light->pos[1] + 0.5, light->pos[2] + 0.5);
				to += -light_axis.sign() * 0.47; //make it more likely to receive a ray

				Vector3 norm(
						(((cells[idx].normal >> 16) & 0xFF) / 255.0) * 2.0 - 1.0,
						(((cells[idx].normal >> 8) & 0xFF) / 255.0) * 2.0 - 1.0,
						(((cells[idx].normal >> 0) & 0xFF) / 255.0) * 2.0 - 1.0);

				float att = norm.dot(-light_axis);
				if (att < 0.001) {
					//not lighting towards this
					continue;
				}

				Vector3 from = to - max_len * light_axis;

				for (int j = 0; j < clip_planes; j++) {

					clip[j].intersects_segment(from, to, &from);
				}

				float distance = (to - from).length();
				distance += distance_adv - Math::fmod(distance, distance_adv); //make it reach the center of the box always
				from = to - light_axis * distance;

				uint32_t result = 0xFFFFFFFF;

				while (distance > -distance_adv) { //use this to avoid precision errors

					result = _gi_bake_find_cell(cells, int(floor(from.x)), int(floor(from.y)), int(floor(from.z)), header->cell_subdiv);
					if (result != 0xFFFFFFFF) {
						break;
					}

					from += light_axis * distance_adv;
					distance -= distance_adv;
				}

				if (result == idx) {
					//cell hit itself! hooray!
					light->energy[0] += int32_t(light_r * att * ((cell->albedo >> 16) & 0xFF) / 255.0);
					light->energy[1] += int32_t(light_g * att * ((cell->albedo >> 8) & 0xFF) / 255.0);
					light->energy[2] += int32_t(light_b * att * ((cell->albedo) & 0xFF) / 255.0);
					success_count++;
				}
			}

			// print_line("BAKE TIME: " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
			// print_line("valid cells: " + itos(success_count));

		} break;
		case VS::LIGHT_OMNI:
		case VS::LIGHT_SPOT: {

			// uint64_t us = OS::get_singleton()->get_ticks_usec();

			Vector3 light_pos = light_cache.transform.origin;
			Vector3 spot_axis = -light_cache.transform.basis.get_axis(2).normalized();

			float local_radius = light_cache.radius * light_cache.transform.basis.get_axis(2).length();

			for (int i = 0; i < p_leaf_count; i++) {

				uint32_t idx = leaves[i];

				const GIProbeDataCell *cell = &cells[idx];
				InstanceGIProbeData::LocalData *light = &local_data[idx];

				Vector3 to(light->pos[0] + 0.5, light->pos[1] + 0.5, light->pos[2] + 0.5);
				to += (light_pos - to).sign() * 0.47; //make it more likely to receive a ray

				Vector3 norm(
						(((cells[idx].normal >> 16) & 0xFF) / 255.0) * 2.0 - 1.0,
						(((cells[idx].normal >> 8) & 0xFF) / 255.0) * 2.0 - 1.0,
						(((cells[idx].normal >> 0) & 0xFF) / 255.0) * 2.0 - 1.0);

				Vector3 light_axis = (to - light_pos).normalized();
				float distance_adv = _get_normal_advance(light_axis);

				float att = norm.dot(-light_axis);
				if (att < 0.001) {
					//not lighting towards this
					continue;
				}

				{
					float d = light_pos.distance_to(to);
					if (d + distance_adv > local_radius)
						continue; // too far away

					float dt = CLAMP((d + distance_adv) / local_radius, 0, 1);
					att *= powf(1.0 - dt, light_cache.attenuation);
				}

				if (light_cache.type == VS::LIGHT_SPOT) {

					float angle = Math::rad2deg(acos(light_axis.dot(spot_axis)));
					if (angle > light_cache.spot_angle)
						continue;

					float d = CLAMP(angle / light_cache.spot_angle, 0, 1);
					att *= powf(1.0 - d, light_cache.spot_attenuation);
				}

				clip_planes = 0;

				for (int c = 0; c < 3; c++) {

					if (ABS(light_axis[c]) < CMP_EPSILON)
						continue;
					clip[clip_planes].normal[c] = 1.0;

					if (light_axis[c] < 0) {

						clip[clip_planes].d = limits[c] + 1;
					} else {
						clip[clip_planes].d -= 1.0;
					}

					clip_planes++;
				}

				Vector3 from = light_pos;

				for (int j = 0; j < clip_planes; j++) {

					clip[j].intersects_segment(from, to, &from);
				}

				float distance = (to - from).length();

				distance -= Math::fmod(distance, distance_adv); //make it reach the center of the box always, but this tame make it closer
				from = to - light_axis * distance;

				uint32_t result = 0xFFFFFFFF;

				while (distance > -distance_adv) { //use this to avoid precision errors

					result = _gi_bake_find_cell(cells, int(floor(from.x)), int(floor(from.y)), int(floor(from.z)), header->cell_subdiv);
					if (result != 0xFFFFFFFF) {
						break;
					}

					from += light_axis * distance_adv;
					distance -= distance_adv;
				}

				if (result == idx) {
					//cell hit itself! hooray!

					light->energy[0] += int32_t(light_r * att * ((cell->albedo >> 16) & 0xFF) / 255.0);
					light->energy[1] += int32_t(light_g * att * ((cell->albedo >> 8) & 0xFF) / 255.0);
					light->energy[2] += int32_t(light_b * att * ((cell->albedo) & 0xFF) / 255.0);
				}
			}
			//print_line("BAKE TIME: " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
		} break;
	}
}

void VisualServerScene::_bake_gi_downscale_light(int p_idx, int p_level, const GIProbeDataCell *p_cells, const GIProbeDataHeader *p_header, InstanceGIProbeData::LocalData *p_local_data, float p_propagate) {

	//average light to upper level

	float divisor = 0;
	float sum[3] = { 0.0, 0.0, 0.0 };

	for (int i = 0; i < 8; i++) {

		uint32_t child = p_cells[p_idx].children[i];

		if (child == 0xFFFFFFFF)
			continue;

		if (p_level + 1 < (int)p_header->cell_subdiv - 1) {
			_bake_gi_downscale_light(child, p_level + 1, p_cells, p_header, p_local_data, p_propagate);
		}

		sum[0] += p_local_data[child].energy[0];
		sum[1] += p_local_data[child].energy[1];
		sum[2] += p_local_data[child].energy[2];
		divisor += 1.0;
	}

	divisor = Math::lerp((float)8.0, divisor, p_propagate);
	sum[0] /= divisor;
	sum[1] /= divisor;
	sum[2] /= divisor;

	//divide by eight for average
	p_local_data[p_idx].energy[0] = Math::fast_ftoi(sum[0]);
	p_local_data[p_idx].energy[1] = Math::fast_ftoi(sum[1]);
	p_local_data[p_idx].energy[2] = Math::fast_ftoi(sum[2]);
}

void VisualServerScene::_bake_gi_probe(Instance *p_gi_probe) {

	InstanceGIProbeData *probe_data = static_cast<InstanceGIProbeData *>(p_gi_probe->base_data);

	PoolVector<int>::Read r = probe_data->dynamic.light_data.read();

	const GIProbeDataHeader *header = (const GIProbeDataHeader *)r.ptr();
	const GIProbeDataCell *cells = (const GIProbeDataCell *)&r[16];

	int leaf_count = probe_data->dynamic.level_cell_lists[header->cell_subdiv - 1].size();
	const uint32_t *leaves = probe_data->dynamic.level_cell_lists[header->cell_subdiv - 1].ptr();

	PoolVector<InstanceGIProbeData::LocalData>::Write ldw = probe_data->dynamic.local_data.write();

	InstanceGIProbeData::LocalData *local_data = ldw.ptr();

	//remove what must be removed
	for (Map<RID, InstanceGIProbeData::LightCache>::Element *E = probe_data->dynamic.light_cache.front(); E; E = E->next()) {

		RID rid = E->key();
		const InstanceGIProbeData::LightCache &lc = E->get();

		if ((!probe_data->dynamic.light_cache_changes.has(rid) || probe_data->dynamic.light_cache_changes[rid] != lc) && lc.visible) {
			//erase light data

			_bake_gi_probe_light(header, cells, local_data, leaves, leaf_count, lc, -1);
		}
	}

	//add what must be added
	for (Map<RID, InstanceGIProbeData::LightCache>::Element *E = probe_data->dynamic.light_cache_changes.front(); E; E = E->next()) {

		RID rid = E->key();
		const InstanceGIProbeData::LightCache &lc = E->get();

		if ((!probe_data->dynamic.light_cache.has(rid) || probe_data->dynamic.light_cache[rid] != lc) && lc.visible) {
			//add light data

			_bake_gi_probe_light(header, cells, local_data, leaves, leaf_count, lc, 1);
		}
	}

	SWAP(probe_data->dynamic.light_cache_changes, probe_data->dynamic.light_cache);

	//downscale to lower res levels
	_bake_gi_downscale_light(0, 0, cells, header, local_data, probe_data->dynamic.propagate);

	//plot result to 3D texture!

	if (probe_data->dynamic.compression == RasterizerStorage::GI_PROBE_UNCOMPRESSED) {

		for (int i = 0; i < (int)header->cell_subdiv; i++) {

			int stage = header->cell_subdiv - i - 1;

			if (stage >= probe_data->dynamic.mipmaps_3d.size())
				continue; //no mipmap for this one

			//print_line("generating mipmap stage: " + itos(stage));
			int level_cell_count = probe_data->dynamic.level_cell_lists[i].size();
			const uint32_t *level_cells = probe_data->dynamic.level_cell_lists[i].ptr();

			PoolVector<uint8_t>::Write lw = probe_data->dynamic.mipmaps_3d.write[stage].write();
			uint8_t *mipmapw = lw.ptr();

			uint32_t sizes[3] = { header->width >> stage, header->height >> stage, header->depth >> stage };

			for (int j = 0; j < level_cell_count; j++) {

				uint32_t idx = level_cells[j];

				uint32_t r = (uint32_t(local_data[idx].energy[0]) / probe_data->dynamic.bake_dynamic_range) >> 2;
				uint32_t g = (uint32_t(local_data[idx].energy[1]) / probe_data->dynamic.bake_dynamic_range) >> 2;
				uint32_t b = (uint32_t(local_data[idx].energy[2]) / probe_data->dynamic.bake_dynamic_range) >> 2;
				uint32_t a = (cells[idx].level_alpha >> 8) & 0xFF;

				uint32_t mm_ofs = sizes[0] * sizes[1] * (local_data[idx].pos[2]) + sizes[0] * (local_data[idx].pos[1]) + (local_data[idx].pos[0]);
				mm_ofs *= 4; //for RGBA (4 bytes)

				mipmapw[mm_ofs + 0] = uint8_t(CLAMP(r, 0, 255));
				mipmapw[mm_ofs + 1] = uint8_t(CLAMP(g, 0, 255));
				mipmapw[mm_ofs + 2] = uint8_t(CLAMP(b, 0, 255));
				mipmapw[mm_ofs + 3] = uint8_t(CLAMP(a, 0, 255));
			}
		}
	} else if (probe_data->dynamic.compression == RasterizerStorage::GI_PROBE_S3TC) {

		int mipmap_count = probe_data->dynamic.mipmaps_3d.size();

		for (int mmi = 0; mmi < mipmap_count; mmi++) {

			PoolVector<uint8_t>::Write mmw = probe_data->dynamic.mipmaps_3d.write[mmi].write();
			int block_count = probe_data->dynamic.mipmaps_s3tc[mmi].size();
			PoolVector<InstanceGIProbeData::CompBlockS3TC>::Read mmr = probe_data->dynamic.mipmaps_s3tc[mmi].read();

			for (int i = 0; i < block_count; i++) {

				const InstanceGIProbeData::CompBlockS3TC &b = mmr[i];

				uint8_t *blockptr = &mmw[b.offset * 16];
				copymem(blockptr, b.alpha, 8); //copy alpha part, which is precomputed

				Vector3 colors[16];

				for (uint32_t j = 0; j < b.source_count; j++) {

					colors[j].x = (local_data[b.sources[j]].energy[0] / float(probe_data->dynamic.bake_dynamic_range)) / 1024.0;
					colors[j].y = (local_data[b.sources[j]].energy[1] / float(probe_data->dynamic.bake_dynamic_range)) / 1024.0;
					colors[j].z = (local_data[b.sources[j]].energy[2] / float(probe_data->dynamic.bake_dynamic_range)) / 1024.0;
				}
				//super quick and dirty compression
				//find 2 most further apart
				float distance = 0;
				Vector3 from, to;

				if (b.source_count == 16) {
					//all cells are used so, find minmax between them
					int further_apart[2] = { 0, 0 };
					for (uint32_t j = 0; j < b.source_count; j++) {
						for (uint32_t k = j + 1; k < b.source_count; k++) {
							float d = colors[j].distance_squared_to(colors[k]);
							if (d > distance) {
								distance = d;
								further_apart[0] = j;
								further_apart[1] = k;
							}
						}
					}

					from = colors[further_apart[0]];
					to = colors[further_apart[1]];

				} else {
					//if a block is missing, the priority is that this block remains black,
					//otherwise the geometry will appear deformed
					//correct shape wins over correct color in this case
					//average all colors first
					Vector3 average;

					for (uint32_t j = 0; j < b.source_count; j++) {
						average += colors[j];
					}
					average.normalize();
					//find max distance in normal from average
					for (uint32_t j = 0; j < b.source_count; j++) {
						float d = average.dot(colors[j]);
						distance = MAX(d, distance);
					}

					from = Vector3(); //from black
					to = average * distance;
					//find max distance
				}

				int indices[16];
				uint16_t color_0 = 0;
				color_0 = CLAMP(int(from.x * 31), 0, 31) << 11;
				color_0 |= CLAMP(int(from.y * 63), 0, 63) << 5;
				color_0 |= CLAMP(int(from.z * 31), 0, 31);

				uint16_t color_1 = 0;
				color_1 = CLAMP(int(to.x * 31), 0, 31) << 11;
				color_1 |= CLAMP(int(to.y * 63), 0, 63) << 5;
				color_1 |= CLAMP(int(to.z * 31), 0, 31);

				if (color_1 > color_0) {
					SWAP(color_1, color_0);
					SWAP(from, to);
				}

				if (distance > 0) {

					Vector3 dir = (to - from).normalized();

					for (uint32_t j = 0; j < b.source_count; j++) {

						float d = (colors[j] - from).dot(dir) / distance;
						indices[j] = int(d * 3 + 0.5);

						static const int index_swap[4] = { 0, 3, 1, 2 };

						indices[j] = index_swap[CLAMP(indices[j], 0, 3)];
					}
				} else {
					for (uint32_t j = 0; j < b.source_count; j++) {
						indices[j] = 0;
					}
				}

				//by default, 1 is black, otherwise it will be overridden by source

				uint32_t index_block[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

				for (uint32_t j = 0; j < b.source_count; j++) {

					int x = local_data[b.sources[j]].pos[0] % 4;
					int y = local_data[b.sources[j]].pos[1] % 4;

					index_block[y * 4 + x] = indices[j];
				}

				uint32_t encode = 0;

				for (int j = 0; j < 16; j++) {
					encode |= index_block[j] << (j * 2);
				}

				blockptr[8] = color_0 & 0xFF;
				blockptr[9] = (color_0 >> 8) & 0xFF;
				blockptr[10] = color_1 & 0xFF;
				blockptr[11] = (color_1 >> 8) & 0xFF;
				blockptr[12] = encode & 0xFF;
				blockptr[13] = (encode >> 8) & 0xFF;
				blockptr[14] = (encode >> 16) & 0xFF;
				blockptr[15] = (encode >> 24) & 0xFF;
			}
		}
	}

	//send back to main thread to update un little chunks
	if (probe_bake_mutex) {
		probe_bake_mutex->lock();
	}

	probe_data->dynamic.updating_stage = GI_UPDATE_STAGE_UPLOADING;

	if (probe_bake_mutex) {
		probe_bake_mutex->unlock();
	}
}

bool VisualServerScene::_check_gi_probe(Instance *p_gi_probe) {

	InstanceGIProbeData *probe_data = static_cast<InstanceGIProbeData *>(p_gi_probe->base_data);

	probe_data->dynamic.light_cache_changes.clear();

	bool all_equal = true;

	auto &reg = VSG::ecs->registry;
	auto light_view = reg.group<>(entt::get<InstanceBoundsComponent, LightComponent, InstanceComponent>);

	for (auto et : light_view) { //List<Instance *>::Element *E = p_gi_probe->scenario->directional_lights.front(); E; E = E->next()) {
		InstanceComponent &inst_comp = light_view.get<InstanceComponent>(et);
		InstanceGIProbeData::LightCache lc;
		auto base = inst_comp.instance->base;
		lc.type = VSG::storage->light_get_type(base);
		lc.color = VSG::storage->light_get_color(base);
		lc.energy = VSG::storage->light_get_param(base, VS::LIGHT_PARAM_ENERGY) * VSG::storage->light_get_param(base, VS::LIGHT_PARAM_INDIRECT_ENERGY);
		lc.radius = VSG::storage->light_get_param(base, VS::LIGHT_PARAM_RANGE);
		lc.attenuation = VSG::storage->light_get_param(base, VS::LIGHT_PARAM_ATTENUATION);
		lc.spot_angle = VSG::storage->light_get_param(base, VS::LIGHT_PARAM_SPOT_ANGLE);
		lc.spot_attenuation = VSG::storage->light_get_param(base, VS::LIGHT_PARAM_SPOT_ATTENUATION);
		lc.transform = probe_data->dynamic.light_to_cell_xform * inst_comp.instance->transform;
		lc.visible = has_component<Visible>(et);

		if (!probe_data->dynamic.light_cache.has(inst_comp.instance->self) || probe_data->dynamic.light_cache[inst_comp.instance->self] != lc) {
			all_equal = false;
		}

		probe_data->dynamic.light_cache_changes[inst_comp.instance->self] = lc;
	}

	for (Set<Instance *>::Element *E = probe_data->lights.front(); E; E = E->next()) {

		InstanceGIProbeData::LightCache lc;
		lc.type = VSG::storage->light_get_type(E->get()->base);
		lc.color = VSG::storage->light_get_color(E->get()->base);
		lc.energy = VSG::storage->light_get_param(E->get()->base, VS::LIGHT_PARAM_ENERGY) * VSG::storage->light_get_param(E->get()->base, VS::LIGHT_PARAM_INDIRECT_ENERGY);
		lc.radius = VSG::storage->light_get_param(E->get()->base, VS::LIGHT_PARAM_RANGE);
		lc.attenuation = VSG::storage->light_get_param(E->get()->base, VS::LIGHT_PARAM_ATTENUATION);
		lc.spot_angle = VSG::storage->light_get_param(E->get()->base, VS::LIGHT_PARAM_SPOT_ANGLE);
		lc.spot_attenuation = VSG::storage->light_get_param(E->get()->base, VS::LIGHT_PARAM_SPOT_ATTENUATION);
		lc.transform = probe_data->dynamic.light_to_cell_xform * E->get()->transform;
		lc.visible = has_component<Visible>(E->get()->self);

		if (!probe_data->dynamic.light_cache.has(E->get()->self) || probe_data->dynamic.light_cache[E->get()->self] != lc) {
			all_equal = false;
		}

		probe_data->dynamic.light_cache_changes[E->get()->self] = lc;
	}

	//lighting changed from after to before, must do some updating
	return !all_equal || probe_data->dynamic.light_cache_changes.size() != probe_data->dynamic.light_cache.size();
}

void VisualServerScene::render_probes() {
	AUTO_PROFILE;
	/* REFLECTION PROBES */

	bool busy = false;

	auto ref_probes = VSG::ecs->registry.view<MarkUpdate<ReflectionProbeComponent> >();
	for (auto e : ref_probes) {
		AUTO_PROFILE;
		RID base = get_component<InstanceComponent>(e).instance->base;

		ReflectionProbeComponent &reflection_cmp = get_component<ReflectionProbeComponent>(e);
		switch (VSG::storage->reflection_probe_get_update_mode(base)) {

			case VS::REFLECTION_PROBE_UPDATE_ONCE: {
				if (busy) //already rendering something
					break;

				bool done = _render_reflection_probe_step(reflection_cmp.owner, reflection_cmp.render_step);
				if (done) {
					clear_component<MarkUpdate<ReflectionProbeComponent> >(e);

					//reflection_probe_render_list.remove(ref_probe);
				} else {
					reflection_cmp.render_step++;
				}

				busy = true; //do not render another one of this kind
			} break;
			case VS::REFLECTION_PROBE_UPDATE_ALWAYS: {

				int step = 0;
				bool done = false;
				while (!done) {
					done = _render_reflection_probe_step(reflection_cmp.owner, step);
					step++;
				}
				clear_component<MarkUpdate<ReflectionProbeComponent> >(e);

			} break;
		}
	}

	auto ref_giprobes = VSG::ecs->registry.view<MarkUpdate<GIProbeComponent> >();
	for (auto e : ref_giprobes) {

		RID base = get_component<InstanceComponent>(e).instance->base;

		InstanceGIProbeData *probe = get_component<GIProbeComponent>(e).Data;
		Instance *instance_probe = probe->owner;

		//check if probe must be setup, but don't do if on the lighting thread

		bool force_lighting = false;

		if (probe->invalid || (probe->dynamic.updating_stage == GI_UPDATE_STAGE_CHECK && probe->base_version != VSG::storage->gi_probe_get_version(instance_probe->base))) {

			_setup_gi_probe(instance_probe);
			force_lighting = true;
		}

		float propagate = VSG::storage->gi_probe_get_propagation(instance_probe->base);

		if (probe->dynamic.propagate != propagate) {
			probe->dynamic.propagate = propagate;
			force_lighting = true;
		}

		if (!probe->invalid && probe->dynamic.enabled) {

			switch (probe->dynamic.updating_stage) {
				case GI_UPDATE_STAGE_CHECK: {

					if (_check_gi_probe(instance_probe) || force_lighting) { //send to lighting thread

#ifndef NO_THREADS
						probe_bake_mutex->lock();
						probe->dynamic.updating_stage = GI_UPDATE_STAGE_LIGHTING;
						probe_bake_list.push_back(instance_probe);
						probe_bake_mutex->unlock();
						probe_bake_sem->post();

#else

						_bake_gi_probe(instance_probe);
#endif
					}
				} break;
				case GI_UPDATE_STAGE_LIGHTING: {
					//do none, wait til done!

				} break;
				case GI_UPDATE_STAGE_UPLOADING: {

					//uint64_t us = OS::get_singleton()->get_ticks_usec();

					for (int i = 0; i < (int)probe->dynamic.mipmaps_3d.size(); i++) {

						PoolVector<uint8_t>::Read r = probe->dynamic.mipmaps_3d[i].read();
						VSG::storage->gi_probe_dynamic_data_update(probe->dynamic.probe_data, 0, probe->dynamic.grid_size[2] >> i, i, r.ptr());
					}

					probe->dynamic.updating_stage = GI_UPDATE_STAGE_CHECK;

					//print_line("UPLOAD TIME: " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
				} break;
			}
		}
		//_update_gi_probe(gi_probe->self()->owner);

		//gi_probe = next;
	}
}
std::mutex mat_mutex;
void VisualServerScene::_update_instance_material(Instance *p_instance) {
	if (p_instance->base_type == VS::INSTANCE_MESH) {
		// remove materials no longer used and un-own them

		int new_mat_count = VSG::storage->mesh_get_surface_count(p_instance->base);
		for (int i = p_instance->materials.size() - 1; i >= new_mat_count; i--) {
			if (p_instance->materials[i].is_valid()) {
				std::lock_guard<std::mutex> lock(mat_mutex);
				VSG::storage->material_remove_instance_owner(p_instance->materials[i], p_instance);
			}
		}
		p_instance->materials.resize(new_mat_count);

		int new_blend_shape_count = VSG::storage->mesh_get_blend_shape_count(p_instance->base);
		if (new_blend_shape_count != p_instance->blend_values.size()) {
			p_instance->blend_values.resize(new_blend_shape_count);
			for (int i = 0; i < new_blend_shape_count; i++) {
				p_instance->blend_values.write[i] = 0;
			}
		}
	}

	if (has_component<GeometryComponent>(p_instance->self)) {
		InstanceGeometryData *geom = get_instance_geometry(p_instance->self);
		GeometryComponent &gcomp = get_component<GeometryComponent>(p_instance->self);

		bool can_cast_shadows = true;
		bool is_animated = false;

		if (p_instance->cast_shadows == VS::SHADOW_CASTING_SETTING_OFF) {
			can_cast_shadows = false;
		} else if (p_instance->material_override.is_valid()) {
			can_cast_shadows = VSG::storage->material_casts_shadows(p_instance->material_override);
			is_animated = VSG::storage->material_is_animated(p_instance->material_override);
		} else {
			if (p_instance->base_type == VS::INSTANCE_MESH) {
				RID mesh = p_instance->base;

				if (mesh.is_valid()) {
					bool cast_shadows = false;

					for (int i = 0; i < p_instance->materials.size(); i++) {
						RID mat = p_instance->materials[i].is_valid() ? p_instance->materials[i] : VSG::storage->mesh_surface_get_material(mesh, i);

						if (!mat.is_valid()) {
							cast_shadows = true;
						} else {
							if (VSG::storage->material_casts_shadows(mat)) {
								cast_shadows = true;
							}

							if (VSG::storage->material_is_animated(mat)) {
								is_animated = true;
							}
						}
					}

					if (!cast_shadows) {
						can_cast_shadows = false;
					}
				}
			} else if (p_instance->base_type == VS::INSTANCE_MULTIMESH) {
				RID mesh = VSG::storage->multimesh_get_mesh(p_instance->base);
				if (mesh.is_valid()) {
					bool cast_shadows = false;

					int sc = VSG::storage->mesh_get_surface_count(mesh);
					for (int i = 0; i < sc; i++) {
						RID mat = VSG::storage->mesh_surface_get_material(mesh, i);

						if (!mat.is_valid()) {
							cast_shadows = true;
						} else {
							if (VSG::storage->material_casts_shadows(mat)) {
								cast_shadows = true;
							}
							if (VSG::storage->material_is_animated(mat)) {
								is_animated = true;
							}
						}
					}

					if (!cast_shadows) {
						can_cast_shadows = false;
					}
				}
			} else if (p_instance->base_type == VS::INSTANCE_IMMEDIATE) {
				RID mat = VSG::storage->immediate_get_material(p_instance->base);

				if (!mat.is_valid() || VSG::storage->material_casts_shadows(mat)) {
					can_cast_shadows = true;
				} else {
					can_cast_shadows = false;
				}

				if (mat.is_valid() && VSG::storage->material_is_animated(mat)) {
					is_animated = true;
				}
			} else if (p_instance->base_type == VS::INSTANCE_PARTICLES) {
				bool cast_shadows = false;

				int dp = VSG::storage->particles_get_draw_passes(p_instance->base);

				for (int i = 0; i < dp; i++) {
					RID mesh = VSG::storage->particles_get_draw_pass_mesh(p_instance->base, i);
					if (!mesh.is_valid())
						continue;

					int sc = VSG::storage->mesh_get_surface_count(mesh);
					for (int j = 0; j < sc; j++) {
						RID mat = VSG::storage->mesh_surface_get_material(mesh, j);

						if (!mat.is_valid()) {
							cast_shadows = true;
						} else {
							if (VSG::storage->material_casts_shadows(mat)) {
								cast_shadows = true;
							}

							if (VSG::storage->material_is_animated(mat)) {
								is_animated = true;
							}
						}
					}
				}

				if (!cast_shadows) {
					can_cast_shadows = false;
				}
			}
		}

		if (can_cast_shadows != gcomp.can_cast_shadows) {
			// ability to cast shadows change, let lights now
			for (auto e : gcomp.AffectingLights) {
				if (has_component<LightComponent>(e)) {

					get_component<LightComponent>(e).shadow_dirty = true;
				}
			}
			gcomp.can_cast_shadows = can_cast_shadows;
		}

		gcomp.material_is_animated = is_animated;
	}
}
_FORCE_INLINE_ void VisualServerScene::_update_dirty_instance(Instance *p_instance) {
	const Dirty &dt = get_component<Dirty>(p_instance->self);

	if (dt.update_aabb) {
		_update_instance_aabb(p_instance);
	}

	if (dt.update_materials) {

		_update_instance_material(p_instance);
	}

	_update_instance(p_instance);
	clear_component<Dirty>(p_instance->self);
}

void VisualServerScene::update_dirty_instances() {

	SCOPE_PROFILE(update_dirty_instances);

	{
		SCOPE_PROFILE(update_resources);
		VSG::storage->update_dirty_resources();
	}

	auto view = VSG::ecs->registry.group<>(entt::get<InstanceComponent, Dirty>);

	{
		SCOPE_PROFILE(update_aabbs);

		//for (auto entity : view) {
		parallel_for(view, [&](auto entity) {
			Instance *p_instance = view.get<InstanceComponent>(entity).instance;
			const Dirty &dt = view.get<Dirty>(entity);

			if (dt.update_aabb) {
				_update_instance_aabb(p_instance);
			}
		});
	}
	{
		SCOPE_PROFILE(update_materials);
		//for (auto entity : view) {
		parallel_for(view, [&](auto entity) {
			Instance *p_instance = view.get<InstanceComponent>(entity).instance;
			const Dirty &dt = view.get<Dirty>(entity);

			if (dt.update_materials) {
				_update_instance_material(p_instance);
			}
		});
	}

	//{
	//	SCOPE_PROFILE(update_materials);
	//	for (auto entity : view) {
	//		Instance *p_instance = view.get<InstanceComponent>(entity).instance;
	//		const Dirty & dt = view.get<Dirty>(entity);
	//
	//		if (dt.update_materials) {
	//			_update_instance_material(p_instance);
	//		}
	//	}
	//}
	//
	{
		SCOPE_PROFILE(update_inst);
		for (auto entity : view) {
			Instance *p_instance = view.get<InstanceComponent>(entity).instance;
			const Dirty &dt = view.get<Dirty>(entity);

			_update_instance(p_instance);

			//VSG::ecs->registry.remove<Dirty>(entity);
		}
	}
	{
		bAsyncLightmapsCalculating = true;
		//async_lightmap_captures = std::async(std::launch::async, [this]() {
			SCOPE_PROFILE(lightmap_captures);
			parallel_dequeue_concurrent_queue_unbatched(lightmap_update_queue, [this](Instance *inst) {
				_update_instance_lightmap_captures(inst);
			});
		//});
		VSG::ecs->registry.reset<Dirty>();
	}
}

bool VisualServerScene::free(RID p_rid) {

	if (scenario_owner.owns(p_rid)) {

		Scenario *scenario = scenario_owner.get(p_rid);

		while (scenario->instances.first()) {
			instance_set_scenario(scenario->instances.first()->self()->self, RID());
		}
		VSG::scene_render->free(scenario->reflection_probe_shadow_atlas);
		VSG::scene_render->free(scenario->reflection_atlas);
		scenario_owner.free(p_rid);
		memdelete(scenario);

	} else if (instance_owner.owns(p_rid)) {
		// delete the instance

		update_dirty_instances();
		
		Instance *instance = instance_owner.get(p_rid);

		instance_set_use_lightmap(p_rid, RID(), RID());
		instance_set_scenario(p_rid, RID());
		instance_set_base(p_rid, RID());
		instance_geometry_set_material_override(p_rid, RID());
		instance_attach_skeleton(p_rid, RID());

		update_dirty_instances(); //in case something changed this
		JoinAsyncLigthmaps();
		if (VSG::ecs->registry.valid(p_rid.eid)) {
			VSG::ecs->registry.destroy(p_rid.eid);
		}

		instance_owner.free(p_rid);
		memdelete(instance);
	} else {
		return false;
	}

	if (VSG::ecs->registry.valid(p_rid.eid)) {
		VSG::ecs->registry.destroy(p_rid.eid);
	}
	return true;
}

VisualServerScene *VisualServerScene::singleton = NULL;

VisualServerScene::VisualServerScene() {

#ifndef NO_THREADS
	probe_bake_sem = Semaphore::create();
	probe_bake_mutex = Mutex::create();
	probe_bake_thread = Thread::create(_gi_probe_bake_threads, this);
	probe_bake_thread_exit = false;
#endif
	PROFILER_INIT();
	render_pass = 1;
	singleton = this;
}

VisualServerScene::~VisualServerScene() {

#ifndef NO_THREADS
	probe_bake_thread_exit = true;
	probe_bake_sem->post();
	Thread::wait_to_finish(probe_bake_thread);
	memdelete(probe_bake_thread);
	memdelete(probe_bake_sem);
	memdelete(probe_bake_mutex);

#endif
}
