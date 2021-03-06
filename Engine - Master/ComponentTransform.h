#ifndef __COMPOMENTTRANSFORM_H__
#define __COMPOMENTTRANSFORM_H__

#include "Component.h"
#include "MathGeoLib.h"

class ComponentTransform : public Component
{ 
public:
	ComponentTransform(GameObject* go);
	ComponentTransform(const ComponentTransform& comp);
	~ComponentTransform();

	Component* Duplicate() override;
	void SetTransform(const math::float4x4& transform);
	void SetTransform(const math::float3& pos, const math::Quat& rot, const math::float3& sca);
	void SetGlobalTransform(const math::float4x4& global_transform);
	void SetRotation(const math::Quat& rot);
	void SetRotation(const math::float3& rot);

	void LocalToGlobal(const math::float4x4& local_transform);
	void GlobalToLocal(const math::float4x4& global_transform);

	bool DrawOnInspector() override;

	JSON_value* Save(JSON_value* component) const override;
	void Load(JSON_value* component) override;

public:
	math::float3 position = math::float3().zero;
	math::float3 euler_rotation = math::float3().zero;
	math::Quat rotation = math::Quat().identity;
	math::float3 scale = math::float3().one;
};

#endif /*__COMPOMENTTRANSFORM_H__*/