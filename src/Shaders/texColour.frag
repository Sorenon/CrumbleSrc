#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

uniform float alpha;

void main()
{
	//FragColor = vec4(texture(texture1, TexCoord).xyz, alpha);
	FragColor = vec4(0, 0, 0, alpha);
}