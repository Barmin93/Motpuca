#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "glmodel.h"
#include "log.h"
#include "glwidget.h"


void rGlModel::load_from_file(QString fname, float scale)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new Error(__FILE__, __LINE__, "Cannot open file", fname.toLatin1().constData());

    QTextStream ts(&file);
    source.clear();
    while (!ts.atEnd())
        source << ts.readLine();
    file.close();

    count_items();
    alloc_items();
    parse_items(scale);
}


void rGlModel::count_items()
{
    v_cnt = vn_cnt = f_cnt = 0;
    for (int i = 0; i < source.count(); i++)
    {
        if (source[i].startsWith("v "))
            v_cnt++;
        else if (source[i].startsWith("vn "))
            vn_cnt++;
        else if (source[i].startsWith("f "))
            f_cnt++;
        else if (source[i].startsWith("l "))
            l_cnt++;
    }
}


void rGlModel::alloc_items()
{
    v = new float[3*v_cnt];
    memset(v, 0, sizeof(float)*3*v_cnt);
    vn = new float[3*vn_cnt];
    memset(vn, 0, sizeof(float)*3*vn_cnt);

    if (l_cnt)
        vertData = new float[2*4*l_cnt]; // line segment: 2*(x, y, z, alpha)
    else
        vertData = new float[3*6*f_cnt]; // triangle: 3*(x, y, z, nx, ny, nz)
}


void rGlModel::parse_items(float scale)
{
    QString l;
    QStringList sl, sl2;

    // wierzcholki...
    int p = 0;
    for (int i = 0; i < source.count(); i++)
    {
        if (source[i].startsWith("v "))
        {
            l = source[i].mid(2).trimmed();
            sl = l.split(" ");
            v[3*p + 0] = sl[0].toFloat()*scale;
            v[3*p + 1] = sl[1].toFloat()*scale;
            v[3*p + 2] = sl[2].toFloat()*scale;
            p++;
        }
    }

    // normalne...
    p = 0;
    for (int i = 0; i < source.count(); i++)
    {
        if (source[i].startsWith("vn "))
        {
            l = source[i].mid(3).trimmed();
            sl = l.split(" ");
            vn[3*p + 0] = sl[0].toFloat();
            vn[3*p + 1] = sl[1].toFloat();
            vn[3*p + 2] = sl[2].toFloat();
            p++;
        }
    }


    // trojkaty...
    p = 0;
    for (int i = 0; i < source.count(); i++)
    {
        if (source[i].startsWith("f "))
        {
            l = source[i].mid(2).trimmed();
            sl = l.split(" ");

            for (int j = 0; j < 3; j++)
            {
                sl2 = sl[j].split("/");
                while (sl2.count() < 3)
                    sl2.append("");

                int vi = sl2[0].toInt() - 1;

                vertData[6*p + 0] = v[3*vi + 0];
                vertData[6*p + 1] = v[3*vi + 1];
                vertData[6*p + 2] = v[3*vi + 2];

                int vni = sl2[2].toInt() - 1;
                vertData[6*p + 3] = vn[3*vni + 0];
                vertData[6*p + 4] = vn[3*vni + 1];
                vertData[6*p + 5] = vn[3*vni + 2];\

                p++;
            }
        }
    }

    // linie...
    p = 0;
    for (int i = 0; i < source.count(); i++)
    {
        if (source[i].startsWith("l "))
        {
            l = source[i].mid(2).trimmed();
            sl = l.split(" ");

            for (int j = 0; j < 2; j++)
            {
                sl2 = sl[j].split("/");
                if (sl2.count() == 1)
                    sl2.append("1");

                int vi = sl2[0].toInt() - 1;

                vertData[4*p + 0] = v[3*vi + 0];
                vertData[4*p + 1] = v[3*vi + 1];
                vertData[4*p + 2] = v[3*vi + 2];

                float alpha = sl2[1].toFloat();
                vertData[4*p + 3] = alpha;

                p++;
            }
        }
    }


    delete [] v;
    delete [] vn;
}


void rGlModel::make_vao(bool normals)
{
    widget->glGenVertexArrays(1, &vao);
    widget->glBindVertexArray(vao);

    // vbo...
    GLuint vbo;
    widget->glGenBuffers(1, &vbo);
    widget->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    widget->glBufferData(GL_ARRAY_BUFFER, getVertDataSize(), getVertData(), GL_STATIC_DRAW);

    // vertices...
    GLint a = gl_program->getAttribLocation("position");
    widget->glVertexAttribPointer(a, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 0);
    widget->glEnableVertexAttribArray(a);

    // normals...
    if (normals)
    {
        a = gl_program->getAttribLocation("normal");
        widget->glVertexAttribPointer(a, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        widget->glEnableVertexAttribArray(a);
    }

    widget->glBindVertexArray(0);
}


void rGlModel::make_vao_lines()
{
    widget->glGenVertexArrays(1, &vao);
    widget->glBindVertexArray(vao);

    // vbo...
    GLuint vbo;
    widget->glGenBuffers(1, &vbo);
    widget->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    widget->glBufferData(GL_ARRAY_BUFFER, getVertDataLineSize(), getVertData(), GL_STATIC_DRAW);

    // vertices...
    GLint a = gl_program->getAttribLocation("position");
    widget->glVertexAttribPointer(a, 3, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    widget->glEnableVertexAttribArray(a);

    // alphas...
    a = gl_program->getAttribLocation("alpha");
    widget->glVertexAttribPointer(a, 1, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
    widget->glEnableVertexAttribArray(a);

    widget->glBindVertexArray(0);
}


void rGlModel::print()
{
    for (int i = 0; i < f_cnt; i++)
    {
        QString s = "";
        for (int j = 0; j < 6*3; j++)
            s += (j ? ", ": "") + QString::number(vertData[i*6 + j]);
        qDebug() << s;
    }
}


void rGlModel::draw(bool bind)
{
    if (bind) widget->glBindVertexArray(vao);
    if (l_cnt > 0)
        widget->glDrawArrays(GL_LINES, 0, 2*l_cnt);
    else
        widget->glDrawArrays(GL_TRIANGLES, 0, 3*f_cnt);
    if (bind) widget->glBindVertexArray(0);
}


void rGlModel::drawInstanced(int instances, bool bind)
{
    if (bind) widget->glBindVertexArray(vao);
    if (l_cnt > 0)
        widget->glDrawArraysInstanced(GL_LINES, 0, 2*l_cnt, instances);
    else
        widget->glDrawArraysInstanced(GL_TRIANGLES, 0, 3*f_cnt, instances);
    if (bind) widget->glBindVertexArray(0);
}

