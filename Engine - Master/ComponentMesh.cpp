#include "ComponentMesh.h"

#include "ComponentTransform.h"
#include "ComponentMaterial.h"

#include "Application.h"
#include "ModuleShader.h"
#include "ModuleResources.h"

#include "MeshImporter.h"

#include "GameObject.h"
#include "par_shapes.h"
#include "GL/glew.h"

ComponentMesh::ComponentMesh(GameObject* go) : Component(go, component_type::Mesh)
{
}

ComponentMesh::ComponentMesh(const ComponentMesh& comp) : Component(comp)
{
	mesh = comp.mesh;
	currentMesh = comp.currentMesh;
	App->resources->meshes.push_back(this);
}

ComponentMesh::~ComponentMesh()
{
}

Component* ComponentMesh::Duplicate()
{
	return new ComponentMesh(*this);
}

void ComponentMesh::CleanUp()
{
	Component::CleanUp();
	DeleteMesh();
}

bool ComponentMesh::DrawOnInspector()
{
	ImGui::PushID(this);
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Mesh Component"))
	{
		bool deleted = Component::DrawOnInspector();
		if (!deleted)
		{
			if (ImGui::BeginCombo("##meshCombo", currentMesh.c_str())) {

				for (std::vector<std::string>::iterator it = App->resources->file_meshes->begin(); it != App->resources->file_meshes->end(); ++it) 
				{
					bool isSelected = (currentMesh == (*it));
					if (ImGui::Selectable((*it).c_str(), isSelected)) 
					{
						currentMesh = (*it);

						if (isSelected)
							ImGui::SetItemDefaultFocus();
						else
							LoadMesh(currentMesh.c_str());
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Triangles Count: %d", mesh.num_indices / 3);
			ImGui::Text("Vertices Count: %d", mesh.num_vertices);
		}
		else
		{
			ImGui::PopID();
			return deleted;
		}
	}
	ImGui::PopID();
	return false;
}

void ComponentMesh::LoadMesh(const char* name) 
{
	if (mesh.vbo != 0)
		glDeleteBuffers(1, &mesh.vbo);

	if (mesh.ibo != 0)
		glDeleteBuffers(1, &mesh.ibo);

	MeshImporter::Load(&mesh, name);
	ComputeMesh();
	my_go->ComputeBBox();
}

void ComponentMesh::ComputeMesh()
{
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	unsigned offset = sizeof(math::float3);

	if (mesh.normals != nullptr)
	{
		mesh.normalsOffset = offset;
		offset += sizeof(math::float3);
	}

	if (mesh.uvs != nullptr)
	{
		mesh.texturesOffset = offset;
		offset += sizeof(math::float2);
	}

	mesh.vertexSize = offset;

	glBufferData(GL_ARRAY_BUFFER, mesh.vertexSize * mesh.num_vertices, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * mesh.num_vertices, mesh.vertices);

	if (mesh.normals != nullptr)
		glBufferSubData(GL_ARRAY_BUFFER, mesh.normalsOffset * mesh.num_vertices, sizeof(float) * 3 * mesh.num_vertices, mesh.normals);


	if (mesh.uvs != nullptr)
		glBufferSubData(GL_ARRAY_BUFFER, mesh.texturesOffset * mesh.num_vertices, sizeof(float) * 2 * mesh.num_vertices, mesh.uvs);


	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_indices * sizeof(unsigned), mesh.indices, GL_STATIC_DRAW);

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &mesh.vao);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (mesh.normalsOffset != 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(mesh.normalsOffset * mesh.num_vertices));
	}

	if (mesh.texturesOffset != 0) 
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(mesh.texturesOffset * mesh.num_vertices));
	}

	glBindVertexArray(0);

	mesh.boundingBox.SetNegativeInfinity();
	mesh.boundingBox.Enclose((float3*)mesh.vertices, mesh.num_vertices);
	App->resources->meshes.push_back(this);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ComponentMesh::RenderMesh(const math::float4x4& view, const math::float4x4& proj)
{
	//Draw meshes
	unsigned program = 0;

	if (my_go->material_comp != nullptr)
		program = App->shader->programs[my_go->material_comp->shader];
	else
		program = App->shader->programs[0];

	if (program < 1)
	{
		LOG("Program shader couldn't be found, it may not be loaded.");
		return;
	}

	glUseProgram(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, (const float*)&my_go->GetGlobalTransform()[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, (const float*)&view);
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, (const float*)&proj);

	// Render Material
	if (my_go->material_comp != nullptr && my_go->material_comp->active)
	{
		my_go->material_comp->RenderMaterial();
	}

	glBindVertexArray(mesh.vao);
	glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, nullptr);

	// Disable VAO
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void ComponentMesh::DeleteMesh()
{
	if (mesh.vbo != 0)
	{
		glDeleteBuffers(1, &mesh.vbo);
	}

	if (mesh.ibo != 0)
	{
		glDeleteBuffers(1, &mesh.ibo);
	}

	if (mesh.vao != 0)
	{
		glDeleteBuffers(1, &mesh.vao);
	}
}

void ComponentMesh::Delete()
{
	my_go->mesh_comp = nullptr;
	Component::Delete();
	App->resources->DeleteMesh(this);
}

/*void ComponentMesh::GenerateMesh(par_shapes_mesh_s* mesh)
{

	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Positions

	for (unsigned i = 0; i< unsigned(mesh->npoints); ++i)
	{
		math::float3 point(mesh->points[i * 3], mesh->points[i * 3 + 1], mesh->points[i * 3 + 2]);
		// TODO: Set position
		//point = my_go->transform.TransformPos(point);
		for (unsigned j = 0; j<3; ++j)
		{
			min_v[j] = min(min_v[j], point[i]);
			max_v[j] = max(max_v[j], point[i]);
		}
	}

	unsigned offset_acc = sizeof(math::float3);
	unsigned normals_offset = 0;

	if (mesh->normals)
	{
		normals_offset = offset_acc;
		offset_acc += sizeof(math::float3);
	}

	glBufferData(GL_ARRAY_BUFFER, offset_acc*mesh->npoints, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3)*mesh->npoints, mesh->points);

	// normals
	if (mesh->normals)
	{
		glBufferSubData(GL_ARRAY_BUFFER, normals_offset*mesh->npoints, sizeof(math::float3)*mesh->npoints, mesh->normals);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// indices

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned), indices , GL_STATIC_DRAW);

	unsigned* indices = (unsigned*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
		sizeof(unsigned)*mesh->ntriangles * 3, GL_MAP_WRITE_BIT);

	for (unsigned i = 0; i< unsigned(mesh->ntriangles * 3); ++i)
	{
		*(indices++) = mesh->triangles[i];
	}

	// generate VAO
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (normals_offset != 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(normals_offset*mesh->npoints));
	}

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	vertices.reserve(mesh->npoints);
	for (unsigned int i = 0; i < mesh->npoints; i++)
	{
		vertices.push_back(float3((float *)&mesh->points[i]));
	}

	//materialIndex = 0;
	num_indices = mesh->ntriangles * 3;
	num_vertices = mesh->npoints;
}*/

JSON_value* ComponentMesh::Save(JSON_value* component) const
{
	JSON_value* mesh = Component::Save(component);

	mesh->AddString("currentMesh", currentMesh.c_str());
	component->addValue("", mesh);

	return mesh;
}

void ComponentMesh::Load(JSON_value* component)
{
	Component::Load(component);

	currentMesh = component->GetString("currentMesh");
	LoadMesh(currentMesh.c_str());
}