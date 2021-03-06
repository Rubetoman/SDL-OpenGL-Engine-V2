#include "ModuleEditor.h"

#include "Globals.h"
#include "Application.h"

#include "ModuleRender.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleResources.h"
#include "ModuleFileManager.h"
#include "ModuleWindow.h"
#include "ModuleModelLoader.h"

#include "Window.h"
#include "Window.h"
#include "WindowScene.h"
#include "WindowAbout.h"
#include "WindowConsole.h"
#include "WindowHardware.h"
#include "WindowConfiguration.h"
#include "WindowHierarchy.h"
#include "WindowInspector.h"
#include "WindowCamera.h"
#include "WindowExplorer.h"

#include "GameObject.h"
#include "ComponentCamera.h"

#include "debugdraw.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "ImGuizmo/ImGuizmo.h"


ModuleEditor::ModuleEditor()
{
}

// Destructor
ModuleEditor::~ModuleEditor()
{
}

// Called before render is available
bool ModuleEditor::Init()
{
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

	// Add EditorWindows
	editorWindows.push_back(scene = new WindowScene("Scene"));
	editorWindows.push_back(about = new WindowAbout("About"));
	editorWindows.push_back(console = new WindowConsole("console"));
	editorWindows.push_back(hardware = new WindowHardware("hardware"));
	editorWindows.push_back(configuration = new WindowConfiguration("configuration"));
	editorWindows.push_back(hierarchy = new WindowHierarchy("hierarchy"));
	editorWindows.push_back(inspector = new WindowInspector("inspector"));
	editorWindows.push_back(explorer = new WindowExplorer("explorer"));

	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer->context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup style
	ImGui::StyleColorsDark();

	// Set default name for the scene
	strcpy(temp_name, "Untitled");

	return true;
}

update_status ModuleEditor::PreUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	CreateDockSpace();

	// Update performance
	configuration->fps_log.erase(configuration->fps_log.begin());
	configuration->fps_log.push_back(App->time->FPS);
	configuration->ms_log.erase(configuration->ms_log.begin());
	configuration->ms_log.push_back(App->time->real_delta_time * 1000.0f);

	// Update game performance
	configuration->fps_game_log.erase(configuration->fps_game_log.begin());
	configuration->fps_game_log.push_back(App->time->FPS);
	configuration->ms_game_log.erase(configuration->ms_game_log.begin());
	configuration->ms_game_log.push_back(App->time->delta_time * App->time->time_scale * 1000.0f);

	return UPDATE_CONTINUE;
}

update_status ModuleEditor::Update()
{
	ShowMainMenuBar();

	if (show_scene_save_popup)
		SceneSavePopup();
	if (show_scene_load_popup)
		SceneLoadPopup();

	//ImGui::ShowDemoWindow();	//Example Window
	return update;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	for (std::list<Window*>::iterator it = editorWindows.begin(); it != editorWindows.end(); ++it)
	{
		RELEASE((*it));
	}

	editorWindows.clear();
	console = nullptr;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void ModuleEditor::Draw()
{
	for (std::list<Window*>::iterator it = editorWindows.begin(); it != editorWindows.end(); ++it)
	{
		if ((*it)->isActive())
		{
			ImGui::SetNextWindowSizeConstraints({ 10,10 }, { (float)App->window->screen_width, (float)App->window->screen_height });
			(*it)->Draw();
		}
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ModuleEditor::DrawDebugReferences()
{
	if (show_grid)
	{
		dd::xzSquareGrid(-500.0f * scale, 500.0f * scale, 0.0f, 1.0f * scale, math::float3(0.65f, 0.65f, 0.65f));
	}

	if (selectedGO == nullptr && show_axis)
	{
		float axis_size = scale;
		dd::axisTriad(math::float4x4::identity, axis_size*0.125f, axis_size, 0, true);
	}
}

void ModuleEditor::CreateDockSpace() const
{
	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ (float)App->window->screen_width, (float)App->window->screen_height });
	ImGui::SetNextWindowBgAlpha(0.0f);

	//TODO: change this to a simple define
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", 0, windowFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspaceId = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void ModuleEditor::ShowMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("New Scene"))
			{
				strcpy(temp_name, "Untitled");
				App->scene->NewScene(true);
			}
			if (ImGui::MenuItem("Save Scene")) 
			{ 
				if (App->scene->name.empty())
				{
					show_scene_save_popup = true;
				}
				else
				{
					App->scene->SaveScene(App->scene->name.c_str());
				}
			}
			if (ImGui::MenuItem("Save Scene As..."))
			{
				if (!App->scene->name.empty())
				{
					strcpy(temp_name, App->scene->name.c_str());
				}
				else
				{
					strcpy(temp_name, "Untitled");
				}		
				show_scene_save_popup = true;
			}
			if (ImGui::MenuItem("Load Scene"))
			{
				strcpy(temp_name, "");
				App->resources->UpdateScenesList();
				show_scene_load_popup = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "ALT+F4")) { update = UPDATE_STOP; }

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Game Object"))
		{
			if (ImGui::MenuItem("Create Empty")) 
			{ 
				App->scene->CreateGameObject("GameObject"); 
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Create Sphere"))
			{
				GameObject* sphere = App->scene->CreateGameObject("Sphere", selectedGO);
				App->model_loader->LoadGeometry(sphere, Geometry_type::SPHERE);
				selectedGO = sphere;
			}
			if (ImGui::MenuItem("Create Cube"))
			{
				GameObject* cube = App->scene->CreateGameObject("Cube", selectedGO);
				App->model_loader->LoadGeometry(cube, Geometry_type::CUBE);
				selectedGO = cube;
			}
			if (ImGui::MenuItem("Create Plane"))
			{
				GameObject* plane = App->scene->CreateGameObject("Plane", selectedGO);
				App->model_loader->LoadGeometry(plane, Geometry_type::PLANE);
				selectedGO = plane;
			}
			if (ImGui::MenuItem("Create Torus"))
			{
				GameObject* torus = App->scene->CreateGameObject("Torus", selectedGO);
				App->model_loader->LoadGeometry(torus, Geometry_type::TORUS);
				selectedGO = torus;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Scene", NULL, scene->isActive())) { scene->toggleActive(); }
			if (ImGui::MenuItem("Inspector", NULL, inspector->isActive())) { inspector->toggleActive(); }
			if (ImGui::MenuItem("Hierarchy", NULL, hierarchy->isActive())) { hierarchy->toggleActive(); }
			ImGui::Separator();
			if (ImGui::BeginMenu("Cameras"))
			{
				for (auto &camera : App->resources->cameras)
				{
					GameObject* camera_go = camera->my_go;
					if (ImGui::MenuItem(camera_go->name.c_str(), NULL, camera->window->isActive())) { camera->window->toggleActive(); }
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Explorer", NULL, explorer->isActive())) { explorer->toggleActive(); }
			if (ImGui::MenuItem("Configuration", NULL, configuration->isActive())) { configuration->toggleActive(); }
			if (ImGui::MenuItem("Hardware Info", NULL, hardware->isActive())) { hardware->toggleActive(); }
			if (ImGui::MenuItem("Console", NULL, console->isActive())) { console->toggleActive(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About", NULL, about->isActive())) { about->toggleActive(); }
			ImGui::Separator();
			if (ImGui::MenuItem("View GIT Repository")) { ShowInBrowser("https://github.com/Rubetoman/SDL-OpenGL-Engine-V2"); }
			ImGui::EndMenu();
		}
		ImGui::SameLine(300); ImGui::Separator();
		// Show Play/Pause/Stop/Step buttons (depending on current Game State)
		if (App->time->game_running == Game_State::Stoped)
		{
			if (ImGui::Button("Play"))
			{
				App->scene->SaveScene("temporalScene");
				App->time->Start_Game();
			}
		}
		else
		{
			if (ImGui::Button("Stop"))
			{
				App->time->Stop_Game();
				App->scene->LoadScene("temporalScene");
				App->scene->DeleteScene("temporalScene");
			}
		}
		if (ImGui::Button("Pause"))
		{
			if(App->time->game_running == Game_State::Running)
				App->time->Pause_Game(true);
			else if(App->time->game_running == Game_State::Paused)
				App->time->Pause_Game(false);
		}
		// Step
		if (App->time->step_frame)
		{
			App->time->Pause_Game(true);
			App->time->step_frame = false;
		}
		if (App->time->game_running != Game_State::Running)
		{
			if (ImGui::Button("Step"))
			{
				App->time->step_frame = true;
				App->time->Pause_Game(false);
			}
		}
		ImGui::Separator();
		ImGui::EndMainMenuBar();
	}
}

void ModuleEditor::SceneSavePopup()
{
	ImGui::OpenPopup("Save Scene As");
	if (ImGui::BeginPopupModal("Save Scene As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Scene name:\n");
		
		ImGui::InputText(".proScene", temp_name, 64);
		ImGui::Separator();

		if (ImGui::Button("Save", ImVec2(120, 0))) 
		{
			App->scene->name = temp_name;
			App->scene->SaveScene(App->scene->name.c_str());
			show_scene_save_popup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			show_scene_save_popup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void ModuleEditor::SceneLoadPopup()
{
	ImGui::OpenPopup("Load Scene");
	if (ImGui::BeginPopupModal("Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// List of scenes on scenes folder
		ImGui::Text("Scenes list:\n");
		ImGui::BeginChild("ScenesList", ImVec2(ImGui::GetWindowContentRegionWidth(), 100), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
		for (std::vector<std::string>::iterator scene = App->resources->file_scenes->begin(); scene != App->resources->file_scenes->end(); ++scene)
		{
			if (ImGui::Selectable((*scene).c_str(), false))
			{
				std::string name = (*scene).c_str();
				App->file->SplitPath((*scene).c_str(), nullptr, &name, nullptr);
				strcpy(temp_name, name.c_str());
			}
		}
		ImGui::EndChild();

		// Name can be introduces manually in this InputText
		ImGui::Text("Scene name:\n");
		ImGui::InputText(".proScene", temp_name, 64);
		ImGui::Separator();

		if (ImGui::Button("Load", ImVec2(120, 0)))
		{
			App->scene->LoadScene(temp_name);
			show_scene_load_popup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			show_scene_load_popup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void ModuleEditor::ShowInBrowser(const char* url) const
{
	assert(url != nullptr);
	ShellExecute(0, "open", url, 0, 0, SW_SHOW);
}

void ModuleEditor::HandleInputs(SDL_Event& event)
{
	ImGui_ImplSDL2_ProcessEvent(&event);
}

WindowCamera* ModuleEditor::CreateCameraWindow(ComponentCamera& camera)
{
	WindowCamera* newCamera = new WindowCamera(camera.uuid.c_str());
	editorWindows.push_back(newCamera);
	newCamera->camera = &camera;
	if (App->resources->cameras.size() < 1)
		newCamera->toggleActive();
	return newCamera;
}

void ModuleEditor::DeleteCameraWindow(WindowCamera* camera)
{
	for (std::list<Window*>::iterator it_window = editorWindows.begin(); it_window != editorWindows.end();)
	{
		if (*it_window == camera)
		{
			delete *it_window;
			editorWindows.erase(it_window++);
		}
		else
		{
			++it_window;
		}
	}
}