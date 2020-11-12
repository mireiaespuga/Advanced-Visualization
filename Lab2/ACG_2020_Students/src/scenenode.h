#ifndef SCENENODE_H
#define SCENENODE_H

#include "framework.h"
#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"

class SceneNode {
public:

	static unsigned int lastNameId;
	enum eNodeType { CUBEMAP, OBJECT };
	eNodeType nodeType;

	SceneNode();
	SceneNode(const char* name, eNodeType nodeType);
	~SceneNode();

	Material * material = NULL;
	std::string name;
	Texture* texture = NULL;

	Mesh* mesh = NULL;
	Matrix44 model;
	bool light = true;
	bool enable = true;

	virtual void render(Camera* camera, Light* light);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();

};

#endif