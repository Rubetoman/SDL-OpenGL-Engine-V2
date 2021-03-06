#include "ModuleScene.h"

#include "Globals.h"
#include "Application.h"

#include "ModuleResources.h"
#include "ModuleEditor.h"
#include "ModuleFileManager.h"
#include "ModuleWindow.h"
#include "ModuleCamera.h"
#include "ModuleInput.h"

#include "WindowScene.h"

#include "Quadtree.h"
#include "GameObject.h"

#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"

#include <map>

ModuleScene::ModuleScene()
{
}


ModuleScene::~ModuleScene()
{
	for (std::vector<GameObject*>::iterator it_go = scene_gos.begin(); it_go != scene_gos.end();)
	{
		DeleteGameObject(*it_go);
		delete *it_go;
		scene_gos.erase(it_go++);
	}
	scene_gos.clear();
}

bool ModuleScene::Init()
{
	return InitScene();
}

bool ModuleScene::Start()
{
	root->Init();
	//LoadScene("test_scene.proScene");
	return true;
}

update_status ModuleScene::PreUpdate()
{
	if (quadtree_dirty)
	{
		ComputeSceneQuadtree();
		quadtree_dirty = false;
	}

	root->Update();

	if (dirty)
	{
		// CTRL + S: save scene
		if (App->input->GetKey(SDL_SCANCODE_LCTRL) && App->input->GetKey(SDL_SCANCODE_S))
			if (name.empty())
				App->editor->show_scene_save_popup = true;
			else
				SaveScene(name.c_str());
	}

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	if (root != nullptr)
	{
		root->CleanUp();
	}
	return true;
}

void ModuleScene::Draw(const math::float4x4& view, const math::float4x4& proj, ComponentCamera& camera)
{
	if (root != nullptr)
	{
		if (use_quadtree)
		{
			// Draw static GOs
			quadtree->Intersect(static_gos, camera.frustum);
			DrawStaticGameObjects(view, proj, camera);
			static_gos.clear();
		}

		// Draw non static GOs (If quadtree is not in use will draw also static GOs)
		//root->Draw(view, proj, camera);
		for (auto go : App->resources->meshes)
		{
			go->my_go->Draw(view, proj, camera);
		}
	}

	if (&camera == App->camera->editor_camera_comp)
	{
		// Draw cameras
		for (auto cam : App->resources->cameras)
		{
			cam->my_go->Draw(view, proj, camera);
		}

		// Draw lights
		for (auto light : App->resources->lights)
		{
			light->Draw(view, proj, camera);
		}

		if (use_quadtree && draw_quadtree)
			quadtree->Draw();

		if(App->editor->show_raycast)
			App->camera->DrawRaycast();
	}
}

void ModuleScene::DrawStaticGameObjects(const math::float4x4& view, const math::float4x4& proj, ComponentCamera& camera) const
{
	for (auto go : static_gos)
	{
		// Compute and Draw BBox on Editor
		AABB boundingBox = go->ComputeBBox();

		if (go->mesh_comp != nullptr && go->mesh_comp->active)
		{
			// Avoid drawing mesh if it is not inside frustum
			if (!camera.frustum_culling || camera.ContainsAABB(boundingBox))
				((ComponentMesh*)go->mesh_comp)->RenderMesh(view, proj);
		}
	}
}

void ModuleScene::DrawImGuizmo(ImGuizmo::OPERATION operation, ImGuizmo::MODE mode) const
{
	math::float2 pos = App->editor->scene->viewport;
	ImGuizmo::SetRect(pos.x, pos.y, App->window->screen_width, App->window->screen_height);
	ImGuizmo::SetDrawlist();

	GameObject* selectedGO = App->editor->selectedGO;

	if (selectedGO != nullptr)
	{
		ComponentTransform* transform = selectedGO->transform;
		ComponentCamera* editor_camera = App->camera->editor_camera_comp;

		math::float4x4 model = selectedGO->GetGlobalTransform();
		math::float4x4 proj = editor_camera->frustum.ProjectionMatrix();
		math::float4x4 view = editor_camera->frustum.ViewMatrix();

		ImGuizmo::SetOrthographic(false);

		model.Transpose();
		view.Transpose();
		proj.Transpose();
		ImGuizmo::Manipulate((float*)&view, (float*)&proj, operation, mode, (float*)&model, NULL, NULL, NULL, NULL);

		if (ImGuizmo::IsUsing())
		{
			model.Transpose();
			transform->SetGlobalTransform(model);
		}
	}
}

GameObject* ModuleScene::CreateGameObject(const char* name)
{
	GameObject* go;

	// Create a root in case it was deleted
	if(root == nullptr)
		root = new GameObject("World");

	// Check the pointer isn't null
	if (name != nullptr && name[0] != '\0')
	{
		go = new GameObject(name, root);
	}	
	else	// Give default name
	{
		go = new GameObject(GO_DEFAULT_NAME, root);
	}	
	SetSceneDirty(true);
	return go;
}

GameObject* ModuleScene::CreateGameObject(const char* name, GameObject* parent)
{
	GameObject* go;

	// Create a root in case it was deleted
	if (root == nullptr)
		root = new GameObject("World");

	// Check the pointers aren't null
	if (parent != nullptr)
	{
		if (name != nullptr && name[0] != '\0')
			go = new GameObject(name, parent);
		else
			go = new GameObject(GO_DEFAULT_NAME, parent);
	}
	else
	{
		LOG("Warning: parent == nullptr, creating GameObject without parent. \n ")
		go = CreateGameObject(name);
	}
	SetSceneDirty(true);
	return go;
}

GameObject* ModuleScene::CreateGameObject(const char* name, math::float4x4& transform)
{
	GameObject* go;

	// Create a root in case it was deleted
	if (root == nullptr)
		root = new GameObject("World");

	// Check the pointers aren't null
	if (name != nullptr && name[0] != '\0')
		go = new GameObject(name, transform, root);
	else
		go = new GameObject(GO_DEFAULT_NAME, transform, root);
	
	SetSceneDirty(true);
	return go;
}

GameObject* ModuleScene::CreateGameObject(const char* name, math::float4x4& transform, GameObject* parent)
{
	GameObject* go;

	// Create a root in case it was deleted
	if (root == nullptr)
		root = new GameObject("World");

	// Check the pointers aren't null
	if (parent != nullptr)
	{
		if (name != nullptr && name[0] != '\0')
			go = new GameObject(name, transform, parent);
		else
			go = new GameObject(GO_DEFAULT_NAME, transform, parent);
	}
	else
	{
		LOG("Warning: parent == nullptr, creating GameObject without parent. \n ")
		go = CreateGameObject(name, transform);
	}
	SetSceneDirty(true);
	return go;
}

void ModuleScene::DeleteGameObject(GameObject* go)
{
	if (go == nullptr)
	{
		LOG("Warning: GameObject == nullptr.");
		return;
	}
	if (go == root)
		root = nullptr;

	SetSceneDirty(true);
	go->DeleteGameObject();
}

GameObject* ModuleScene::DuplicateGameObject(const GameObject* go)
{
	GameObject* new_go = new GameObject(*go);
	new_go->parent = go->parent;
	new_go->parent->children.push_back(new_go);
	SetSceneDirty(true);
	return new_go;
}

void ModuleScene::Unchild(GameObject* go)
{
	if (go == nullptr)
	{
		LOG("Warning: GameObject == nullptr.");
		return;
	}
	go->SetParent(root);
	SetSceneDirty(true);
}

unsigned ModuleScene::GetSceneGONumber(GameObject& go) const
{
	auto pos = std::find(scene_gos.begin(), scene_gos.end(), &go) - scene_gos.begin();
	if (pos >= scene_gos.size())
	{
		LOG("Warning: go not found on scene_gos.");
		return -1;
	}
	return pos;
}

#pragma region scene management functions

bool ModuleScene::InitScene()
{
	quadtree = new Quadtree();
	
	// Root
	root = new GameObject("World");
	root->static_GO = true;
	scene_gos.push_back(root);

	//TODO: Change it for an ambient light and added to scene_gos without parent
	// Default Light
	GameObject* default_light = CreateGameObject("Default Light", root);
	default_light->static_GO = true;
	default_light->transform->position = math::float3(-2.0f, 0.0f, 6.0f);
	default_light->CreateComponent(component_type::Light);

	// Game Main Camera
	GameObject* game_camera = CreateGameObject("Game Camera", root);
	game_camera->transform->position = math::float3(0.0f, 2.0f * App->editor->scale, 10.0f * App->editor->scale);
	game_camera->CreateComponent(component_type::Camera);

	quadtree_dirty = true;
	SetSceneDirty(true);

	return true;
}

void ModuleScene::NewScene(bool init)
{
	quadtree->Clear();

	App->editor->selectedGO = nullptr;

	// Delete root
	root->DeleteGameObject();
	scene_gos.erase(scene_gos.begin() + GetSceneGONumber(*root));

	name = "";

	App->resources->CleanUp();
	App->resources->UpdateFilesList();

	if(init) InitScene();
	else	 SetSceneDirty(true);
}

bool ModuleScene::SaveScene(const char* scene_name)
{
	JSON_file* scene = App->json->openWriteFile(App->file->getFullPath(scene_name, SCENES_FOLDER, SCENES_EXTENSION).c_str());
	if (scene == nullptr)
	{
		LOG("Error saving scene in %s.", App->file->getFullPath(scene_name, SCENES_FOLDER, SCENES_EXTENSION).c_str());
		return false;
	}

	JSON_value* gameObjects = scene->createValue();
	gameObjects->convertToArray();

	root->Save(gameObjects);

	scene->addValue("Root", gameObjects);
	scene->Write();
	App->json->closeFile(scene);

	App->resources->UpdateScenesList();

	SetSceneDirty(false);

	LOG("Scene saved successfully.");
	return true;
}

bool ModuleScene::LoadScene(const char* scene_name)
{
	// Load scene file
	LOG("Loading %s.", scene_name);

	JSON_file* scene = App->json->openReadFile(App->file->getFullPath(scene_name, SCENES_FOLDER, SCENES_EXTENSION).c_str());
	if (scene == nullptr)
	{
		LOG("Error loading scene, %s could not be opened.", App->file->getFullPath(scene_name, SCENES_FOLDER, SCENES_EXTENSION).c_str());
		return false;
	}

	NewScene(false);

	// Change scene name
	name = scene_name;

	// Load JSON
	JSON_value* go_root = scene->getValue("Root"); //It is an array of values
	if (go_root->getRapidJSONValue()->IsArray()) //Just make sure
	{
		std::map<std::string, GameObject*> gameobjects;
		for (int i = 0; i < go_root->getRapidJSONValue()->Size(); i++)
		{
			GameObject* go = new GameObject("");
			go->Load(go_root->getValueFromArray(i));
			gameobjects.insert(std::pair<std::string, GameObject*>(go->uuid, go));
		}

		for (std::map<std::string, GameObject*>::iterator it_go = gameobjects.begin(); it_go != gameobjects.end(); it_go++)
		{
			if ((*it_go).second->parentUID == "") //If it has no parent, add it to the scene list
			{
				scene_gos.push_back((*it_go).second);
				if((*it_go).second->name == "World")
					root = (*it_go).second;
			}
			else
			{
				GameObject* parent = gameobjects[(*it_go).second->parentUID];
				(*it_go).second->parent = parent;
				parent->children.push_back((*it_go).second);
			}
		}
	}
	App->json->closeFile(scene);

	quadtree_dirty = true;
	SetSceneDirty(false);

	LOG("Scene load successful.");
	return true;
}

bool ModuleScene::DeleteScene(const char* scene_name)
{
	bool deleted = App->json->deleteFile(App->file->getFullPath(scene_name, SCENES_FOLDER, SCENES_EXTENSION).c_str());

	if (!deleted)
	{
		LOG("Error loading scene, %s could not be opened.", App->file->getFullPath(scene_name, SCENES_FOLDER, SCENES_EXTENSION).c_str());
		return false;
	}
	else
	{
		LOG("Scene deleted: [%s]", scene_name);
		return true;
	}
}

bool ModuleScene::SetSceneDirty(bool set)
{
	dirty = set;

	// Change window title
	std::string windowTitle = name.empty()? "Untitled" : name;
	windowTitle += dirty ? "* - " : " - ";
	windowTitle += TITLE;

	SDL_SetWindowTitle(App->window->window, windowTitle.c_str());

	return dirty;
}

#pragma endregion

#pragma region Quadtree functions

void ModuleScene::FillQuadtree(GameObject* go)
{
	if (go != nullptr && go->static_GO)
	{
		quadtree->Insert(go);

		for (auto child : go->children)
		{
			FillQuadtree(child);
		}
	}
}

void ModuleScene::ComputeSceneQuadtree()
{
	quadtree->Clear();

	for (auto go : scene_gos)
	{
		ResizeQuadtree(go);
		FillQuadtree(go);
	}
}

void ModuleScene::ResizeQuadtree(GameObject* go)
{
	if (go != nullptr && go->static_GO)
	{
		quadtree->QuadTree_Box.Enclose(go->ComputeStaticTotalBBox());

		for (auto child : go->children)
		{
			ResizeQuadtree(child);
		}
	}
}

#pragma endregion