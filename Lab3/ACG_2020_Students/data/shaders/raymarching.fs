varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform float u_thr;
uniform vec4 u_color;
uniform vec3 u_camera_position;
uniform sampler3D u_texture;
uniform float u_time;
uniform mat4 u_viewprojection;
uniform mat4 u_model;
uniform float u_text_width;
uniform float u_text_height;
uniform float u_text_depth;
uniform float u_step;
uniform float u_h;
uniform sampler2D u_noise_texture;
uniform vec3 Id;
uniform vec3 Kd;
//uniform float u_z_coord;

struct rayProperties{
	vec3 rayDirection;
	float rayStep;
	vec3 stepVector;
	vec3 gradientN;
}rayprops;

vec3 to01range(vec3 vector){
	return (vector + vec3(1.0))/ vec3(2.0);
}

void raySetup(){

	rayprops.rayStep = u_step;
	mat4 inverse_model = inverse(u_model);

	vec3 local_camera = (inverse_model * vec4(u_camera_position, 1.0)).xyz;

	rayprops.rayDirection = normalize(v_position - local_camera);
	rayprops.stepVector = rayprops.rayStep * rayprops.rayDirection;
}

float sample_volume(float x_pos, float y_pos, float z_pos){
	return texture3D(u_texture, vec3(x_pos, y_pos, z_pos)).x;
}

void setGradient(vec3 pos){
	float g_x = sample_volume(pos.x+u_h, pos.y, pos.z) - sample_volume(pos.x-u_h, pos.y, pos.z);
	float g_y = sample_volume(pos.x, pos.y+u_h, pos.z) - sample_volume(pos.x, pos.y-u_h, pos.z);
	float g_z = sample_volume(pos.x, pos.y, pos.z+u_h) - sample_volume(pos.x, pos.y, pos.z-u_h);
	rayprops.gradientN = 0.5 * u_h * vec3(g_x, g_y, g_z);
}

vec4 rayLoop(){

	vec3 Ip = vec3(0.0);
	int max_steps = 1000;
	vec4 finalColor = vec4(0.0);

	float random_offset = texture2D( u_noise_texture, gl_FragCoord.xy * 0.005).x;
	vec3 current_sample = vec3(v_position.x, v_position.y, v_position.z) + random_offset * rayprops.rayDirection ; //ray_start
	current_sample = to01range(current_sample);
	for( int i=1; i<=max_steps; i+=1){
	
		// volume sampling
		float d = sample_volume(current_sample.x, current_sample.y, current_sample.z);

		
		// classification
		vec4 sample_color = vec4(d,d,d,d);
		
		if (d < 0.3){
			sample_color = vec4(1,0,0,d);
			//finalColor.a = 0.7;
		}else if (d < 0.5){
			sample_color = vec4(0,1,0,d);
			//finalColor.a = 0.1;
		}else{
			sample_color = vec4(1,1,1,d);
			//finalColor.a = 0.1;
		}

		sample_color.rgb *= sample_color.a;

		if(d > u_thr){
			finalColor.a = 1.0;

			setGradient(current_sample);

			rayprops.gradientN = to01range(rayprops.gradientN);
			//light cannot be negative (but the dot product can)
			vec3 L = normalize(u_camera_position - current_sample);
			float NdotL = dot(rayprops.gradientN, L);
			//NdotL = clamp( NdotL, 0.0, 1.0 );

			//store the amount of diffuse light
			Ip += Kd * NdotL * Id;
			
			finalColor += sample_color * (1 - finalColor.a);
			finalColor.xyz *= Ip;

		}else{
			finalColor += rayprops.rayStep * (1.0 - finalColor.a) * sample_color;
		}
		


		//Composition
		//Opcio 1
		finalColor += rayprops.rayStep * (1.0 - finalColor.a) *sample_color;
		//Opcio 2
		//finalColor.rgb +=  sample_color.rgb * sample_color.a + (1 - sample_color.a) *finalColor.rgb;
		//finalColor.a +=  sample_color.a + (1.0f - sample_color.a)*finalColor.a;    
		//Opcio 3
		//finalColor.rgb += rayprops.rayStep *( sample_color.rgb * sample_color.a + (1 - sample_color.a) *finalColor.rgb);
		//finalColor.a = (sample_color.a + (1.0f - sample_color.a)*finalColor.a);
       
		//Make a step on in the ray direction.
		current_sample += rayprops.stepVector;

		//Exit conditions
		if( current_sample.x < 0 || current_sample.x > 1 || current_sample.y < 0 || current_sample.y > 1 || current_sample.z < 0 || current_sample.z > 1 || finalColor.a > 1){ 
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