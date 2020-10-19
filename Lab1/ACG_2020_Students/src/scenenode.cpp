#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
}


SceneNode::SceneNode(const char* name, eNodeType nodeType)
{
	this->name = name;
	Mesh* mesh = new Mesh();
	StandardMaterial* material = new StandardMaterial();
	Texture* texture = new Texture();

	switch (nodeType)
	{
	case CUBEMAP:
		this->light = false;
		// Set mesh to node
		mesh->createCube();
		this->mesh = mesh;
		// Set model
		model.setScale(50.0f, 50.0f, 50.0f);
		this->model = model;

		// Set material
		material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/textureCube.fs");
		texture->cubemapFromImages("data/environments/city");
		material->texture = texture;
		this->material = material;

		break;

	case REFLECT:
		// Set mesh to node
		this->light = false;
		this->mesh = Mesh::Get("data/meshes/sphere.obj");

		// Set model
		model.setTranslation(2.0f, 2.0f, 2.0f);
		this->model = model;

		// Set material
		material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/reflect.fs");
		texture->cubemapFromImages("data/environments/city");
		material->texture = texture;
		this->material = material;
		break;

	case OBJECT: //TODO: TREURE TEXTURA A PHONG I Q FUNCIONI
		// Set mesh to node
		this->light = true;
		this->mesh = Mesh::Get("data/meshes/sphere.obj");

		// Set model
		model.setTranslation(2.0f, 2.0f, 2.0f);
		this->model = model;

		// Set material
		material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs"); 
		texture = Texture::Get("data/textures/blueNoise.png");
		material->texture = texture;
		this->material = material;
		break;
	default:
	
		break;
	}

}

SceneNode::~SceneNode()
{

}

void SceneNode::render(Camera* camera, Light* light = NULL)
{
	if (enable) {
		if (material) {
			glDisable(GL_DEPTH_TEST);
			material->render(mesh, model, camera, light);
			glEnable(GL_DEPTH_TEST);
		}
	}
}

void SceneNode::renderWireframe(Camera* camera)
{
	WireframeMaterial mat = WireframeMaterial();
	mat.render(mesh, model, camera);
}

void SceneNode::renderInMenu()
{
	//Model edit
	if (ImGui::TreeNode("Model")) 
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);

		ImGui::TreePop();
	}

	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		material->renderInMenu();
		ImGui::TreePop();
	}

	ImGui::Checkbox("Enable", &enable);

	ImGui::Checkbox("Light", &light);
}