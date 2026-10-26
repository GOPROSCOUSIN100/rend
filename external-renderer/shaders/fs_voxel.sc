$input v_texcoord : TEXCOORD0
$input v_color : COLOR0
$input v_normal : TEXCOORD1
$input v_position : TEXCOORD2

// Uniforms
uniform sampler2D u_texture;
uniform vec3 u_cameraPos;
uniform vec4 u_fogColor;
uniform float u_fogDensity;
uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform float u_time;

// Output
$output o_color : COLOR

void main()
{
	// Sample texture
	vec4 texColor = texture2D(u_texture, v_texcoord);
	
	// Base color
	vec4 color = v_color * texColor;
	
	// Simple lighting (Lambertian diffuse)
	vec3 normal = normalize(v_normal);
	vec3 lightDir = normalize(u_lightDir);
	float diffuse = max(dot(normal, lightDir), 0.0);
	
	// Ambient light
	float ambient = 0.3;
	
	// Final color with lighting
	color.rgb *= (ambient + diffuse) * u_lightColor;
	
	// Fog
	vec3 cameraToFrag = v_position - u_cameraPos;
	float fogDistance = length(cameraToFrag);
	float fogFactor = exp(-u_fogDensity * u_fogDensity * fogDistance * fogDistance);
	color.rgb = mix(u_fogColor.rgb, color.rgb, fogFactor);
	
	// Time-based pulsing for debug
	// color.rgb += 0.1 * sin(u_time * 2.0 + v_position) * vec3(1.0, 0.5, 0.0);
	
	// Output final color
	o_color = color;
}
