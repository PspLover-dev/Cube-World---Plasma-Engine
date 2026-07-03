#version 330 core
in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTex;
uniform int uUseTexture;

out vec4 FragColor;

void main() {
    if (uUseTexture != 0) {
        vec4 tex = texture(uTex, vUV);
        FragColor = tex * vColor;
        if (FragColor.a < 0.004) {
            discard;
        }
    } else {
        FragColor = vColor;
    }
}
