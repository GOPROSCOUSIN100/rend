$input a_position
$input a_normal
$input a_texcoord
$input a_color

// Uniforms
uniform mat4 u_modelViewProj;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

// Varying (passed to fragment shader)
$output v_texcoord : TEXCOORD0
$output v_color : COLOR0
$output v_normal : TEXCOORD1
$output v_position : TEXCOORD2

void main()
{
	// Transform position
	gl_Position = u_modelViewProj * vec4(a_position, 1.0);
	
	// Pass varying data to fragment shader
	v_texcoord = a_texcoord;
	v_color = a_color;
	v_normal = a_normal;
	v_position = a_position;
}
