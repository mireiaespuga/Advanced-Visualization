#include "light.h"
#include "application.h"
#include "mesh.h"

unsigned int Light::lightID = 0;

Light::Light()
{
	this->Id = vec3(1.f, 1.f, 1.f);
	this->Is = vec3(0.6f, 0.6f, 0.6f);
	this->Ia = vec3(0.24f, 0.24f, 0.24f);
	this->name = std::string("Light" + std::to_string(lightID++));
	this->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	this->mesh = Mesh::Get("data/meshes/sphere.obj");
	this->maxDist = 8.0f;
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
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", vec4(Id, 1.f));
}

void Light::renderInMenu()
{
	//Light edit
	//TODO: canviar model a vector per no tenir scale ni rotation
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);

	ImGui::SliderFloat("Max Distance", &maxDist, 1.0f, 100.0f);
	ImGui::SliderFloat("Intensity", &intensity, 1.0f, 10.0f);

	ImGui::ColorEdit3("Diffuse", (float*)&Id); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Ambient", (float*)&ambient); // Edit 3 floats representing a color

	ImGui::ColorEdit3("Specular", (float*)&Is); // Edit 3 floats representing a color
	
	ImGui::Checkbox("Enable", &enable);
	ImGui::Checkbox("Show", &show_light);
	ImGui::Checkbox("Enable Ambient", &hasAmbient);
}

