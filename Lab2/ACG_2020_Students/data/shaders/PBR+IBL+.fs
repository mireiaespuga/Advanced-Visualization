
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec3 u_camera_position;

uniform vec4 u_color;

uniform float u_metalness;
uniform float u_roughness;

//light
uniform vec3 u_light_position;
uniform vec3 u_light_color;
uniform vec3 u_ambient_light;
uniform float u_light_intensity;
uniform float u_light_maxdist;

// textures
uniform float u_has_texture;
uniform float u_has_metalness_texture;
uniform float u_has_roughness_texture;
uniform float u_has_normal_texture;
uniform float u_has_emissive_texture;
uniform float u_has_ao_texture;

uniform sampler2D u_color_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_ao_texture;
uniform sampler2D u_texture_brdfLUT;

// Levels of the HDR Environment to simulate roughness material
// (IBL)
uniform samplerCube u_texture;
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

#define PI 3.14159265359
#define RECIPROCAL_PI 0.3183098861837697
#define GAMMA 2.2
#define INV_GAMMA 0.45

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv){
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

vec3 perturbNormal( vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel )
{
	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, V, texcoord);
	return normalize(TBN * normal_pixel);
}
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

// degamma
vec3 gamma_to_linear(vec3 color)
{
	return pow(color, vec3(GAMMA));
}

// gamma
vec3 linear_to_gamma(vec3 color)
{
	return pow(color, vec3(INV_GAMMA));
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
vec3 getReflectionColor(vec3 r, float roughness)
{
	float lod = roughness * 5.0;
	vec4 color;

	if(lod < 1.0) color = mix( textureCube(u_texture, r), textureCube(u_texture_prem_0, r), lod );
	else if(lod < 2.0) color = mix( textureCube(u_texture_prem_0, r), textureCube(u_texture_prem_1, r), lod - 1.0 );
	else if(lod < 3.0) color = mix( textureCube(u_texture_prem_1, r), textureCube(u_texture_prem_2, r), lod - 2.0 );
	else if(lod < 4.0) color = mix( textureCube(u_texture_prem_2, r), textureCube(u_texture_prem_3, r), lod - 3.0 );
	else if(lod < 5.0) color = mix( textureCube(u_texture_prem_3, r), textureCube(u_texture_prem_4, r), lod - 4.0 );
	else color = textureCube(u_texture_prem_4, r);
	
	color.rgb = gamma_to_linear(color.rgb);
	// Any other computations in linear-space (examples)
	// color *= u_exposure;
	// color *= u_tintColor;

	// Gamma correction: apply here only in case it's the last step (debug)
	//color.rgb = linear_to_gamma(color.rgb);

	return color.rgb;
}

void main()
{	
	vec4 color;
	vec2 uv = v_uv;
	if (u_has_texture == 1.0) {
		
		color = u_color * texture2D( u_color_texture, uv );
	} else {
		color = u_color;
	}


	float metalness;
	float roughness;

	if (u_has_metalness_texture == 1.0){
		metalness = clamp(texture2D( u_metalness_texture, uv ).x, 0.01, 0.99);
	} else {
		metalness = u_metalness;
	}
	if (u_has_roughness_texture == 1.0){
		roughness = clamp(texture2D( u_roughness_texture, uv ).x, 0.01, 0.99);
	} else {
		roughness = u_roughness;
	}
	vec4 emissive=vec4(0.0);
	if (u_has_emissive_texture == 1.0){
		emissive= texture2D( u_emissive_texture, uv );
	}

	float ambient_light = 1.0;
	if (u_has_ao_texture == 1.0){
		ambient_light = clamp(texture2D( u_ao_texture, uv ).x, 0.01, 0.99);
	}

	//vector towards the eye (V)
	vec3 V = normalize( u_camera_position - v_world_position );

	//vector from the point to the light (L)
	vec3 L =  u_light_position - v_world_position;
	float light_distance = length(L);
	L = normalize(L);

	//normal vector at the point (N)
	vec3 N = normalize( v_normal );

	//half vector between V and L (H)
	vec3 H = normalize( V + L );

	
	if (u_has_normal_texture == 1.0){
		vec3 normal_c = texture2D( u_normal_texture, uv ).xyz;
		N = perturbNormal( N, V, uv, normal_c );
	}

	//compute 
	float NoL = dot(N,L);
	NoL = clamp( NoL, 0.01, 0.99);

	float NoV = dot(N,V);
	NoV = clamp( NoV, 0.01, 0.99);

	float NoH = dot(N,H);
	NoH = clamp( NoH, 0.01, 0.99);

	float LoH = dot(L,H);
	LoH = clamp( LoH, 0.01, 0.99);

	vec3 R = reflect( -V, N );
	R = normalize(R);

	//specular F0 (conductors -> base color, dielectrics -> vec3(0.04))
	vec3 f0 = (1.0 - metalness) * vec3(0.04) + metalness * color.xyz;

	//compute the specular
	vec3 FSpecular = specularDFG(  roughness, f0, NoH, NoV, NoL, LoH);

	//diffuse cDiff (conductors -> vec3(0.0), dielectrics -> base color)
	vec3 diffuseColor = (1.0 - metalness) * color.xyz;

	//fLambert
	vec3 FDiffuse = diffuseColor / PI;

	//diffuse and specular terms from direct lighting
	vec3 direct = FSpecular + FDiffuse;

	//IBL
	//diffuse IBL
	vec3 diffuseSample = getReflectionColor ( R, roughness );
	vec3 diffuseIBL = diffuseSample * diffuseColor;

	//specular IBL
	vec3 specularSample = getReflectionColor ( R, roughness );
	vec4 brdfLUT = texture2D( u_texture_brdfLUT, vec2(roughness, NoV));
	vec3 SpecularBRDF = f0 * brdfLUT.x + brdfLUT.y;
	vec3 specularIBL = specularSample * SpecularBRDF;

	//diffuse and specular terms from IBL
	vec3 IBL = diffuseIBL + specularIBL;
	
	//compute a linear attenuation factor
	float att_factor = u_light_maxdist - light_distance;
	att_factor /= u_light_maxdist;
	att_factor = max(att_factor, 0.0);

	vec3 light_params = u_light_color * u_light_intensity * att_factor;
	//modulate light 
	vec3 light = (direct + IBL) * NoL * light_params;

	//apply to final pixel color
	color.xyz = light;
	color = color + emissive;

	//gamma
	color.xyz = linear_to_gamma(color.xyz);

	gl_FragColor = color;

}
