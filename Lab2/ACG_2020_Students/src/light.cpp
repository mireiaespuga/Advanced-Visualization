#include "light.h"
#include "application.h"
#include "mesh.h"

unsigned int Light::lightID = 0;

Light::Light()
{
	this->name = std::string("Light" + std::to_string(lightID++));
	this->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	this->mesh = Mesh::Get("data/meshes/sphere.obj");
	this->maxDist = 8.0f;
	this->intensity = 1.0f;
}

Light::~Light()
{

}

void Light::render(Camera* camera)
{
	if (enable && show_light) {
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
}

void Light::setUniforms(Camera* camera) {
	//upload light uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", vec4(color, 1.f));
}

void Light::renderInMenu()
{
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
	ImGui::ColorEdit3("Diffuse", (float*)&color); // Edit 3 floats representing a color
	ImGui::SliderFloat("Max Distance", &maxDist, 1.0f, 100.0f);
	ImGui::SliderFloat("Intensity", &intensity, 1.0f, 5.0f);
	ImGui::Checkbox("Enable", &enable);
	ImGui::Checkbox("Show", &show_light);
}

