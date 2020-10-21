#include "material.h"
#include "texture.h"
#include "application.h"

StandardMaterial::StandardMaterial()
{
	setMaterial(StandardMaterial::GENERIC);
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

void StandardMaterial::setMaterial(eMatType material)
{
	matType = material;
	switch (material)
	{
	case GENERIC:
		color = vec4(1.f, 1.f, 1.f, 1.f);
		Ka = vec3(1.0f, 1.0f, 1.0f);
		Kd = vec3(1.0f, 1.0f, 1.0f);
		Ks = vec3(1.0f, 1.0f, 1.0f);
		alpha = 30.0;
		break;

	case BLACKRUBBER:
		//color = vec4(1.f, 1.f, 1.f, 1.f);
		Ka = vec3(0.02f, 0.02f, 0.02f);
		Kd = vec3(0.01f, 0.01f, 0.01f);
		Ks = vec3(0.4f, 0.4f, 0.4f); 
		alpha = 0.078125*128;
		break;

	case PEARL: 
		//color = vec4(1.f, 1.f, 1.f, 1.f);
		Ka = vec3(0.25f, 0.20725f, 0.20725f);
		Kd = vec3(1.0f, 0.829f, 0.829f);
		Ks = vec3(0.296648f, 0.296648f, 0.296648f);
		alpha = 0.088 * 128;
		break;

	case GOLD:
		Ka = vec3(0.24725f, 0.1995f, 0.0745f);
		Kd = vec3(0.75164f, 0.60648f, 0.22648f);
		Ks = vec3(0.628281f, 0.555802f, 0.366065f);
		alpha = 51.2f;
		break;
	default:
		break;
	}
}

void StandardMaterial::renderInMenu()
{
	//Material
	if (ImGui::TreeNode("Custom"))
	{
		ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color

		ImGui::SliderFloat3("Ka", (float*)&Ka, 0.0f, 1.0f);
		ImGui::SliderFloat3("Ks", (float*)&Ks, 0.0f, 1.0f);
		ImGui::SliderFloat3("Kd", (float*)&Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("Shine", &alpha, 0.07f, 100.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Examples"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Ex", (int*)&matType, "GENERIC\0BLACKRUBBER\0PEARL\0GOLD", 4);
		if (changed && matType == GENERIC)
			setMaterial(GENERIC);
		else if (changed && matType == PEARL)
			setMaterial(PEARL);
		else if (changed && matType == BLACKRUBBER)
			setMaterial(BLACKRUBBER);
		else if (changed && matType == GOLD)
			setMaterial(GOLD);
		ImGui::TreePop();
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
