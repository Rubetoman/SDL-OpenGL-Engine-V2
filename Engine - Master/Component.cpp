#include "Component.h"
#include "GameObject.h"
#include "Globals.h"

Component::Component(GameObject* go, component_type type) : my_go(go), type(type)
{
}

Component::Component(const Component& comp)
{
	my_go = comp.my_go;
	type = comp.type;
	active = comp.active;
}

Component::~Component()
{
	CleanUp();
}

void Component::Update()
{

}

void Component::DrawOnInspector()
{
	ImGui::Checkbox("Active", &active);	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f / 7.0f, 0.7f, 0.7f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f / 7.0f, 0.8f, 0.8f));

	bool removed = ImGui::SmallButton("Delete Component");
	if (removed) Delete();

	ImGui::PopStyleColor(3);
}

void Component::CleanUp()
{

}

void Component::Delete()
{
	my_go->DeleteComponent(this);
}

int Component::GetComponentNumber() const
{
	if (my_go == nullptr)
	{
		LOG("Warning: this component isn't attached to any GameObject.");
		return -1;
	}
	auto pos = std::find(my_go->components.begin(), my_go->components.end(), this) - my_go->components.begin();
	if (pos >= my_go->components.size())
	{
		LOG("Warning: component not found as a component of %s.", my_go->name);
		return -1;
	}
	return pos;
}