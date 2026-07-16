#version 450

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    vec4 colorMultiplier;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = push.mvp * vec4(inPosition, 1.0);

    // Simple directional lighting for now
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(inNormal), lightDir), 0.2); // ambient 0.2

    fragColor = push.colorMultiplier.rgb * diffuse;
    fragTexCoord = inTexCoord;
}
