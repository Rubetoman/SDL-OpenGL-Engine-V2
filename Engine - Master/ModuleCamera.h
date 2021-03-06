#ifndef __MODULECAMERA_H__
#define __MODULECAMERA_H__

#include "Module.h"

#include "Point.h"
#include "Geometry/AABB.h"
#include "Geometry/LineSegment.h"
#include <vector>

class GameObject;
class ComponentCamera;

class ModuleCamera : public Module
{
	enum CameraRotation 
	{
		PositivePitch,
		NegativePitch,
		PositiveYaw,
		NegativeYaw
	};

public:
	ModuleCamera();
	~ModuleCamera();

	bool            Init()		override;
	bool			Start()		override;
	update_status	PreUpdate() override;
	update_status   Update()	override;
	bool            CleanUp()	override;

	void		UpdateScreenSize();

	void		TranslateCameraInput();
	void		RotateCameraInput();
	void		CameraSpeedInput(float modifier);
	void		WheelInputTranslation(const fPoint& wheel_motion);
	void		FitCamera(const math::AABB &boundingBox);
	GameObject* MousePick();
	void		DrawRaycast() const;

public:

	// Editor Camera
	GameObject* editor_camera_go = nullptr;
	ComponentCamera* editor_camera_comp = nullptr;

	// Mouse 
	bool new_scroll = true;
	float mouse_sensitivity = 0.6;

	//Mouse Picking
	std::vector<GameObject*> hitGOs;
	math::LineSegment raycast;

	// Mesh
	math::AABB* BBtoLook = nullptr;

};

#endif /* __MODULECAMERA_H__ */

