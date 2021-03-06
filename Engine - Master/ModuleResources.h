#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Module.h"

#include <vector>
#include "Util/crossguid/guid.hpp"
#include "Math/float2.h"

class GameObject;
class Texture;
class ComponentCamera;
class ComponentMesh;

class ModuleResources : public Module
{
public:
	ModuleResources();
	~ModuleResources();

	bool Init() override;
	update_status Update() override;
	bool CleanUp() override;

	std::string GenerateNewUID();
	unsigned GetLightNumber(GameObject& go) const;

	// Cameras
	unsigned GetCameraNumber(ComponentCamera& camera) const;
	void DeleteCamera(ComponentCamera* camera);

	// Meshes
	unsigned GetMeshNumber(ComponentMesh& mesh) const;
	void DeleteMesh(ComponentMesh* mesh);

	void UpdateFilesList();

	// External files
	void UpdateFBXMeshesList();
	void UpdateExternalTexturesList();

	// Own created files
	void UpdateMeshesList();
	void UpdateTexturesList();
	void UpdateScenesList();

public:
	std::vector<GameObject*> lights;		// List of all the lights on the scene

	// Default textures
	unsigned checkers_texture = 0u;
	math::float2 checkers_size = math::float2::zero;
	unsigned no_camera_texture = 0u;
	math::float2 no_camera_size = math::float2::zero;

	// Scene Cameras
	std::vector<ComponentCamera*> cameras;	// Vector with all the cameras on the scene

	// Meshes
	std::vector<ComponentMesh*> meshes;		// Vector of all the meshes on the scene

	//External files
	std::vector<std::string>* fbx_meshes = nullptr;
	std::vector<std::string>* external_textures = nullptr;

	// Own files
	std::vector<std::string>* file_meshes = nullptr;
	std::vector<std::string>* file_textures = nullptr;
	std::vector<std::string>* file_scenes = nullptr;

	bool meshes_dirty = false;
	bool textures_dirty = false;
	bool scenes_dirty = false;
};

#endif /*__MODULERESOURCES_H__*/