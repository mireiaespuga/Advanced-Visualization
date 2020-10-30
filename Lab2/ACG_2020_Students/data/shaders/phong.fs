
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform vec3 u_light_position;
uniform float u_light_maxdist;
uniform vec3 Id;
uniform vec3 Ia;
uniform vec3 Is;
uniform float u_has_light;
uniform float u_has_texture;
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float alpha;

void main()
{	
	vec4 color;
	if (u_has_light == 1.0) {

		//here we store the L vector
		vec3 L;

		//vector from the point to the light
		L = u_light_position - v_world_position;
		float light_distance = length(L);

		//compute a linear attenuation factor
		//float att_factor = 1.0 / light_distance;
		float att_factor = u_light_maxdist - light_distance;
		att_factor /= u_light_maxdist;
		att_factor = max(att_factor, 0.0);

		//we ignore the light distance for now
		L = normalize(L);
	
		//here we can store the total amount of light
		vec3 Ip= vec3(0.0);

		//lets add the ambient light first
		Ip += Ka * Ia;

		//very important to normalize as they come
		//interpolated so normalization is lost
		vec3 N = normalize( v_normal );

		//compute how much is aligned
		float NdotL = dot(N,L);

		//light cannot be negative (but the dot product can)
		NdotL = clamp( NdotL, 0.0, 1.0 );

		//store the amount of diffuse light
		Ip+= Kd * NdotL * Id;
		// 
		//compute refletion
		vec3 R = reflect(-L,N);
		float RdotV = dot(R,v_position);

		//light cannot be negative (but the dot product can)
		RdotV = clamp( RdotV, 0.0, 1.0);

		//store the amount of diffuse light
		Ip += Ks*Is*pow(RdotV, alpha);

		if (u_has_texture == 1.0) {
			vec2 uv = v_uv;
			color = u_color * texture2D( u_texture, uv );
		} else {
			color = u_color;
		}

		//apply to final pixel color
		color.xyz *= Ip * att_factor;

	} else {
		if (u_has_texture == 1.0) {
			vec2 uv = v_uv;
			color = u_color * texture2D( u_texture, uv );
		} else {
			color = u_color;
		}
	}

	gl_FragColor = color;

}
