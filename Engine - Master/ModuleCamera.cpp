#include "Application.h"

#include "ModuleCamera.h"
#include "ModuleWindow.h"

#include "GL/glew.h"
#include "SDL.h"

static void DrawCoordinates();
static void DrawPlane();

ModuleCamera::ModuleCamera()
{
}

ModuleCamera::~ModuleCamera()
{
}

bool ModuleCamera::Init()
{
	float3 vertex_buffer_data[] = {
		{ -1.0f, -1.0f, 0.0f },
	{ 1.0f, -1.0f, 0.0f },
	{ 0.0f,  1.0f, 0.0f }
	};

	InitProyection();
	UpdateProyection();

	for (int i = 0; i < 3; i++)
	{
		float4 res = proj * view_matrix * float4(vertex_buffer_data[i], 1.0f);
		vertex_buffer_data[i] = res.xyz() / res.w;
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vbo;
}

update_status ModuleCamera::PreUpdate() 
{
	//if (App->input->keyboard[SLD_SCANCODE_Q]) 
	eye.y ++;
	target.y++;
	UpdateProyection();

	return UPDATE_CONTINUE;
}

update_status ModuleCamera::Update()
{

	DrawPlane();
	DrawCoordinates();


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		0,                  // attribute 0
		3,                  // number of componentes (3 floats)
		GL_FLOAT,           // data type
		GL_FALSE,           // should be normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return UPDATE_CONTINUE;
}

bool ModuleCamera::CleanUp()
{
	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
	}

	return true;
}

static void DrawCoordinates() 
{
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	// X axis
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.1f, 0.0f); glVertex3f(1.1f, -0.1f, 0.0f);
	glVertex3f(1.1f, 0.1f, 0.0f); glVertex3f(1.0f, -0.1f, 0.0f);
	// Y axis
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f(0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f(0.0f, 1.15f, 0.0f); glVertex3f(0.0f, 1.05f, 0.0f);
	// Z axis
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.05f, 0.1f, 1.05f); glVertex3f(0.05f, 0.1f, 1.05f);
	glVertex3f(0.05f, 0.1f, 1.05f); glVertex3f(-0.05f, -0.1f, 1.05f);
	glVertex3f(-0.05f, -0.1f, 1.05f); glVertex3f(0.05f, -0.1f, 1.05f);
	glEnd();
	glLineWidth(1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
}

static void DrawPlane() 
{
	glLineWidth(1.0f);
	glBegin(GL_LINES);

	float d = 200.0f;

	for (float i = -d; i <= d; i += 1.0f)
	{
		glVertex3f(i, 0.0f, -d);
		glVertex3f(i, 0.0f, d);
		glVertex3f(-d, 0.0f, i);
		glVertex3f(d, 0.0f, i);
	}
}

void ModuleCamera::InitProyection() 
{
	Frustum frustum;
	float aspect = SCREEN_WIDTH / SCREEN_HEIGHT;
	frustum.type = FrustumType::PerspectiveFrustum;
	frustum.pos = float3::zero;
	frustum.front = -float3::unitZ;
	frustum.up = float3::unitY;
	frustum.nearPlaneDistance = 0.1f;
	frustum.farPlaneDistance = 100.0f;
	frustum.verticalFov = math::pi / 4.0f;
	frustum.horizontalFov = 2.f * atanf(tanf(frustum.verticalFov * 0.5f)) *aspect;
	proj = frustum.ProjectionMatrix();
}

void ModuleCamera::UpdateProyection()
{
	// projection
	math::float3 f(target - eye); f.Normalize();
	math::float3 s(f.Cross(up)); s.Normalize();
	math::float3 u(s.Cross(f));

	view_matrix[0][0] = s.x;				view_matrix[0][1] = s.y;				view_matrix[0][2] = s.z;
	view_matrix[1][0] = u.x;				view_matrix[1][1] = u.y;				view_matrix[1][2] = u.z;
	view_matrix[2][0] = -f.x;				view_matrix[2][1] = -f.y;				view_matrix[2][2] = -f.z;

	view_matrix[0][3] = -s.Dot(eye);		view_matrix[1][3] = -u.Dot(eye);		view_matrix[2][3] = f.Dot(eye);
	view_matrix[3][0] = 0.0f;				view_matrix[3][1] = 0.0f;				view_matrix[3][2] = 0.0f;			view_matrix[3][3] = 1.0f;
}