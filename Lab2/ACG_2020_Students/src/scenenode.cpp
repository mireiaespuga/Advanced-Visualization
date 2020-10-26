#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
}


SceneNode::SceneNode(const char* name, eNodeType nodeType, Texture* texture)
{
	this->name = name;
	Mesh* mesh = new Mesh();
	StandardMaterial* material = new StandardMaterial();
	this->nodeType = nodeType;
	switch (nodeType)
	{
	case CUBEMAP:
		this->light = false;
		mesh->createCube();
		this->mesh = mesh;
		this->model.setScale(50.0f, 50.0f, 50.0f);
		material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/textureCube.fs");
		material->texture = texture;
		this->material = material;
		break;

	case REFLECT:
		this->light = false;
		material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/reflect.fs");
		material->texture = texture;
		this->material = material;
		break;

	case OBJECT: 
		material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs"); 
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
	ImGui::Checkbox("Enable", &enable);
	//Model edit
	if (nodeType == eNodeType::OBJECT || nodeType == eNodeType::REFLECT) {
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
		if (nodeType == eNodeType::OBJECT) {
			//Material
			if (material && ImGui::TreeNode("Material"))
			{
				material->renderInMenu();
				ImGui::TreePop();
			}
			ImGui::Checkbox("Light", &light);
		}
	}
}