#ifndef __COMPOMENTCOMPONENT_H__
#define __COMPOMENTCOMPONENT_H__

#include "Component.h"
#include "GameObject.h"
#include "Texture.h"

class ComponentMaterial : public Component
{
public:
	ComponentMaterial(GameObject* go);
	ComponentMaterial(const ComponentMaterial& comp);
	~ComponentMaterial();

	Component* Duplicate() override;
	void DrawOnInspector() override;
	void Delete() override;

	Texture* texture;
};

#endif /*__COMPOMENTCOMPONENT_H__*/