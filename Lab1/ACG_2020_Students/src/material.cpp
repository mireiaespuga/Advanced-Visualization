#include "material.h"
#include "texture.h"
#include "application.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	Ka = vec3(1.0f, 1.0f, 1.0f);
	Kd = vec3(1.0f, 1.0f, 1.0f);
	Ks = vec3(1.0f, 1.0f, 1.0f);
	alpha = 30.0;
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model, Light* light = NULL)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);

	shader->setUniform("u_color", color);

	if (light) {
		shader->setUniform("u_light_position", light->model.getTranslation());
		shader->setUniform("Id", light->Id);
		shader->setUniform("Ia", light->Ia);
		shader->setUniform("Is", light->Is);
		shader->setUniform("Ka", Ka);
		shader->setUniform("Kd", Kd);
		shader->setUniform("Ks", Ks);
		shader->setUniform("alpha", alpha);
		shader->setUniform("u_has_light", 1.0);
	}
	else {
		shader->setUniform("u_has_light", 0.0);
	}
	
	if (texture) {
		shader->setUniform("u_texture", texture);
		shader->setUniform("u_has_texture", 1.0);
	} 
	else {
		shader->setUniform("u_has_texture", 0.0);
	}
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera, Light* light = NULL)
{
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model, light);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}


void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color

	ImGui::SliderFloat3("Ka", (float*)&Ka, 0.0f, 1.0f);
	ImGui::SliderFloat3("Ks", (float*)&Ks, 0.0f, 1.0f);
	ImGui::SliderFloat3("Kd", (float*)&Kd, 0.0f, 1.0f);
	ImGui::SliderFloat("Shine", &alpha, 0.07f, 100.0f);

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
