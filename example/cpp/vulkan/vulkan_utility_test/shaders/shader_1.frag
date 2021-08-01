#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
   
   vec4 textureColor = texture(texSampler, fragTexCoord);
   
   // outColor = vec4(fragColor, 1.0);
   // outColor = textureColor;
   // outColor = vec4(mix(fragColor.rgb, textureColor.rgb, 0.5), textureColor.a);
   outColor = vec4(fragColor.rgb * textureColor.rgb, textureColor.a);
}