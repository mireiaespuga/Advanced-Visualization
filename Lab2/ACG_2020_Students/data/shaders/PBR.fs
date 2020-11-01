
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec3 u_camera_position;
uniform vec4 u_color;
uniform sampler2D u_texture;
uniform vec3 u_light_position;
uniform vec3 u_light_color;
uniform float metalness;
uniform float roughness;
uniform float u_has_texture;

#define RECIPROCAL_PI 0.3183098861837697
#define PI 3.14159265359

// Normal Distribution Function using GGX Distribution --D
float D_GGX (	const in float NoH, const in float alpha )
{
	float a2 = alpha * alpha;
	float f = (NoH * NoH) * (a2 - 1.0) + 1.0;
	return a2 / (PI * f * f);
}

// Fresnel term with scalar optimization(f90=1) --F
vec3 F_Schlick( const in float LoH, const in vec3 f0)
{
	float f = pow(1.0 - LoH, 5.0);
	return f0 + (1.0 - f0) * f;
}


// Geometry Term: Geometry masking/shadowing due to microfacets --G
float GGX(float NdotV, float k){
	return NdotV / (NdotV * (1.0 - k) + k);
}
	
float G_Smith( float NdotV, float NdotL, float roughness) // -- G continuation
{
	float k = pow(roughness + 1.0, 2.0) / 8.0;
	return GGX(NdotL, k) * GGX(NdotV, k);
}


//this is the cook torrance specular reflection model
vec3 specularBRDF( float roughness, vec3 f0, float NoH, float NoV, float NoL, float LoH )
{
	float alpha = roughness * roughness;
	// Normal Distribution Function
	float D = D_GGX( NoH, alpha );

	// Fresnel Function
	vec3 F = F_Schlick( LoH, f0 );

	// Visibility Function (shadowing/masking)
	float G = G_Smith( NoV, NoL, roughness );
		
	// Norm factor
	vec3 spec = D * G * F;
	spec /= (4.0 * NoL * NoV);

	return spec;
}


void main()
{	
	vec4 color;
	if (u_has_texture == 1.0) {
		vec2 uv = v_uv;
		color = u_color * texture2D( u_texture, uv );
	} else {
		color = u_color;
	}

	//here we store the V vector
	vec3 V = normalize( u_camera_position - v_world_position );

	//vector from the point to the light
	vec3 L = normalize( u_light_position - v_world_position );

	//interpolated so normalization is lost
	vec3 N = normalize( v_normal );

	//here we store the H vector V + L
	vec3 H = normalize( V + L );

	//compute how much is aligned
	float NoL = dot(N,L);

	float NoV = dot(N,V);

	float NoH = dot(N,H);

	float LoH = dot(L,H);

	//compute refletion
	//vec3 R = reflect(-L,N);
	//float RoV = dot(R,v_position);

	//light cannot be negative (but the dot product can)
	//RoV = clamp( RoV, 0.0, 1.0);

	//we compute the reflection in base to the color and the metalness
	vec3 f0 = (1.0 - metalness) * vec3(0.4) + metalness * color.xyz;

	//metallic materials do not have diffuse
	vec3 diffuseColor = (1.0 - metalness) * color.xyz;

	//compute the specular
	vec3 FSpecular = specularBRDF(  roughness, f0, NoH, NoV, NoL, LoH);

	// Here we use the Burley, but you can replace it by the Lambert.
	vec3 FDiffuse = diffuseColor / PI;

	//add diffuse and specular reflection
	vec3 f = FSpecular + FDiffuse;

	//modulate direct light by light received
	vec3 light = f * u_light_color;

	//apply to final pixel color
	color.xyz = light;

	color.xyz = pow(color.xyz, vec3(1.0/2.2));

	gl_FragColor = color;

}
