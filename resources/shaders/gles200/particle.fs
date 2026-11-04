#version 100

precision mediump float;

varying float lifetime;       // Normalized lifetime from vertex shader

void main() {
    // Set color based on lifetime (fade out as it reaches end of life)
    vec3 color = mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 0.0, 0.0), lifetime);  // Color shift
    float alpha = 1.0 - lifetime;  // Fade out
    gl_FragColor = vec4(color, alpha);
}
