    vec3 irradiance = vec3(0.0);
	#ifndef SH_LOW
	const int num = 9;
	#else
	const int num = 4;
	#endif
	
    for(int i = 0; i < num; ++i)
        irradiance += radiance.c[i] * shCosine.c[i];
	
    return irradiance;
}

vec3 ComputeSHDiffuse(in vec3 normal, in SH9Color radiance)
{
    // Diffuse BRDF is albedo / Pi
    return ComputeSHIrradiance( normal, radiance ) * (1.0 / Pi);
}


void main()
{
	vec3 normal = normalize( v_normal );
	SH9Color coeffs;
	coeffs.c[0] = u_sh_coeffs[0];
	coeffs.c[1] = u_sh_coeffs[1];
	coeffs.c[2] = u_sh_coeffs[2];
	coeffs.c[3] = u_sh_coeffs[3];
	coeffs.c[4] = u_sh_coeffs[4];
	coeffs.c[5] = u_sh_coeffs[5];
	coeffs.c[6] = u_sh_coeffs[6];
	coeffs.c[7] = u_sh_coeffs[7];
	coeffs.c[8] = u_sh_coeffs[8];

	vec3 color = ComputeSHDiffuse( normal, coeffs );
	gl_FragColor = vec4( max( vec3(0.001), color ), 1.0 );
}