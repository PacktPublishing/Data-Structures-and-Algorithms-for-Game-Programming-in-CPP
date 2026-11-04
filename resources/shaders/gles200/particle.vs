#version 100

// Attributes from the buffer (position, velocity, lifetime)
attribute vec3 vertexPosition;
attribute vec3 vertexVelocity;
attribute float vertexLifetime;

// Uniform for time passed since start
uniform float time;

varying float lifetime;  // Pass lifetime to fragment shader

void main() {
    // Update position based on velocity and time (simple motion with gravity)
    float age = mod(time, vertexLifetime);  // Reset particle after lifetime
    vec3 position = vertexPosition + vertexVelocity * age - vec3(0.0, 0.5 * age * age, 0.0);

    // Set the updated vertex position
    gl_Position = vec4(position, 1.0);
    lifetime = age / vertexLifetime;  // Normalize lifetime for use in the fragment shader
}
