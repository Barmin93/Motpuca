#ifndef GLMODEL_H
#define GLMODEL_H

#include <QString>
#include <QStringList>

#include "transform.h"
#include "glshaderprogram.h"
#include "config.h"


class rGlModel
{
public:
    rGlModel(): vao(0), v_cnt(0), vn_cnt(0), f_cnt(0), l_cnt(0), vertData(0), v(0), vn(0), gl_program(0) {}
    ~rGlModel() { delete [] vertData; }


    float *getVertData() { return vertData; }
    int getTrianglesCount() { return f_cnt; }
    int getLinesCount() { return l_cnt; }
    int getVertDataSize() { return 3*6*f_cnt*sizeof(float); }
    int getVertDataLineSize() { return 2*4*l_cnt*sizeof(float); }
    void setupMatrices(anyTransform const &m_matrix);

    void init(class GLWidget *glWidget, rGlShaderProgram &program, QString fname, float scale = 1.0)
    {
        this->widget = glWidget;
        this->gl_program = &program;
        load_from_file(QString("%1include/%2").arg(GlobalSettings.app_dir).arg(fname), scale);
        if (l_cnt > 0)
            make_vao_lines();
        else
            make_vao(vn_cnt > 0);
    }

    unsigned int vbo, vao;

private:
    GLWidget *widget;
    QStringList source;
    int v_cnt, vn_cnt, f_cnt, l_cnt;
    float *vertData;
    float *v, *vn;

    rGlShaderProgram *gl_program;

    void load_from_file(QString fname, float scale = 1.0);
    void make_vao(bool normals);
    void make_vao_lines();
    void count_items();
    void alloc_items();
    void parse_items(float scale);
    void generate_tangents();

public:
    void print();
    rGlShaderProgram *program() { return gl_program; }
    void draw(bool bind = true);
    void drawInstanced(int instances, bool bind = true);
};

#endif // GLMODEL_H
