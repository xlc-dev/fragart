#version 330 core

uniform float iTime;
uniform vec2 iResolution;
out vec4 FragColor;

vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b * cos(6.283185 * (c * t + d));
}

void main() {
    vec2 uv = (gl_FragCoord.xy / iResolution.xy - 0.5) * 2.0;
    uv.x *= iResolution.x / iResolution.y;

    float zoom = 2.0 + sin(iTime * 0.3) * 0.8;
    uv /= zoom;

    float t1 = iTime * 0.3;
    float t2 = iTime * 0.4;
    vec2 c = vec2(
        0.38 + 0.4 * sin(t1) * cos(t2),
        0.28 + 0.4 * cos(t1) * sin(t2)
    );

    vec2 z = uv;
    int iterations = 0;
    const int iter = 100;

    float minDist = 1000.0;
    float maxDist = 0.0;

    for(int i = 0; i < iter; i++) {
        float x = z.x * z.x - z.y * z.y;
        float y = 2.0 * z.x * z.y;

        float variation = 1.0 + 0.2 * sin(iTime * 0.5 + length(z));
        z = vec2(x, y) * variation + c;

        float dist = length(z);
        minDist = min(minDist, dist);
        maxDist = max(maxDist, dist);

        if(dist > 4.0) break;
        iterations++;
    }

    float smooth_iter = float(iter) + 1.0 - log2(log2(length(z)));
    float t = smooth_iter / float(iter);

    float colorVar = (t * 0.7 + minDist * 0.3) * (1.0 + 0.2 * sin(iTime));

    vec3 color = palette(
        colorVar,
        vec3(0.1, 0.1, 0.2),
        vec3(0.9, 0.8, 0.7),
        vec3(0.8, 0.7, 0.6),
        vec3(0.15, 0.25, 0.35)
    );

    color *= 1.0 + 0.1 * sin(length(z) * 5.0 + iTime);

    float glow = 1.0 - (minDist / maxDist);
    color += vec3(0.1, 0.2, 0.4) * glow * glow;

    FragColor = vec4(color, 1.0);
}
