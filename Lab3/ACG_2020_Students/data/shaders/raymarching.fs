
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform vec3 u_camera_position;
uniform sampler3D u_texture;
uniform float u_time;
uniform mat4 u_viewprojection;
uniform int u_text_width;
uniform int u_text_height;
uniform int u_text_depth;
uniform float u_z_coord;

struct rayProperties{
	vec3 rayDirection;
	float rayStep;
	vec3 stepVector;
}rayprops;


void raySetup(){

	vec4 coord2d = vec4(u_text_width, u_text_height, u_z_coord+u_text_depth, 1.0);
	mat4 inverse_vp = inverse(u_viewprojection); 
	
	//project 2d coord into 3d
	vec4 p_4d = coord2d * inverse_vp;

	//we convert to homogenous 
	vec3 p_position3d = vec3(p_4d.x/p_4d.w, p_4d.y/p_4d.w, p_4d.z/p_4d.w);

	rayprops.rayDirection = normalize(p_position3d - u_camera_position);
	rayprops.stepVector = rayprops.rayStep * rayprops.rayDirection;

}

vec4 rayLoop(){

	int max_steps = 1000;
	vec4 finalColor = vec4(0.0);

	vec3 current_sample = u_camera_position; //ray_start
	
	for( int i=0; i<max_steps; i+=1){
	
		// volume sampling _> sample value
		float d = texture(u_texture, current_sample).x;
		
		// classification -> sample 
		vec4 sample_color = vec4(d,d,d,d);

		//Composition
		finalColor += rayprops.rayStep * (1 - finalColor.a) * sample_color;

		//Make a step on in the ray direction.
		current_sample += rayprops.stepVector;

		//Exit conditions
		if( current_sample.x < -1 || current_sample.x > 1 || current_sample.y < -1 || current_sample.y > 1 || current_sample.z < -1 || current_sample.z > 1 || finalColor.a > 0.95){
			break;
		}

	}	

	return finalColor;
}

void main()
{
	raySetup();

	vec4 finalColor = rayLoop();
	
	gl_FragColor = finalColor;
}
