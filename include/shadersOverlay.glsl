[vertex]=================================================================

in vec3 position;

uniform vec2 scale;
uniform vec2 move;

out vec2 fragTexCoor;

void main()
{
    gl_Position = vec4(position.x*scale.x, position.y*scale.y, position.z, 1.0)
                + vec4(move, 0.0, 0.0);

    fragTexCoor = vec2(position.x*0.5 + 0.5, 0.5 - position.y*0.5);
}



[fragment]===============================================================

in vec2 fragTexCoor;

uniform sampler2D tex;
uniform vec3 navColor;
uniform vec3 intColor;

out vec4 color;

void main()
{
    color = texture(tex, fragTexCoor).rgba;
    color.a *= 0.75;
    if (color.r > 0.95 && color.g < 0.05 && color.b < 0.05)
        color = vec4(intColor, color.a);
    else if (color.r > 0.5 && color.g > 0.5 && color.b > 0.5)
        color = vec4(navColor, color.a);
}
