#version 450

layout(push_constant) uniform PushConstants {
    float x;
    float y;
} pc;

vec2 positions[4] = vec2[](
    vec2(-0.05, -0.05),
    vec2( 0.05, -0.05),
    vec2(-0.05,  0.05),
    vec2( 0.05,  0.05)
);

void main() {
    // Normalizing X and Y slightly for demo purposes, so 1.0 unit = screen size
    gl_Position = vec4(positions[gl_VertexIndex] + vec2(pc.x * 0.1, pc.y * 0.1), 0.0, 1.0);
}
