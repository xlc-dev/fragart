#version 330 core

in vec2 fragUV;

out vec4 FragColor;

uniform float iTime;

void main() {
  float r = 0.5 + 0.5 * sin(iTime + fragUV.x * 10.0);
  float g = 0.5 + 0.5 * sin(iTime + fragUV.y * 10.0);
  float b = 0.5 + 0.5 * sin(iTime + (fragUV.x + fragUV.y) * 5.0);

  FragColor = vec4(r, g, b, 1.0);
}
