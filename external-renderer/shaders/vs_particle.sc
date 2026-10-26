$input a_position
$input a_texcoord
$input a_color
$input a_size
$input a_rotation
$input a_texIndex

// Uniforms
uniform mat4 u_viewProj;

// Varying
$output v_texcoord : TEXCOORD0
$output v_color : COLOR0
$output v_texIndex : TEXCOORD1
$output v_size : TEXCOORD2
$output v_rotation : TEXCOORD3

void main()
{
	// Particle is billboarded - we'll handle this in geometry shader or with quad instancing
	// For now, just pass through
	gl_Position = u_viewProj * vec4(a_position, 1.0);
	
	// Pass varying data
	v_texcoord = a_texcoord;
	v_color = a_color;
	v_texIndex = vec2(a_texIndex, 0.0);
	v_size = vec2(a_size, 0.0);
	v_rotation = vec2(a_rotation, 0.0);
	
	// Point size for point rendering
	gl_PointSize = a_size;
}
