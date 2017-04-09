[vertex]=================================================================

in vec3 position;
in float alpha;

uniform mat4 pvm_matrix;

out float fragAlpha;

void main()
{
    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragAlpha = alpha;
}



[fragment]===============================================================

in float fragAlpha;
uniform vec3 colorRGB;
out vec4 color;

void main()
{
  color = vec4(colorRGB, fragAlpha);
}
