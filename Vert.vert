#version 440 core
layout(location = 0) in vec4 aPos;
/*layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
*/
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float iTime;

// Pass through normal vertex buffer, transform vertices to the 
// usual eye space.

void main()
{

	gl_PointSize = 100.0;

	vec3 position = aPos.xyz;// + sin( iTime );

	gl_Position = projection * view * model * vec4( position, 1.0 );

}