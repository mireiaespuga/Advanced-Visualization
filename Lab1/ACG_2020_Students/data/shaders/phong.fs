
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform vec3 u_light_position;
uniform vec3 u_light_color;
uniform float u_has_light;

void main()
{
	vec4 color;

	if (u_has_light == 0) {
		//here we store the L vector
		vec3 L;

		//vector from the point to the light
		L = u_light_position - v_world_position;

		//we ignore the light distance for now
		L = normalize(L);
	
		//here we can store the total amount of light
		vec3 light = vec3(0.0);

		//lets add the ambient light first
		//light += u_ambient_light;

		//very important to normalize as they come
		//interpolated so normalization is lost
		vec3 N = normalize( v_normal );

		//compute how much is aligned
		float NdotL = dot(N,L);

		//light cannot be negative (but the dot product can)
		NdotL = clamp( NdotL, 0.0, 1.0 );

		//store the amount of diffuse light
		light += NdotL * u_light_color;

		vec2 uv = v_uv;

		color = u_color * texture2D( u_texture, uv );

		//apply to final pixel color
		color.xyz *= light;

	} else {
		vec2 uv = v_uv;

		color = u_color * texture2D( u_texture, uv );
	}

	gl_FragColor = color; 
}
