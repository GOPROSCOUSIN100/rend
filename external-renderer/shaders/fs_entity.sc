$input v_texcoord : TEXCOORD0
$input v_color : COLOR0
$input v_normal : TEXCOORD1

// Uniforms
uniform sampler2D u_texture;
uniform vec3 u_lightDir;
uniform vec3 u_lightColor;

// Output
$output o_color : COLOR

void main()
{
	// Sample texture
	vec4 texColor = texture2D(u_texture, v_texcoord);
	
	// Base color
	vec4 color = v_color * texColor;
	
	// Simple lighting
	vec3 normal = normalize(v_normal);
	vec3 lightDir = normalize(u_lightDir);
	float diffuse = max(dot(normal, lightDir), 0.0);
	float ambient = 0.4;
	
	// Final color with lighting
	color.rgb *= (ambient + diffuse) * u_lightColor;
	
	// Alpha blending
	o_color = color;
}
