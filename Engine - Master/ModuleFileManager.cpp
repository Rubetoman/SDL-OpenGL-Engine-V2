#include "ModuleFileManager.h"



ModuleFileManager::ModuleFileManager()
{
}


ModuleFileManager::~ModuleFileManager()
{
}

void ModuleFileManager::manageFile(const char* path)
{
	std::string extension = getFileExtension(path);

	if (extension == "fbx" || extension == "FBX")
	{
		App->model_loader->CleanUp();
		App->model_loader->LoadMesh(path);
	}
	else if (extension == "png" || extension == "dds" || extension == "jpg")
	{
		for (int i = 0; i < App->model_loader->materials.size(); ++i)
		{
			App->textures->unloadTexture(App->model_loader->materials[i].texture0);
		}
		App->model_loader->ChangeMeshTexture(path);
	}
	else
	{
		LOG("File format not supported");
	}
}

std::string ModuleFileManager::getFileExtension(const char* path)
{
	std::string filename = path;
	std::string extension = path;
	splitPath(path, nullptr, &filename, &extension);
	return extension;
}

void ModuleFileManager::splitPath(const char* full_path, std::string* path, std::string* filename, std::string* extension)
{
	std::string str = normalizePath(full_path);
	unsigned pos_slash = str.find_last_of('/');
	unsigned pos_dot = str.find_last_of('.');

	if (path != nullptr)
	{
		if (pos_slash < str.length())
			*path = str.substr(0, pos_slash + 1);
		else
			path->clear();
	}
	if (filename != nullptr)
	{
		if (pos_slash < str.length())
			*filename = str.substr(pos_slash + 1, pos_dot - pos_slash - 1);
		else
			*filename = str.substr(0, pos_dot);
	}
	if (extension != nullptr)
	{
		if (pos_dot < str.length())
			*extension = str.substr(pos_dot + 1);
		else
			extension->clear();
	}
}

std::string ModuleFileManager::normalizePath(const char * path)
{
	std::string str = path;

	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == '\\')
			str[i] = '/';
	}

	return str.c_str();
}