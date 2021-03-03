#version 440 core

struct Particle
{

	vec4 Position;
	vec4 Velocity;

};

layout(location = 0) in Particle particle;
/*layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
*/
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float iTime;

out vec4 outColour;

// Pass through normal vertex buffer, transform vertices to the 
// usual eye space.

void main()
{

	gl_PointSize = 1.0;

	vec3 position = particle.Position.xyz;//aPos.xyz;// + sin( iTime );

	outColour = vec4( 1., 0., 0., 1. );

	gl_Position = /* projection * view * model * */ vec4( position, 1.0 );

}