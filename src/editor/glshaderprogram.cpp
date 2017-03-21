#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "glshaderprogram.h"
#include "config.h"
#include "glwidget.h"


void rGlShaderProgram::init(GLWidget *glWidget, QString shaderfname)
{
    widget = glWidget;

    // shaders collection...
    QString shader_source[5];
    GLenum shader_enum[5] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER};
    QString fname = QString("%1include/%2").arg(GlobalSettings.app_dir).arg(shaderfname);
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new Error(__FILE__, __LINE__, "Cannot open shader file.", fname.toLatin1().constData());
    QTextStream fstr(&f);
    int shader_index = -1;
    while(!fstr.atEnd())
    {
        QString l = fstr.readLine();
        if (l.startsWith("[vertex]"))
            shader_index = 0;
        else if (l.startsWith("[fragment]"))
            shader_index = 1;
        else if (l.startsWith("[geometry]"))
            shader_index = 2;
        else if (l.startsWith("[tess control]"))
            shader_index = 3;
        else if (l.startsWith("[tess eval]"))
            shader_index = 4;
        else if (l.trimmed().length() > 0 && shader_index != -1)
            shader_source[shader_index] += l + "\n";
    }
    f.close();

    // shaders compilation...
    GLint success;
    program = widget->glCreateProgram();

    for (int i = 0; i < 5; i++)
    {
        if (shader_source[i].length()> 0)
        {
            shader_source[i] = "#version " SHADER_VERSION "\n\n" + shader_source[i];

            GLuint shader_id = widget->glCreateShader(shader_enum[i]);
            GLchar *s = new GLchar[shader_source[i].length() + 1];
            for (int j = 0; j < shader_source[i].length(); j++)
                s[j] = shader_source[i].at(j).toLatin1();
            s[shader_source[i].length()] = 0;
            widget->glShaderSource(shader_id, 1, &s, NULL);
            delete s;
            widget->glCompileShader(shader_id);

            // ok?
            widget->glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                GLchar infoLog[512];
                widget->glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
                throw new Error(__FILE__, __LINE__, "Shader error", infoLog);
            }
            widget->glAttachShader(program, shader_id);
        }
    }

    // shaders linking...
    widget->glLinkProgram(program);
    widget->glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        widget->glGetProgramInfoLog(program, 512, NULL, infoLog);
        throw new Error(__FILE__, __LINE__, "Shader linking error", infoLog);
    }

    f.close();
}


GLuint rGlShaderProgram::getUniformLocation(const GLchar *uniform)
{
    int a = widget->glGetUniformLocation(program, uniform);
    if (a < 0) throw new Error(__FILE__, __LINE__, "Wrong shader uniform", uniform);
    return a;
}


GLuint rGlShaderProgram::getAttribLocation(const GLchar *attrib)
{
    int a = widget->glGetAttribLocation(program, attrib);
    if (a < 0) throw new Error(__FILE__, __LINE__, "Wrong shader attrib", attrib);
    return a;
}


void rGlShaderProgram::use()
{
    widget->glUseProgram(program);
}


void rGlShaderProgram::setUniformMatrix4x4(const GLchar *uniform_name, const anyTransform &matrix)
{
    int a = getUniformLocation(uniform_name);
    widget->glUniformMatrix4fv(a, 1, GL_FALSE, matrix.matrix);
}


void rGlShaderProgram::setUniformMatrix3x3(const GLchar *uniform_name, const anyTransform &matrix)
{
    int a = getUniformLocation(uniform_name);
    widget->glUniformMatrix3fv(a, 1, GL_FALSE, matrix.matrix3x3());
}


void rGlShaderProgram::setUniformColorRGB(const GLchar *uniform_name, const anyColor &color)
{
    int a = getUniformLocation(uniform_name);
    widget->glUniform3fv(a, 1, color.rgba_array);
}


void rGlShaderProgram::setUniformColorRGBA(const GLchar *uniform_name, const anyColor &color)
{
    int a = getUniformLocation(uniform_name);
    widget->glUniform4fv(a, 1, color.rgba_array);
}


void rGlShaderProgram::setUniformFloat(const GLchar *uniform_name, float x)
{
    int a = getUniformLocation(uniform_name);
    widget->glUniform1f(a, x);
}


void rGlShaderProgram::setUniformFloat2(const GLchar *uniform_name, float x, float y)
{
    int a = getUniformLocation(uniform_name);
    widget->glUniform2f(a, x, y);
}


void rGlShaderProgram::setUniformVector(const GLchar *uniform_name, const anyVector &vector)
{
    int a = getUniformLocation(uniform_name);
    widget->glUniform3f(a, vector.x, vector.y, vector.z);
}


void rGlShaderProgram::setAttrPointer(const GLchar *attr_name, int size, int stride, int offset)
{
    int a = getAttribLocation(attr_name);
    widget->glVertexAttribPointer(a, size, GL_FLOAT, GL_FALSE, stride, (void *)(offset*sizeof(GLfloat)));
    widget->glVertexAttribDivisor(a, 1);
    widget->glEnableVertexAttribArray(a);
}


void rGlShaderProgram::setTexture(const GLchar *uniform_name, GLuint slot, GLuint tex)
{
    widget->glActiveTexture(GL_TEXTURE0 + slot);
    widget->glBindTexture(GL_TEXTURE_2D, tex);
    int a = getUniformLocation(uniform_name);
    widget->glUniform1i(a, slot);
}
