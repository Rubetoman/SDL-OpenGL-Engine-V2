#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Module.h"
#include "GameObject.h"
#include "Texture.h"

#include <vector>
#include <map>
#include "Util/crossguid/guid.hpp"

class ModuleResources : public Module
{
public:
	ModuleResources();
	~ModuleResources();

	inline const char* GenerateNewUID()
	{
		return xg::newGuid().str().c_str();
	}

public:
	std::vector<GameObject*> lights;		// List of all the lights on the scene
	std::map<Texture*, unsigned> textures;	// Map of all the textures and number of objects that use that texture
};

#endif /*__MODULERESOURCES_H__*/