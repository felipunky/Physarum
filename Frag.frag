#version 440 core
/*layout( location = 0 ) */out vec4 FragColor;

in vec4 outColour;

//layout( location = 0, r8 ) uniform image3D volumeTex;
uniform float iTimeDelta;
uniform vec2 iResolution;

void main()
{

    vec2 uv = gl_FragCoord.xy / iResolution;

    //vec4 sampleVolume = imageLoad( volumeTex, ivec3( gl_FragCoord.xy, 0 ) );

    FragColor = outColour;// + 10. * sampleVolume.xxxx;// * texture( volumeTex, gl_FragCoord.xyz ); //vec4( 1., 0., 0., 1. );

} 