#include "light.h"
#include "application.h"
#include "mesh.h"

unsigned int Light::lightID = 0;

Light::Light()
{
	this->color = vec3(1.f, 1.f, 1.f);
	this->name = std::string("Light" + std::to_string(lightID++));
	this->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	this->mesh = Mesh::Get("data/meshes/sphere.obj");
	this->model.scale(0.2f, 0.2f, 0.2f);
}

Light::~Light()
{

}

void Light::render(Camera* camera)
{
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void Light::setUniforms(Camera* camera) {
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", vec4(color, 1.f));
}

void Light::renderInMenu()
{
	//Light edit
	if (ImGui::TreeNode("Model"))
	{
		//TODO: canviar model a vector per no tenir scale ni rotation
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);

		ImGui::TreePop();
	}

	//Material
	if (ImGui::TreeNode("Color"))
	{
		ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
		ImGui::TreePop();
	}
	
	ImGui::Checkbox("Enable", &enable);
}

