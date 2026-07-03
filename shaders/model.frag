#version 330 core
in vec3 vColor;
in vec3 vNormal;

out vec4 FragColor;

void main() {
    vec3 n = normalize(vNormal);
    vec3 lightDir = normalize(vec3(0.35, 0.75, 0.5));
    float diff = max(dot(n, lightDir), 0.0);
    float ambient = 0.55;
    vec3 lit = vColor * (ambient + diff * 0.45);
    FragColor = vec4(lit, 1.0);
}
