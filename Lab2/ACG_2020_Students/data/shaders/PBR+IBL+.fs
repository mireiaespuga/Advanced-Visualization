
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
uniform float u_has_opacity_texture;
uniform float u_has_ao_texture;
uniform float u_has_dispf_texture;

uniform sampler2D u_color_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_opacity_texture;
uniform sampler2D u_ao_texture;
uniform sampler2D u_dispf_texture;
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

struct SceneVectors{
	vec3 V;
	vec3 L;
	vec3 N;
	vec3 H;
	vec3 R;
	float NoL;
	float NoV;
	float NoH;
	float LoH;
} sceneVectors;

struct MatProps{
	float ambient_occlusion;
	float opacity;
	float metalness;
	float roughness;
	vec4 emissive;
	vec4 color;
	float displacement;  
	vec3 normal_c;
} matProps;

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

//tonemapping
vec3 toneMap(vec3 color)
{
    return color / (color + vec3(1.0));
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

	return color.rgb;
}

void setsceneVectors(vec2 uv)
{
	//vector towards the eye (V)
	sceneVectors.V = normalize( u_camera_position - v_world_position + matProps.displacement );

	//vector from the point to the light (L)
	sceneVectors.L =  u_light_position - v_world_position  + matProps.displacement;
	sceneVectors.L = normalize(sceneVectors.L);

	sceneVectors.N = normalize( v_normal );

	//normal vector at the point (N)
	if(u_has_normal_texture == 1.0){
		sceneVectors.N = perturbNormal(sceneVectors.N, sceneVectors.V, uv, matProps.normal_c );
	}

	//half vector between V and L (H)
	sceneVectors.H = normalize( sceneVectors.V  + sceneVectors.L );

	//reflection vector R
	sceneVectors.R = reflect( -sceneVectors.V, sceneVectors.N );
	sceneVectors.R = normalize(sceneVectors.R);
}

void setMaterialProps(vec2 uv)
{

	if (u_has_texture == 1.0) {
		matProps.color = u_color * texture2D( u_color_texture, uv );
	} else {
		matProps.color = u_color;
	}
	
	//Color to linear
	matProps.color.rgb = gamma_to_linear(matProps.color.rgb);

	if (u_has_metalness_texture == 1.0){
		matProps.metalness = clamp(texture2D( u_metalness_texture, uv ).x, 0.01, 0.99);
	} else {
		matProps.metalness = u_metalness;
	}

	if (u_has_roughness_texture == 1.0){
		matProps.roughness = clamp(texture2D( u_roughness_texture, uv ).x, 0.01, 0.99);
	} else {
		matProps.roughness = u_roughness;
	}
	
	if (u_has_emissive_texture == 1.0){
		matProps.emissive= texture2D( u_emissive_texture, uv );
	}else{
		matProps.emissive= vec4(0.0);
	}

	if (u_has_dispf_texture == 1.0){
		matProps.displacement= clamp(texture2D( u_dispf_texture, uv ).x, 0.0, 1.0);
	}else{
		matProps.displacement=1.0;
	}

	//Emissive to linear
	matProps.emissive.rgb = gamma_to_linear(matProps.emissive.rgb);

	if (u_has_opacity_texture == 1.0){
		matProps.opacity = clamp(texture2D( u_opacity_texture, uv ).x, 0.0, 1.0);
	}else{
		matProps.opacity = 1.0;
	}

	if (u_has_ao_texture == 1.0){
		matProps.ambient_occlusion = clamp(texture2D( u_ao_texture, uv ).x, 0.01, 0.99);
	}else{
		matProps.ambient_occlusion = 1.0;
	}

	if (u_has_normal_texture == 1.0){
		matProps.normal_c = texture2D( u_normal_texture, uv ).xyz;

	}
}

vec3 computeDirect(vec3 f0, vec3 diffuseColor)
{
	//compute the specular
	vec3 FSpecular = specularDFG(  matProps.roughness, f0, sceneVectors.NoH, sceneVectors.NoV, sceneVectors.NoL, sceneVectors.LoH);

	//fLambert
	vec3 FDiffuse = diffuseColor / PI;

	//diffuse and specular terms from direct lighting
	return FSpecular + FDiffuse;
}

vec3 computeIBL(vec3 f0, vec3 diffuseColor)
{
	//diffuse IBL
	vec3 diffuseSample = getReflectionColor ( sceneVectors.R, matProps.roughness );
	vec3 diffuseIBL = diffuseSample * diffuseColor;

	//specular IBL
	vec3 specularSample = getReflectionColor ( sceneVectors.R, matProps.roughness );
	vec4 brdfLUT = texture2D( u_texture_brdfLUT, vec2(matProps.roughness, sceneVectors.NoV));
	vec3 SpecularBRDF = f0 * brdfLUT.x + brdfLUT.y;
	vec3 specularIBL = specularSample * SpecularBRDF;

	//diffuse and specular terms from IBL
	return (diffuseIBL + specularIBL) * matProps.ambient_occlusion;
}

vec3 computeLightParams()
{	
	//compute a linear attenuation factor
	float light_distance = length(u_light_position - v_world_position);
	float att_factor = u_light_maxdist - light_distance;
	att_factor /= u_light_maxdist;
	att_factor = max(att_factor, 0.0);

	return u_light_color * u_light_intensity * att_factor;
}

vec4 getPixelColor()
{
	//compute 
	sceneVectors.NoL = dot(sceneVectors.N, sceneVectors.L);
	sceneVectors.NoL = clamp( sceneVectors.NoL, 0.01, 0.99);

	sceneVectors.NoV = dot(sceneVectors.N, sceneVectors.V);
	sceneVectors.NoV = clamp( sceneVectors.NoV, 0.01, 0.99);
	 
	sceneVectors.NoH = dot(sceneVectors.N, sceneVectors.H);
	sceneVectors.NoH = clamp( sceneVectors.NoH, 0.01, 0.99);

	sceneVectors.LoH = dot(sceneVectors.L, sceneVectors.H);
	sceneVectors.LoH = clamp( sceneVectors.LoH, 0.01, 0.99);

	//specular F0 (conductors -> base color, dielectrics -> vec3(0.04))
	vec3 f0 = (1.0 - matProps.metalness) * vec3(0.04) +  matProps.metalness *  matProps.color.rgb;

	//diffuse cDiff (conductors -> vec3(0.0), dielectrics -> base color)
	vec3 diffuseColor = (1.0 - matProps.metalness) *  matProps.color.rgb;

	//compte direct lighting
	vec3 direct = computeDirect(f0, diffuseColor);

	//IBL
	vec3 IBL = computeIBL(f0, diffuseColor) ;
	
	//Compute light properties
	vec3 light_params = computeLightParams();

	//modulate light 
	vec3 light = (direct + IBL) * sceneVectors.NoL * light_params ;

	//apply to final pixel color
	matProps.color.xyz = light;
	matProps.color.a = matProps.opacity;

	return matProps.color;
}

void main()
{	
	vec2 uv = v_uv;
	vec4 p_color;

	//set material properties from sampler2D textures
	setMaterialProps(uv); 

	//Set Vectors V L H N R
	setsceneVectors(uv);

	p_color = getPixelColor();
	
	//tonemapping
	p_color.xyz = toneMap(p_color.xyz);

	//apply extra textures
	p_color = matProps.color + matProps.emissive;

	//gamma
	p_color.xyz = linear_to_gamma(p_color.xyz);

	gl_FragColor = p_color ;

}
