#ifndef LIGHT_H
#define LIGHT_H


#include "framework.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"



class Light {
public:
	static unsigned int lightID;

	Light();
	~Light();

	std::string name;
	vec3 color;
	Mesh* mesh = NULL;
	Shader* shader = NULL;
	Matrix44 model;
	bool enable = true;


	virtual void renderInMenu();
	virtual void render(Camera* camera);
	virtual void setUniforms(Camera* camera);

};
#endif