#ifndef GLSHADERPROGRAM_H
#define GLSHADERPROGRAM_H

#include "openglstaff.h"
#include "transform.h"
#include "color.h"


class rGlShaderProgram
{
private:
    class GLWidget *widget;

public:
    GLuint program;

    rGlShaderProgram(): program(0) { }

    void init(GLWidget *glWidget, QString shaderfname);

    GLuint getUniformLocation(const GLchar *uniform);
    GLuint getAttribLocation(const GLchar *attrib);

    void use();
    void setUniformMatrix4x4(const GLchar *uniform_name, const anyTransform &matrix);
    void setUniformMatrix3x3(const GLchar *uniform_name, const anyTransform &matrix);
    void setUniformColorRGB(const GLchar *uniform_name, const anyColor &color);
    void setUniformColorRGBA(const GLchar *uniform_name, const anyColor &color);
    void setUniformFloat(const GLchar *uniform_name, float x);
    void setUniformFloat2(const GLchar *uniform_name, float x, float y);
    void setUniformVector(const GLchar *uniform_name, const anyVector &vector);
    void setAttrPointer(const GLchar *attr_name, int size, int stride, int offset);
    void setTexture(const GLchar *uniform_name, GLuint slot, GLuint tex);
};


#endif // GLSHADERPROGRAM_H
