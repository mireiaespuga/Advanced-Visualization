
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform float u_z_coord;
uniform vec3 u_camera_position;
uniform sampler3D u_texture;
uniform float u_time;

struct rayProperties{
	vec4 rayDirection;
	float rayStep;
	vec3 rayVector;
	vec3 stepVector;
}rayprops;

void raySetup(){
	//First sample
		//- Step vector
		//- Final color = 0
	
	rayprops.rayDirection = normalize(v_position - u_camera_position);
	rayprops.stepVector = rayprops.rayStep * rayprops.rayDirection;

}

void rayLoop(){

	vec4 color_acc;
	vec4 color_i;
	vec3 current_sample = v_position; 
	float max_steps = 1000;

	for( int i=0; i<max_steps; i+=1){

	
	// volume sampling _> sample value
	// classification -> sample color
	//Composition -> finalColor += rayprops.rayStep * (1 - finalColor.a) * sampleColor;
	//next sample & early termination
	//Make a step on in the ray direction.
		current_sample = current_sample + rayprops.stepVector;

		//Exit conditions
		if( 	current_sample.x < -1 || current_sample.x > 1 ||
				current_sample.y < -1 || current_sample.y > 1 ||
				current_sample.z < -1 || current_sample.z > 1 || color_acc.a > 0.95){
			break;
		}

	}	


	
	//finish
}

void main()
{
	vec3 texture_coords;
	//Ray direction (between 2 points)
	float d = texture(u_texture, texture_coords).x;
	//Ray step (small length)

	//Sample pos (initialized as entry point to the volume)

	//Final color
	vec2 uv = v_uv;
	gl_FragColor = u_color * texture2D( u_texture, uv );
}
