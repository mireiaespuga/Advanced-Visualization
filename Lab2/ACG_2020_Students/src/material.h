#ifndef MATERIAL_H
#define MATERIAL_H

#include "light.h"
#include "framework.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "mesh.h"

class Material {
public:
	Shader* shader = NULL;
	Texture* color_texture = NULL;
	Texture* metalness_texture = NULL;
	Texture* roughness_texture = NULL;
	Texture* normal_texture = NULL;
	Texture* emissive_texture = NULL;
	Texture* ao_texture = NULL;
	Texture* opacity_texture = NULL;
	Texture* displacement_texture = NULL;
	Texture* texture_LUT = Texture::Get("data/textures/brdfLUT.png");

	bool has_texture = false;
	bool has_metalness_texture = false;
	bool has_roughness_texture = false;
	bool has_normal_texture = false;
	bool has_emissive_texture = false;
	bool has_ao_texture = false;
	bool has_opacity_texture = false;
	bool has_displacement_texture = false;
	bool metalness_in_roughness_texture = false;
	
	struct Environment {
		Texture* texture_environment_0 = new Texture();
		Texture* texture_environment_1 = new Texture();
		Texture* texture_environment_2 = new Texture();
		Texture* texture_environment_3 = new Texture();
		Texture* texture_environment_4 = new Texture();
		Texture* texture_environment = new Texture();
	} environment;

	vec4 color;
	float metalness;
	float roughness;
	enum  eTexType { FACTORYWALL, MOSSYROCK, MARBLETILE };
	char texType;
	bool isSphere = FALSE;


	virtual void setUniforms(Camera* camera, Matrix44 model, Light* light) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera, Light* light) = 0;
	virtual void renderInMenu(bool basic) = 0;
	virtual void setTex(eTexType texturetype);
	virtual void setTextureHDRE(HDRE* hdre);

};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model, Light* light );
	void render(Mesh* mesh, Matrix44 model, Camera * camera, Light* light );
	void renderInMenu(bool basic);
	void setTex(eTexType texturetype);
	void setTextureHDRE(HDRE* hdre);
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

#endif