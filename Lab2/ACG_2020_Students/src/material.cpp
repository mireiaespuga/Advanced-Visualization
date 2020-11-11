#include "material.h"
#include "texture.h"
#include "application.h"

StandardMaterial::StandardMaterial()
{
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	color = vec4(1.f, 1.f, 1.f, 1.f);
	metalness = 0.5;
	roughness = 0.5;
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
		shader->setUniform("u_light_color", light->color);
		shader->setUniform("u_light_intensity", light->intensity);
	}
	else {
		shader->setUniform("u_has_light", 0.0);
	}

	shader->setUniform("u_metalness", metalness);
	shader->setUniform("u_roughness", roughness);

	if (environment.texture_environment) {
		shader->setUniform("u_texture", environment.texture_environment, 0);
		shader->setUniform("u_texture_prem_0",environment.texture_environment_0, 1);
		shader->setUniform("u_texture_prem_1",environment.texture_environment_1, 2);
		shader->setUniform("u_texture_prem_2",environment.texture_environment_2, 3);
		shader->setUniform("u_texture_prem_3",environment.texture_environment_3, 4);
		shader->setUniform("u_texture_prem_4",environment.texture_environment_4, 5);
	}

	shader->setUniform("u_has_texture", has_texture ? 1.0 : 0.0);
	shader->setUniform("u_has_roughness_texture", has_roughness_texture ? 1.0 : 0.0);
	shader->setUniform("u_has_metalness_texture", has_metalness_texture ? 1.0 : 0.0);
	shader->setUniform("u_metalness_in_roughness", metalness_in_roughness_texture ? 1.0 : 0.0);
	shader->setUniform("u_has_normal_texture", has_normal_texture ? 1.0 : 0.0);
	shader->setUniform("u_has_emissive_texture", has_emissive_texture ? 1.0 : 0.0);
	shader->setUniform("u_has_ao_texture", has_ao_texture ? 1.0 : 0.0);
	shader->setUniform("u_has_opacity_texture", has_opacity_texture ? 1.0 : 0.0);
	shader->setUniform("u_has_dispf_texture", has_displacement_texture ? 1.0 : 0.0);

	if (color_texture && has_texture) 
		shader->setUniform("u_color_texture", color_texture, 6);

	if (metalness_texture && has_metalness_texture) 
		shader->setUniform("u_metalness_texture", metalness_texture, 7);

	if (roughness_texture) 
		shader->setUniform("u_roughness_texture", roughness_texture, 8);

	if (displacement_texture && has_displacement_texture) 
		shader->setUniform("u_dispf_texture", displacement_texture, 9);
	
	if (normal_texture && has_normal_texture) 
		shader->setUniform("u_normal_texture", normal_texture, 10);

	if (emissive_texture && has_emissive_texture) 
		shader->setUniform("u_emissive_texture", emissive_texture, 11);

	if (ao_texture && has_ao_texture) 
		shader->setUniform("u_ao_texture", ao_texture, 12);

	if (opacity_texture && has_opacity_texture) 
		shader->setUniform("u_opacity_texture", opacity_texture, 13);

	shader->setUniform("u_texture_brdfLUT", texture_LUT, 14);
	
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

void StandardMaterial::setTex(eTexType texturetype)
{
	texType = texturetype;
	switch (texturetype)
	{
	case FACTORYWALL:
		color_texture = Texture::Get("data/textures/factory_wall/sphere_color.png");
		roughness_texture = Texture::Get("data/textures/factory_wall/sphere_roughness.png");
		normal_texture = Texture::Get("data/textures/factory_wall/sphere_normal.png");
		ao_texture = Texture::Get("data/textures/factory_wall/sphere_ao.png");
		metalness_texture = Texture::Get("data/textures/factory_wall/sphere_metalness.png");
		displacement_texture = Texture::Get("data/textures/factory_wall/sphere_dispf.png");
		isSphere = TRUE;
		break;

	case MOSSYROCK:
		color_texture = Texture::Get("data/textures/mossy_rock/sphere_color.png");
		roughness_texture = Texture::Get("data/textures/mossy_rock/sphere_roughness.png");
		normal_texture = Texture::Get("data/textures/mossy_rock/sphere_normal.png");
		ao_texture = Texture::Get("data/textures/mossy_rock/sphere_ao.png");
		displacement_texture = Texture::Get("data/textures/mossy_rock/sphere_dispf.png");
		metalness_texture = NULL;
		isSphere = TRUE;
		break;

	case MARBLETILE:
		color_texture = Texture::Get("data/textures/marble/sphere_color.png");
		roughness_texture = Texture::Get("data/textures/marble/sphere_roughness.png");
		normal_texture = Texture::Get("data/textures/marble/sphere_normal.png");
		ao_texture = Texture::Get("data/textures/marble/sphere_ao.png");
		metalness_texture = Texture::Get("data/textures/marble/sphere_metalness.png");
		displacement_texture = Texture::Get("data/textures/marble/sphere_dispf.png");
		isSphere = TRUE;
		break;

	default:
		break;
	}
}

void StandardMaterial::setTextureHDRE(HDRE* hdre)
{
	environment.texture_environment->cubemapFromHDRE(hdre, 0);
	environment.texture_environment_0->cubemapFromHDRE(hdre, 1);
	environment.texture_environment_1->cubemapFromHDRE(hdre, 2);
	environment.texture_environment_2->cubemapFromHDRE(hdre, 3);
	environment.texture_environment_3->cubemapFromHDRE(hdre, 4);
	environment.texture_environment_4->cubemapFromHDRE(hdre, 5);
}

void StandardMaterial::renderInMenu(bool basic=false)
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
	if (isSphere && ImGui::TreeNode("Textures"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Ex", (int*)&texType, "FACTORYWALL\0MOSSYROCK\0MARBLETILE", 3);
		if (changed && texType == FACTORYWALL)
			setTex(FACTORYWALL);
		else if (changed && texType == MOSSYROCK)
			setTex(MOSSYROCK);
		else if (changed && texType == MARBLETILE)
			setTex(MARBLETILE);
		ImGui::TreePop();
	}
	if (color_texture) {
		ImGui::Checkbox("Enable color texture", &has_texture);
	}
	if (ao_texture) {
		ImGui::Checkbox("Enable ao texture", &has_ao_texture);
	}
	if (emissive_texture) {
		ImGui::Checkbox("Enable emissive texture", &has_emissive_texture);
	}
	if (normal_texture) {
		ImGui::Checkbox("Enable normal texture", &has_normal_texture);
	}
	if (displacement_texture) {
		ImGui::Checkbox("Enable displacement fields texture", &has_displacement_texture);
	}
	if (roughness_texture) {
		ImGui::Checkbox("Enable roughness texture", &has_roughness_texture);
	}
	if (metalness_texture || metalness_in_roughness_texture) {
		ImGui::Checkbox("Enable metalness texture", &has_metalness_texture);
	}
	if (opacity_texture) {
		ImGui::Checkbox("Enable opacity texture", &has_opacity_texture);
	}
	if (!has_roughness_texture) {
		ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
	}
	if (!has_metalness_texture) {
		ImGui::SliderFloat("Metalness", &metalness, 0.0f, 1.0f);
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

void Material::setTex(eTexType texturetype)
{
}

void Material::setTextureHDRE(HDRE* hdre)
{
}
