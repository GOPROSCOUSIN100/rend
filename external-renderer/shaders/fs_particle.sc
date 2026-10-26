$input v_texcoord : TEXCOORD0
$input v_color : COLOR0
$input v_texIndex : TEXCOORD1
$input v_size : TEXCOORD2
$input v_rotation : TEXCOORD3

// Uniforms
uniform sampler2DArray u_textureArray;
uniform vec2 u_textureSize;

// Output
$output o_color : COLOR

void main()
{
	// Calculate texture index
	int texIndex = int(v_texIndex.x);
	
	// Sample from texture array
	// For now, use v_texcoord directly
	vec4 texColor = texture2DArray(u_textureArray, vec3(v_texcoord, float(texIndex)));
	
	// Apply color
	vec4 color = v_color * texColor;
	
	// Handle alpha
	if (color.a < 0.1) {
		discard;
	}
	
	o_color = color;
}
