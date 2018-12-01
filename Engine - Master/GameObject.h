#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "MathGeoLib.h"
#include <string>
#include <list>

#define GO_NAME_SIZE 24
#define GO_DEFAULT_NAME "GameObject"

class Component;
class ComponentTransform;
class ComponentMesh;
class ComponentMaterial;

enum class component_type;

#pragma region GOFlags
enum GOFlags
{
	None = 0,
	Delete = 1,
	Copy = 2,
	Duplicate = 3
};

inline GOFlags operator|(GOFlags a, GOFlags b)
{
	return static_cast<GOFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline GOFlags operator&(GOFlags a, GOFlags b)
{
	return static_cast<GOFlags>(static_cast<int>(a) & static_cast<int>(b));
}

inline GOFlags operator ~(GOFlags a)
{
	return static_cast<GOFlags>(~static_cast<int>(a));
}

inline GOFlags& operator |=(GOFlags &a, GOFlags b)
{
	a = static_cast<GOFlags> (static_cast<int>(a) | static_cast<int>(b));
	return a;
}
#pragma endregion

class GameObject
{
public:
	GameObject(const char* name);
	GameObject(const char* name, GameObject* parent);
	GameObject(const char* name, const math::float4x4& new_transform);
	GameObject(const char* name, const math::float4x4& new_transform, GameObject* parent);
	GameObject::GameObject(const GameObject& go);
	~GameObject();

	void Update();
	void CleanUp();

	// Game Object
	void Draw();
	void DeleteGameObject();
	math::float4x4 GetLocalTransform() const;
	math::float4x4 GetGlobalTransform() const;

	// Components
	Component* CreateComponent(component_type type);
	std::vector<Component*> GetComponents(component_type type) const;
	void DeleteComponent(Component* component);

	// Children
	void Unchild();

public:

	bool active = true;
	std::string name = "GameObject";
	GOFlags flags = GOFlags::None;
	
	// Hierarchy
	GameObject* parent = nullptr;
	std::list<GameObject*> children;

	// Components
	ComponentTransform* transform = nullptr;
	ComponentMesh* mesh = nullptr;
	ComponentMaterial* material = nullptr;
	std::vector<Component*> components;
};

#endif /*__GAMEOBJECT_H__*/