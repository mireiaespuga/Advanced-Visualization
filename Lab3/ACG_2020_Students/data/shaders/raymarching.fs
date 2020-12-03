varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec3 u_camera_position;
uniform float u_time;
uniform mat4 u_viewprojection;
uniform mat4 u_model;

uniform float u_thr;
uniform float u_step;
uniform float u_h;

uniform sampler3D u_texture;
uniform sampler2D u_noise_texture;
uniform sampler2D u_lut_texture;

uniform vec4 u_plane1;
uniform vec4 u_plane2;
uniform bool u_apply_volume_clipping;
uniform bool u_apply_transfer_function;
uniform bool u_apply_jittering;

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
	rayprops.gradientN = vec3(g_x, g_y, g_z) / (2.0 * u_h);
}

vec4 rayLoop(){

	int max_steps = 1000;
	vec4 finalColor = vec4(0.0);
	vec3 current_sample;

	if (u_apply_jittering){
		float random_offset = texture2D( u_noise_texture, gl_FragCoord.xy * 0.005).x;
		current_sample = vec3(v_position.x, v_position.y, v_position.z) + random_offset * rayprops.rayDirection ; //ray_start
	} else {
		current_sample = vec3(v_position.x, v_position.y, v_position.z);
	}
	current_sample = to01range(current_sample);

	for( int i=1; i<=max_steps; i+=1){
		// volume sampling
		float d = sample_volume(current_sample.x, current_sample.y, current_sample.z);

		// classification
		vec4 sample_color = vec4(d,d,d,d);

		if (u_apply_volume_clipping) {
			float volClip1 = u_plane1.x*current_sample.x + u_plane1.y*current_sample.y + u_plane1.z*current_sample.z + u_plane1.w;
			float volClip2 = u_plane2.x*current_sample.x + u_plane2.y*current_sample.y + u_plane2.z*current_sample.z + u_plane2.w;

			if (volClip1 > 0 || volClip2 > 0){
				continue;
			}
		}

		if (u_apply_transfer_function){
			sample_color.rgba = texture2D(u_lut_texture, vec2(d, 1.0)).xyzw;
			sample_color.rgb *= sample_color.a;
		}
	
		//Composition
		if (d > u_thr){
			vec3 L = normalize(u_camera_position - v_world_position);
			L =  to01range(L);
			setGradient(current_sample);

			rayprops.gradientN = to01range(rayprops.gradientN);
			float NdotL = dot(rayprops.gradientN, L);
			NdotL = to01range(vec3(NdotL)).x;

			finalColor.rgb += (1.0 - finalColor.a) * sample_color.rgb * NdotL; //1-alfa es transmissivitat si es os no passa color pq l'os no deixa passar color
			finalColor.a = 1.0;

		}//else{
		//	finalColor += rayprops.rayStep * (1.0 - finalColor.a) * sample_color;
		//}
		
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
