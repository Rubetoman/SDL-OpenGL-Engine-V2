#include "Application.h"
#include "WindowAbout.h"
#include "ModuleEditor.h"

#include "GL/glew.h"
#include "Assimp/include/assimp/version.h"
#include "IL/il.h"
#include "SDL_version.h"

WindowAbout::WindowAbout(const char* name) : Window(name)
{
}


WindowAbout::~WindowAbout()
{
}

void WindowAbout::Draw()
{
	ImGui::Begin(name.c_str(), &active);   // Pointer to bool variable (close when click on button)
	ImGui::Text(TITLE);
	ImGui::Text("Version:	BT - 2");
	ImGui::Text("Games are not free of propaganda.");
	ImGui::Text("C/C++ engine made by: ");
	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("Ruben Crispin De la Cruz", false, 0, { 165, 13 }))
		App->editor->ShowInBrowser("https://github.com/Rubetoman");

	ImGui::NewLine();
	ImGui::Text("Libraries: ");

	ImGui::NewLine(); ImGui::SameLine(30);
	std::string glewVersion = (const char*)glewGetString(GLEW_VERSION);
	glewVersion = "Glew " + glewVersion;
	if (ImGui::Selectable(glewVersion.c_str(), false, 0, { 142, 13 }))
		App->editor->ShowInBrowser("http://glew.sourceforge.net/");

	ImGui::NewLine(); ImGui::SameLine(30);
	std::string imguiVersion = "ImGui ";
	imguiVersion += ImGui::GetVersion();
	if (ImGui::Selectable(imguiVersion.c_str(), false, 0, { 142, 13 }))
		App->editor->ShowInBrowser("https://github.com/ocornut/imgui");

	ImGui::NewLine(); ImGui::SameLine(30);
	std::string glVersion = (const char*)glGetString(GL_VERSION);
	glVersion = "OpenGL " + glVersion;
	if (ImGui::Selectable(glVersion.c_str(), false, 0, { 310, 13 }))
		App->editor->ShowInBrowser("https://www.opengl.org/");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("MathGeoLib 1.5.0", false, 0, { 185, 13 }))
		App->editor->ShowInBrowser("https://github.com/juj/MathGeoLib");

	ImGui::NewLine(); ImGui::SameLine(30);
	SDL_version version;
	SDL_GetVersion(&version);
	std::string sdlVersion = "SDL ";
	sdlVersion += std::to_string(version.major) + '.' + std::to_string(version.minor) + '.' + std::to_string(version.patch);
	if (ImGui::Selectable(sdlVersion.c_str(), false, 0, { 135, 13 }))
		App->editor->ShowInBrowser("https://www.libsdl.org/");

	ImGui::NewLine(); ImGui::SameLine(30);
	std::string assimpVersion = "Assimp ";
	assimpVersion += std::to_string(aiGetVersionMajor()) + '.' + std::to_string(aiGetVersionMinor()) + '.' + std::to_string(aiGetVersionRevision());
	if (ImGui::Selectable(assimpVersion.c_str(), false, 0, { 155, 13 }))
		App->editor->ShowInBrowser("http://www.assimp.org/");

	ImGui::NewLine(); ImGui::SameLine(30);
	std::string devilVersion = "DevIL " + std::to_string(IL_VERSION);
	if (ImGui::Selectable(devilVersion.c_str(), false, 0, { 135, 13 }))
		App->editor->ShowInBrowser("http://openil.sourceforge.net/");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("mmgr 1.0", false, 0, { 130, 13 }))
		App->editor->ShowInBrowser("http://www.flipcode.com/archives/Presenting_A_Memory_Manager.shtml");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("ImGuizmo", false, 0, { 130, 13 }))
		App->editor->ShowInBrowser("https://github.com/CedricGuillemet/ImGuizmo");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("CrossGuid 0.2", false, 0, { 130, 13 }))
		App->editor->ShowInBrowser("https://github.com/graeme-hill/crossguid");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("Debug Draw v2", false, 0, { 130, 13 }))
		App->editor->ShowInBrowser("https://github.com/glampert/debug-draw");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("PhysFS 3.0.1", false, 0, { 130, 13 }))
		App->editor->ShowInBrowser("http://icculus.org/physfs/docs/html/");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("Rapidjson 1.1.0", false, 0, { 130, 13 }))
		App->editor->ShowInBrowser("https://github.com/Tencent/rapidjson");

	ImGui::NewLine(); ImGui::SameLine(30);
	if (ImGui::Selectable("Par/par_shapes.h", false, 0, { 130, 13 }))
		App->editor->ShowInBrowser("https://github.com/prideout/par/blob/master/par_shapes.h");

	// License
	ImGui::NewLine();
	ImGui::Text("MIT License Copyright (c) [2018] [Ruben Crispin]");

	// Close button
	ImGui::NewLine();
	if (ImGui::Button("Close"))
		active = false;
	ImGui::End();
}