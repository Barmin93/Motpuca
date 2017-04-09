[vertex]=================================================================

in vec3 position;
in vec3 normal;

uniform mat4 pvm_matrix;
uniform mat3 norm_matrix;

out vec3 fragNormal;

void main()
{
    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragNormal = normalize(norm_matrix*normal);
}



[fragment]===============================================================

in vec3 fragNormal;

uniform float ambient;
uniform float diffuse;
uniform vec3 lightDir;
uniform vec4 colorRGBA;

out vec4 color;

void main()
{
  color = vec4(colorRGBA.rgb*(ambient + max(dot(fragNormal, lightDir), 0.0)*diffuse), colorRGBA.a);
}
