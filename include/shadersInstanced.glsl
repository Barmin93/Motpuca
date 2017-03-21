[vertex]=================================================================

in vec3 position;
in vec3 normal;
in vec3 instancePosition;
in float instanceRadius;
in vec3 instanceColor;

uniform mat4 pv_matrix;

out vec3 fragNormal;
out vec3 fragColor;

void main()
{
    mat4 m_matrix = mat4(instanceRadius, 0.0, 0.0, 0.0,
                         0.0, instanceRadius, 0.0, 0.0,
                         0.0, 0.0, instanceRadius, 0.0,
                         instancePosition.x, instancePosition.y, instancePosition.z, 1.0);

    mat4 pvm_matrix = pv_matrix*m_matrix;

    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragColor = instanceColor;
    fragNormal = normalize(mat3(transpose(inverse(m_matrix)))*normal);
}



[fragment]===============================================================

in vec3 fragNormal;
in vec3 fragColor;

uniform float ambient;
uniform float diffuse;
uniform vec3 lightDir;

out vec4 color;

void main()
{
    color = vec4(fragColor*(ambient + max(dot(fragNormal, lightDir), 0.0)*diffuse), 1.0);
}
