#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;

//uniform sampler2D screenTexture;
uniform sampler2D computeTexture;
uniform sampler2D screenTexture;
//layout( binding = 0, rgba32f ) uniform image2D computeTexture;
uniform vec2 iResolution;

void main()
{ 

    FragColor = //imageLoad( computeTexture, ivec2( gl_FragCoord.xy ) );
                //texture( computeTexture, TexCoords );
                mix( texture( computeTexture, TexCoords ), texture( screenTexture, TexCoords ), 0.5 );
                //texelFetch( computeTexture, ivec2( gl_FragCoord.xy ), 0 );
                //texture( computeTexture, TexCoords );
                //vec4( length( gl_FragCoord.xy / iResolution ) );

}