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
		shader->setUniform("u_light_maxdist", light->maxDist);
		shader->setUniform("u_light_color", light->Id);
		shader->setUniform("u_light_intensity", light->intensity);
		shader->setUniform("u_metalness", metalness);
		shader->setUniform("u_roughness", roughness);
		//shader->setUniform("Ia", light->Ia);
		//shader->setUniform("Is", light->Is);
	/*	shader->setUniform("Ka", Ka);
		shader->setUniform("Kd", Kd);
		shader->setUniform("Ks", Ks);
		shader->setUniform("alpha", alpha);*/
		//shader->setUniform("u_has_light", 1.0);
	}
	else {
		shader->setUniform("u_has_light", 0.0);
	}

	int texture_id = 0;

	if (texture_environment_0) {
		shader->setUniform("u_texture_prem_0", texture_environment_0, 0);
			texture_id++;
	}

	if (texture && has_texture) {
		shader->setUniform("u_texture", texture, 1);
		texture_id++;
		shader->setUniform("u_has_texture", 1.0);
	} else shader->setUniform("u_has_texture", 0.0);

	if (metalness_texture && has_metalness_texture) {
		shader->setUniform("u_metalness_texture", metalness_texture, 2);
		texture_id++;
		shader->setUniform("u_has_metalness_texture", 1.0);
	}
	else shader->setUniform("u_has_metalness_texture", 0.0);

	if (roughness_texture && has_roughness_texture) {
		shader->setUniform("u_roughness_texture", roughness_texture, 3);
		texture_id++;
		shader->setUniform("u_has_roughness_texture", 1.0);
	}
	else shader->setUniform("u_has_roughness_texture", 0.0);

	if (normal_texture && has_normal_texture) {
		shader->setUniform("u_normal_texture", normal_texture, 4);
		texture_id++;
		shader->setUniform("u_has_normal_texture", 1.0);
	}
	else shader->setUniform("u_has_normal_texture", 0.0);

	if (emissive_texture && has_emissive_texture) {
		shader->setUniform("u_emissive_texture", emissive_texture, 5);
		texture_id++;
		shader->setUniform("u_has_emissive_texture", 1.0);
	}
	else shader->setUniform("u_has_emissive_texture", 0.0);

	shader->setUniform("u_texture_brdfLUT", texture_LUT, 6);
	texture_id++;

	if (texture_environment_0) {
		texture_id++;
		shader->setUniform("u_texture_prem_1", texture_environment_1, 7);
		texture_id++;
		shader->setUniform("u_texture_prem_2", texture_environment_2, 8);
		texture_id++;
		shader->setUniform("u_texture_prem_3", texture_environment_3, 9);
		texture_id++;
		shader->setUniform("u_texture_prem_4", texture_environment_4, 10);
		texture_id++;
		shader->setUniform("u_texture_prem_5", texture_environment_5, 11);
		texture_id++;
	}

	//if (ao_texture && has_ao_texture) {
	//	shader->setUniform("u_ao_texture", ao_texture);
	//	shader->setUniform("u_has_ao_texture", true);
	//}


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
		metalness = 0.5;
		roughness = 0.5;
		break;

	case BLACKRUBBER:
		//color = vec4(1.f, 1.f, 1.f, 1.f);
		Ka = vec3(0.02f, 0.02f, 0.02f);
		Kd = vec3(0.01f, 0.01f, 0.01f);
		Ks = vec3(0.4f, 0.4f, 0.4f); 
		alpha = 0.078125*128;
		metalness = 0.5;
		roughness = 0.5;
		break;

	case PEARL: 
		//color = vec4(1.f, 1.f, 1.f, 1.f);
		Ka = vec3(0.25f, 0.20725f, 0.20725f);
		Kd = vec3(1.0f, 0.829f, 0.829f);
		Ks = vec3(0.296648f, 0.296648f, 0.296648f);
		alpha = 0.088 * 128;
		metalness = 0.5;
		roughness = 0.5;
		break;

	case GOLD:
		Ka = vec3(0.24725f, 0.1995f, 0.0745f);
		Kd = vec3(0.75164f, 0.60648f, 0.22648f);
		Ks = vec3(0.628281f, 0.555802f, 0.366065f);
		alpha = 51.2f;
		metalness = 0.5;
		roughness = 0.5;
		break;
	default:
		break;
	}
}

void StandardMaterial::setTex(eTexType texturetype)
{
	texType = texturetype;
	switch (texturetype)
	{
	case NORMAL:
		texture = Texture::Get("data/textures/normal.png");
		break;

	case ROUGHNESS:
		texture = Texture::Get("data/textures/roughness.png");
		break;

	case METALNESS:
		texture = Texture::Get("data/textures/metalness.png");
		break;

	case COLOR:
		texture = Texture::Get("data/textures/color.png");
		break;
	default:
		texture = NULL;
		break;
	}
}

void StandardMaterial::setTextureHDRE(HDRE* hdre)
{
	texture_environment_0->cubemapFromHDRE(hdre, 0);
	texture_environment_1->cubemapFromHDRE(hdre, 1);
	texture_environment_2->cubemapFromHDRE(hdre, 2);
	texture_environment_3->cubemapFromHDRE(hdre, 3);
	texture_environment_4->cubemapFromHDRE(hdre, 4);
	texture_environment_5->cubemapFromHDRE(hdre, 5);
}

void StandardMaterial::renderInMenu(bool basic=false)
{
	//Material
	//if (ImGui::TreeNode("Custom")){
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
	if (texture) {
		ImGui::Checkbox("Enable color texture", &has_texture);
	}
	if (emissive_texture) {
		ImGui::Checkbox("Enable emissive texture", &has_emissive_texture);
	}
	if (normal_texture) {
		ImGui::Checkbox("Enable normal texture", &has_normal_texture);
	}
	if (roughness_texture) {
		ImGui::Checkbox("Enable roughness texture", &has_roughness_texture);
	}
	if (metalness_texture) {
		ImGui::Checkbox("Enable metalness texture", &has_metalness_texture);
	}
	if (!has_roughness_texture) {
		ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
	}
	if (!has_metalness_texture) {
		ImGui::SliderFloat("Metalness", &metalness, 0.0f, 1.0f);
	}
	//ImGui::SliderFloat3("Ka", (float*)&Ka, 0.0f, 1.0f);
	//ImGui::SliderFloat3("Kd", (float*)&Kd, 0.0f, 1.0f);
	//ImGui::SliderFloat("Shine", &alpha, 0.07f, 100.0f);
	//ImGui::TreePop();
	//}
	//if (ImGui::TreeNode("Examples"))
	//{
	//	bool changed = false;
	//	changed |= ImGui::Combo("Ex", (int*)&matType, "GENERIC\0BLACKRUBBER\0PEARL\0GOLD", 4);
	//	if (changed && matType == GENERIC)
	//		setMaterial(GENERIC);
	//	else if (changed && matType == PEARL)
	//		setMaterial(PEARL);
	//	else if (changed && matType == BLACKRUBBER)
	//		setMaterial(BLACKRUBBER);
	//	else if (changed && matType == GOLD)
	//		setMaterial(GOLD);
	//	ImGui::TreePop();
	//}
	//if (basic && ImGui::TreeNode("Basic Textures"))
	//{
	//	bool changed = false;
	//	changed |= ImGui::Combo("Ex", (int*)&texType, "NORMAL\0ROUGHNESS\0METALNESS\0COLOR\0NONE", 4);
	//	if (changed && texType == NORMAL)
	//		setTex(NORMAL);
	//	else if (changed && texType == ROUGHNESS)
	//		setTex(ROUGHNESS);
	//	else if (changed && texType == METALNESS)
	//		setTex(METALNESS);
	//	else if (changed && texType == COLOR)
	//		setTex(COLOR);
	//	else if (changed && texType == NONE)
	//		setTex(NONE);
	//	ImGui::TreePop();
	//}

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

void Material::setTex(eTexType texturetype)
{
}

void Material::setTextureHDRE(HDRE* hdre)
{
	texture_environment_0->cubemapFromHDRE(hdre, 0);
	texture_environment_1->cubemapFromHDRE(hdre, 1);
	texture_environment_2->cubemapFromHDRE(hdre, 2);
	texture_environment_3->cubemapFromHDRE(hdre, 3);
	texture_environment_4->cubemapFromHDRE(hdre, 4);
	texture_environment_5->cubemapFromHDRE(hdre, 5);
}
