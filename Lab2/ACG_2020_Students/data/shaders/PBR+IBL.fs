
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
uniform samplerCube u_texture_environment;

// Levels of the HDR Environment to simulate roughness material
// (IBL)
//uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

#define PI 3.14159265359
#define RECIPROCAL_PI 0.3183098861837697
#define GAMMA 2.2
#define INV_GAMMA 0.45

// Normal Distribution Function --D
float NDF (	const in float NoH, const in float alpha )
{
	float a2 = alpha * alpha;
	float f = (NoH * NoH) * (a2 - 1.0) + 1.0;
	return a2 / (PI * f * f);
}

// Fresnel term --F
vec3 Fresnel_Schlick( const in float LoH, const in vec3 f0)
{
	float f = pow(1.0 - LoH, 5.0);
	return f0 + (1.0 - f0) * f;
}


// Geometry Term: Geometry masking/shadowing due to microfacets --G	
float Geometry( float NdotV, float NdotL, float roughness) 
{
	float k = pow(roughness + 1.0, 2.0) / 8.0;
	float GL = NdotL / (NdotL * (1.0 - k) + k);
	float GV = NdotV / (NdotV * (1.0 - k) + k);
	return GL * GV;
}


//this is the cook torrance specular reflection model
vec3 specularDFG( float roughness, vec3 f0, float NoH, float NoV, float NoL, float LoH )
{
	float alpha = roughness * roughness;
	// Normal Distribution Function
	float D = NDF( NoH, alpha );

	// Fresnel Function
	vec3 F = Fresnel_Schlick( LoH, f0 );

	// Visibility Function (shadowing/masking)
	float G = Geometry( NoV, NoL, roughness );
		
	// Norm factor
	vec3 spec = D * G * F;
	spec /= (4.0 * NoL * NoV);

	return spec;
}


vec3 getReflectionColor(vec3 R, float roughness)
{
	float lod = roughness * 5.0;

	vec4 color;

	if(lod < 1.0) color = mix( textureCube(u_texture_environment, R), textureCube(u_texture_prem_0, R), lod );
	else if(lod < 2.0) color = mix( textureCube(u_texture_prem_0, R), textureCube(u_texture_prem_1, R), lod - 1.0 );
	else if(lod < 3.0) color = mix( textureCube(u_texture_prem_1, R), textureCube(u_texture_prem_2, R), lod - 2.0 );
	else if(lod < 4.0) color = mix( textureCube(u_texture_prem_2, R), textureCube(u_texture_prem_3, R), lod - 3.0 );
	else if(lod < 5.0) color = mix( textureCube(u_texture_prem_3, R), textureCube(u_texture_prem_4, R), lod - 4.0 );
	else color = textureCube(u_texture_prem_4, R);

	// Gamma correction
	color = pow(color, vec4(INV_GAMMA));

	return color.rgb;
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

	//vector towards the eye (V)
	//vec3 V = normalize( v_world_position - u_camera_position );
	vec3 V = normalize( u_camera_position - v_world_position );

	//vector from the point to the light (L)
	vec3 L = normalize( u_light_position - v_world_position );

	//normal vector at the point (N)
	vec3 N = normalize( v_normal );

	//half vector between V and L (H)
	vec3 H = normalize( V + L );

	//compute 
	float NoL = dot(N,L);

	float NoV = dot(N,V);

	float NoH = dot(N,H);

	float LoH = dot(L,H);

	vec3 R = reflect( V, N );

	//compute refletion
	//vec3 R = reflect(-L,N);
	//float RoV = dot(R,v_position);

	//light cannot be negative (but the dot product can)
	//RoV = clamp( RoV, 0.0, 1.0);

	//specular F0 (conductors -> base color, dielectrics -> vec3(0.04))
	vec3 f0 = (1.0 - metalness) * vec3(0.4) + metalness * color.xyz;

	//compute the specular
	vec3 FSpecular = specularDFG(  roughness, f0, NoH, NoV, NoL, LoH);

	//diffuse cDiff (conductors -> vec3(0.0), dielectrics -> base color)
	vec3 diffuseColor = (1.0 - metalness) * color.xyz;

	//fLambert
	vec3 FDiffuse = diffuseColor / PI;

	//diffuse and specular terms from direct lighting
	vec3 direct = FSpecular + FDiffuse;

	//diffuse IBL
	vec3 diffuseSample = getReflectionColor ( R, roughness );
	vec3 diffuseIBL = diffuseSample * FDiffuse;

	//specular IBL
	vec3 specularSample = getReflectionColor( R, roughness );
	vec3 specularIBL = specularSample * FSpecular;

	//diffuse and specular terms from IBL
	vec3 IBL = diffuseIBL + specularIBL;

	//modulate light 
	vec3 light = (direct + IBL) * u_light_color;

	//apply to final pixel color
	color.xyz = light;

	//gamma
	//color.xyz = pow(color.xyz, vec3(INV_GAMMA));

	gl_FragColor = color;

}
