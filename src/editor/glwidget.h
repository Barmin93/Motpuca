#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWheelEvent>
#include <QMouseEvent>

#include <QOpenGLWidget>

#include "openglstaff.h"

#include "config.h"
#include "log.h"
#include "glmodel.h"
#include "glshaderprogram.h"
#include "mainwindow.h"


class OpenGLVersionTest: public QOpenGLFunctions_1_0
{
public:
    QString version()
    {
        initializeOpenGLFunctions();
        return (char *)glGetString(GL_VERSION);
    }
};


struct anyCellInstance
{
    float x, y, z;
    float radius;
    float red, green, blue;
};


class GLWidget: public QOpenGLWidget, public QOPENGLFUNCTIONS
{
private:
    int mouse_last_x;
    int mouse_last_y;
    int mouse_press_x;
    int mouse_press_y;
    bool mouse_pressed;
    int mouse_buttons;
    int widget_width;
    int widget_height;
    float widget_ratio;

    float pressure_min;
    float pressure_max;

    bool opengl_ok;
    bool drawing;

    // programs...
    rGlShaderProgram shaderSimple;
    rGlShaderProgram shaderInstanced;
    rGlShaderProgram shaderOverlay;
    rGlShaderProgram shaderWireframe;

    // models...
    rGlModel sphereModelSimple;
    rGlModel sphereModelNice;
    rGlModel sphereModelNicest;
    rGlModel sphereModelTube;
    rGlModel overlaySquare;
    rGlModel boxModel;
    rGlModel arrowModel;
    rGlModel cylinderModel;
    rGlModel clipModel;

    // textures...
    GLuint texNavigatorRotate;
    GLuint texNavigatorMove;
    GLuint texLegendItem;

    // instance cell drawing...
    GLuint cell_instance_vbo;
    anyCellInstance *cell_instance;
    int cell_instance_cnt;
    int cell_instance_to_draw_cnt;


public:
    GLWidget(QWidget *parent): QOpenGLWidget(parent), mouse_pressed(false), mouse_buttons(0),
        pressure_min(0), pressure_max(0), opengl_ok(true), drawing(false), cell_instance(0), cell_instance_cnt(0)
    {
        QSurfaceFormat sfm = QSurfaceFormat::defaultFormat();
        sfm.setSamples(4);
        setFormat(sfm);
    }
    bool is_mouse_pressed() { return mouse_pressed; }

    void user_scene_zoom(double zoom);
    void user_scene_move(anyVector d);
    void user_scene_rotate(anyVector d);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void readShaders(GLuint &program, QString shaderfname);
    GLuint loadTexture(QString fname);

    void paintScene(float eye_shift);
    void draw_comp_box();
    void draw_barrier(anyBarrier const *b, bool selected);
    void draw_all_barriers();
    void draw_all_boxes(bool clip);
    void draw_cell_block(anyCellBlock const *b, bool selected);
    void draw_all_cell_blocks();
    void draw_cell(anyCell const *c, float p_min, float p_max);
    void draw_all_cells(bool clip);
    void draw_cylinder(anyVector const &p1, anyVector const &p2, float r, const anyColor &color);
    void draw_tube(anyTube *v);
    void draw_all_tubes();
    void draw_axes();
    void draw_navigator();
    void draw_legend_item(int pos, anyColor const &color, char const *label);
    void draw_legend_caption(int pos, QString label);
    void draw_legend();
    void draw_clipping_plane();
    void draw_tube_line(anyTubeLine const *vl, bool selected);
    void draw_all_tube_lines();
    void draw_tube_bundle(anyTubeBundle const *vb, bool selected);
    void draw_all_tube_bundles();
    void draw_grilled_box(anyTransform const &trans, anyVector const &from, anyVector const &to, anyColor const &color);
    void draw_string(int x, int y, QString const s, int font_size, const anyColor &color);

    void selection_init();
    void selection_start();
    void selection_finish();

    void scene_move_or_rotate(QMouseEvent* event);
    void selected_object_move(QMouseEvent* event);
    void clip_plane_move(QMouseEvent* event);
    void user_scene_zoom(QWheelEvent* event);
    void clip_plane_move_z(QWheelEvent* event);

    void make_mouse_matrix(int x, int y, Qt::MouseButtons buttons, anyTransform &trans);

    void wheelEvent(QWheelEvent* event)
    /**
      Mouse wheel event (scaling/zooming).
    */
    {
        if (event->modifiers() & Qt::ShiftModifier)
            clip_plane_move_z(event);
        else if (MainWindowPtr->selected_object && (event->modifiers() & Qt::ControlModifier))
        {
            MainWindowPtr->selected_object->scale_event(event->delta(), MainWindowPtr->X_checked(), MainWindowPtr->Y_checked(), MainWindowPtr->Z_checked());
            MainWindowPtr->display_properties();
            scene::UpdateSimulationBox();
            SaveNeeded(true);
        }
        else
            user_scene_zoom(event);

        repaint();
    }

    void mousePressEvent(QMouseEvent* event)
    {
        mouse_last_x = mouse_press_x = event->pos().x() - size().width()/2;
        mouse_last_y = mouse_press_y = -(event->pos().y() - size().height()/2);
        mouse_pressed = true;
        mouse_buttons = event->buttons();
        repaint();
    }

    void mouseReleaseEvent(QMouseEvent*)
    {
        mouse_pressed = false;
        mouse_buttons = 0;
        repaint();
    }

    void mouseMoveEvent(QMouseEvent* event)
    /**
      Mouse move event.
    */
    {
        int x = event->pos().x() - size().width()/2;
        int y = -(event->pos().y() - size().height()/2);
        mouse_buttons = event->buttons();

        if (event->modifiers() & Qt::ShiftModifier)
            clip_plane_move(event);
        else if (MainWindowPtr->selected_object && (event->modifiers() & Qt::ControlModifier))
            selected_object_move(event);
        else
            scene_move_or_rotate(event);

        mouse_last_x = x;
        mouse_last_y = y;

        repaint();
    }

};




#endif // GLWIDGET_H
