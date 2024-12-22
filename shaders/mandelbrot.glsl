#version 330 core
uniform float iTime;
uniform vec2 iResolution;
out vec4 FragColor;

vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b * cos(6.283185 * (c * t + d));
}

void main() {
    // Normalize and center the coordinates
    vec2 uv = (gl_FragCoord.xy / iResolution.xy - 0.5) * 2.0;
    uv.x *= iResolution.x / iResolution.y;

    // Create a cycle for the zoom (15 seconds in, 10 seconds out)
    float cycleDurationIn = 12.0;
    float cycleDurationOut = 10.0;
    float totalCycle = cycleDurationIn + cycleDurationOut;
    float cycleTime = mod(iTime, totalCycle);
    float zoomFactor;

    if (cycleTime < cycleDurationIn) {
        zoomFactor = pow(1.5, cycleTime * 2.0);
    } else {
        float t = (cycleTime - cycleDurationIn) / cycleDurationOut; // Normalize to 0-1
        zoomFactor = pow(1.5, (cycleDurationIn - t * cycleDurationIn) * 2.0);
    }

    // Hand picked target position
    const vec2 target = vec2(-0.743643887037158704752191506114774, 0.131825904205311970493132056385139);

    // Calculate position
    vec2 c = uv / zoomFactor + target;
    vec2 z = vec2(0.0);

    // Iteration and escape condition
    int iterations = 0;
    const int maxIterations = 100;

    for(int i = 0; i < maxIterations; i++) {
        // Mandelbrot equation: z = z^2 + c
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if(length(z) > 4.0) break;
        iterations = i;
    }

    // Smooth coloring calculation
    float smoothIteration = float(iterations) + 1.0 - log(log(length(z))) / log(2.0);
    float t = smoothIteration / float(maxIterations);

    vec3 color = palette(
        t,
        vec3(0.2, 0.1, 0.3),
        vec3(0.6, 0.4, 0.5),
        vec3(1.0, 0.9, 0.8),
        vec3(0.3, 0.5, 0.7)
    );

    FragColor = vec4(color, 1.0);
}
