#include "application.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "volume.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "extra/hdre.h"
#include "extra/imgui/imgui.h"
#include "extra/imgui/imgui_impl_sdl.h"
#include "extra/imgui/imgui_impl_opengl3.h"

#include <cmath>

Application* Application::instance = NULL;
Camera* Application::camera = nullptr;

float cam_speed = 10;
bool render_wireframe = false;

Application::Application(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_debug = true;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(-5.f, 1.5f, 10.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	// Create node and add it to the scene
	SceneNode * node = new SceneNode("Scene node");
	node_list.push_back(node);

	// Set mesh to node
	Mesh* mesh = new Mesh();
	mesh->createCube();
	node->mesh = mesh;

	// Set material
	//StandardMaterial* material = new StandardMaterial();
	//material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	//node->material = material;

	VolumeMaterial* material = new VolumeMaterial();
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/raymarching.fs");

	Volume* volume = new Volume();
	volume->loadPVM("data/volumes/CT-Abdomen.pvm");
	
	Texture* texture = new Texture();
	texture->create3DFromVolume(volume);
	material->texture = texture;
	material->noise_texture = Texture::Get("data/textures/blueNoise.png");
	node->material = material;
	
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

//what to do when the image has to be draw
void Application::render(void)
{
	//set the clear color (the background color)
	glClearColor(0, 0, 0, 1);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	for (int i = 0; i < node_list.size(); i++) {
		node_list[i]->render(camera);

		if(render_wireframe)
			node_list[i]->renderWireframe(camera);
	}

	//Draw the floor grid
	if(render_debug)
		drawGrid();
}

void Application::update(double seconds_elapsed)
{
	mouse_locked = false;
	float speed = seconds_elapsed * cam_speed; //the speed is defined by the seconds_elapsed so it goes constant
	float orbit_speed = seconds_elapsed * 0.5f;
	
	//camera speed modifier
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift

	float pan_speed = speed * 0.5f;

	//async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP))		camera->move(Vector3( 0.0f, 0.0f,  1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN))	camera->move(Vector3( 0.0f, 0.0f, -1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT))	camera->move(Vector3( 1.0f, 0.0f,  0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f,  0.0f) * speed);

	if (!HoveringImGui()) 
	{
		//move in first person view
		if (mouse_locked || Input::mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			mouse_locked = true;
			camera->rotate(-Input::mouse_delta.x * orbit_speed * 0.5, Vector3(0, 1, 0));
			Vector3 right = camera->getLocalVector(Vector3(1, 0, 0));
			camera->rotate(-Input::mouse_delta.y * orbit_speed * 0.5, right);
		}

		//orbit around center
		else if (Input::mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) //is left button pressed?
		{
			mouse_locked = true;
			camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
		}

		//camera panning
		else if(Input::mouse_state& SDL_BUTTON(SDL_BUTTON_MIDDLE)) 
		{
				mouse_locked = true;
				camera->move(Vector3(-Input::mouse_delta.x * pan_speed, 0.f, 0.f));
				camera->move(Vector3(0.f, Input::mouse_delta.y * pan_speed, 0.f));
		}
	}

	//move up or down the camera using Q and E keys
	if (Input::isKeyPressed(SDL_SCANCODE_Q) || Input::isKeyPressed(SDL_SCANCODE_SPACE)) camera->moveGlobal(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_E) || Input::isKeyPressed(SDL_SCANCODE_LCTRL)) camera->moveGlobal(Vector3(0.0f,  1.0f, 0.0f) * speed);
	
	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();

	SDL_ShowCursor(!mouse_locked);
	ImGui::SetMouseCursor(mouse_locked ? ImGuiMouseCursor_None : ImGuiMouseCursor_Arrow);
}

void Application::renderInMenu()
{
	// Show and edit your global variables on the fly here
}

//Keyboard event handler (sync input)
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: render_debug = !render_debug; break;
		case SDLK_F2: render_wireframe = !render_wireframe; break;
		case SDLK_F5: Shader::ReloadAll(); break; 
	}
}

void Application::onKeyUp(SDL_KeyboardEvent event)
{
}

void Application::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Application::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{

}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Application::onMouseWheel(SDL_MouseWheelEvent event)
{
	bool mouse_blocked = false;

	ImGuiIO& io = ImGui::GetIO();
	if (!mouse_locked)
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
		{
			if (event.x > 0) io.MouseWheelH += 1;
			if (event.x < 0) io.MouseWheelH -= 1;
			if (event.y > 0) io.MouseWheel += 1;
			if (event.y < 0) io.MouseWheel -= 1;
		}
		}
	mouse_blocked = ImGui::IsAnyWindowHovered();

	if (!mouse_blocked && event.y)
	{
		if (mouse_locked)
			cam_speed *= 1 + (event.y * 0.1);
		else
			camera->changeDistance(event.y * 0.5);
	}
}

void Application::onResize(int width, int height)
{
  std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

