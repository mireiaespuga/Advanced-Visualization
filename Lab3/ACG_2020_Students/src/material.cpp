#include "material.h"
#include "texture.h"
#include "application.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);

	shader->setUniform("u_color", color);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

VolumeMaterial::VolumeMaterial()
{
	zComponent = 0.0;
	stepLength = 0.01;
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	threshold = 0.38;
	h = 0.07;
	plane1 = vec4(3.082, 10.959, 0.0, -10.959);
	plane2 = vec4(-4.110, 14.384, 0.0, -10.274);
}

VolumeMaterial::~VolumeMaterial()
{

}

void VolumeMaterial::renderInMenu() {
	ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f); // Edit float representing density thresh
	ImGui::SliderFloat("Step length", &stepLength, 0.001f, 0.5f); // Edit float representing step-length
	ImGui::SliderFloat("H", &h, 0.001f, 1.0f); // Edit float representing step-length
	if (ImGui::TreeNode("Volume clipping")) {
		ImGui::SliderFloat4("Plane 1", (float*)&plane1, 0.0f, 20.0f); // Edit float representing plane1
		ImGui::SliderFloat4("Plane 2", (float*)&plane2, 0.0f, 20.0f); // Edit float representing plane1
		ImGui::TreePop();
	}
}

void VolumeMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_z_coord", zComponent);
	shader->setUniform("u_step", stepLength);
	shader->setUniform("u_color", color);
	shader->setUniform("u_noise_texture", noise_texture, 1);
	shader->setUniform("u_lut_texture", lut_texture, 3);
	shader->setUniform("u_thr", threshold);
	shader->setUniform("u_plane1", plane1);
	shader->setUniform("u_plane2", plane2);
	
	shader->setUniform("u_h", h);

	if (texture)
		shader->setUniform("u_texture", texture, 2);
}

void VolumeMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}


WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
