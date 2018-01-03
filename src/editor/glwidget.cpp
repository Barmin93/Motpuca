#include <QMatrix4x4>
#include <QTime>

#include "mainwindow.h"
#include "glwidget.h"
#include "config.h"
#include "color.h"
#include "glmodel.h"

#include "anytube.h"
#include "anycellblock.h"
#include "anybarrier.h"
#include "anytubebundle.h"
#include "anytubeline.h"

void GLWidget::initializeGL()
/**
  OpenGL initialization.
*/
{
    if (opengl_ok)
        try
    {
        OpenGLVersionTest test;
        QString version = test.version();
        if (version < MIN_OPENGL_VERSION)
            throw new Error(__FILE__, __LINE__, "Wrong OpenGL version", version.toLatin1().constData());

        initializeOpenGLFunctions();

        qDebug() << (char *)glGetString(GL_RENDERER);
        qDebug() << (char *)glGetString(GL_VERSION);
        qDebug() << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

        // shaders...
        shaderSimple.init(this, "shadersSimple.glsl");
        shaderInstanced.init(this, "shadersInstanced.glsl");
        shaderOverlay.init(this, "shadersOverlay.glsl");
        shaderWireframe.init(this, "shadersWireframe.glsl");

        // models...
        sphereModelSimple.init(this, shaderInstanced, "sphereSimple.model");
        sphereModelNice.init(this, shaderInstanced, "sphereNice.model");
        sphereModelNicest.init(this, shaderInstanced, "sphereNicest.model");
        sphereModelTube.init(this, shaderSimple, "sphereNice.model");
        arrowModel.init(this, shaderSimple, "arrow.model");
        cylinderModel.init(this, shaderSimple, "cylinder.model");
        boxModel.init(this, shaderWireframe, "box.model");
        clipModel.init(this, shaderWireframe, "clip.model");
        overlaySquare.init(this, shaderOverlay, "overlaySquare.model");

        // textures...
        texNavigatorRotate = loadTexture("navigatorRotate.png");
        texNavigatorMove = loadTexture("navigatorMove.png");
        texLegendItem = loadTexture("legendItem.png");

        // font...
//        loadFont("chars.font");

        // cell instance vbo...
        glGenBuffers(1, &cell_instance_vbo);

        opengl_ok = true;
    }
    catch (Error *err)
    {
        opengl_ok = false;
        LogError(err);
    }
}


GLuint GLWidget::loadTexture(QString fname)
{
    QString fn = QString("%1include/%2").arg(GlobalSettings.app_dir).arg(fname);
    QImage image(fn);
    if (image.width() < 1) throw new Error(__FILE__, __LINE__, "Cannot open file", fn.toLatin1());
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}


void GLWidget::resizeGL(int w, int h)
{
    widget_width = w;
    widget_height = h;
    if (h > 0)
        widget_ratio = (float)w/(float)h;
    else
        widget_ratio = 1.0;
}


void GLWidget::paintGL()
{
    if (drawing) return;
    drawing = true;

    QTime t;
    t.start();

    MainWindowPtr->set_run_repaint(true);

    if (!opengl_ok)
    {
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    else
    try
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glLineWidth(1);

        glClearColor(VisualSettings.bkg_color.r(),
                     VisualSettings.bkg_color.g(),
                     VisualSettings.bkg_color.b(),
                     VisualSettings.bkg_color.a());

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        paintScene(0.0);

        if (glGetError() != GL_NO_ERROR)
            throw new Error(__FILE__, __LINE__, "OpenGL Error");
    }
    catch (Error *err)
    {
        opengl_ok = false;
        LogError(err);
    }

    drawing = false;
    MainWindowPtr->set_run_repaint(false);
}


void GLWidget::paintScene(float /*eye_shift*/)
{
    anyVector comp_box_size = SimulationSettings.comp_box_to - SimulationSettings.comp_box_from;
    float max_comp_box_size = 2.0*qMax(qMax(comp_box_size.x,comp_box_size.y), comp_box_size.z);

    // prespective matrix...
    VisualSettings.p_matrix.setToIdentity();
    float dist = 200;

    // zoom factor...
    float dl = (VisualSettings.v_matrix*anyVector(1, 0, 0)).length();

    VisualSettings.p_matrix.setToPerspective(45, widget_ratio, 0.01f, max_comp_box_size + dl*dist);
    VisualSettings.p_matrix.translate(anyVector(0, 0, -max_comp_box_size*0.5 - dist));

    if (MainWindowPtr->get_show_elements(SHOW_AXIS))
        draw_axes();

    if (MainWindowPtr->get_show_elements(SHOW_TUBES))
        draw_all_tubes();

    if (MainWindowPtr->get_show_elements(SHOW_TUMOR + SHOW_NORMAL))
        draw_all_cells(MainWindowPtr->get_show_elements(SHOW_CLIPPING));

    if (MainWindowPtr->get_show_elements(SHOW_BLOCKS))
    {
        draw_all_cell_blocks();
        draw_all_tube_lines();
        draw_all_tube_bundles();
    }

    if (MainWindowPtr->get_show_elements(SHOW_BOXES))
        draw_all_boxes(MainWindowPtr->get_show_elements(SHOW_CLIPPING));

    if (MainWindowPtr->get_show_elements(SHOW_BARRIERS))
        draw_all_barriers();

    if (MainWindowPtr->get_show_elements(SHOW_COMPBOX))
        draw_comp_box();

    if (MainWindowPtr->get_show_elements(SHOW_CLIPPING_PLANE) && MainWindowPtr->get_show_elements(SHOW_CLIPPING))
        draw_clipping_plane();

    if (MainWindowPtr->get_show_elements(SHOW_LEGEND))
        draw_legend();

    if (mouse_pressed)
        draw_navigator();
}



void GLWidget::draw_comp_box()
/**
  Draws outline of computational box.
*/
{
    anyTransform dummy;
    draw_grilled_box(dummy, SimulationSettings.comp_box_from, SimulationSettings.comp_box_to, VisualSettings.comp_box_color);
}


void GLWidget::draw_barrier(anyBarrier const *b, bool selected)
/**
  Draws barrier.
*/
{

    anyVector size_mod(1, 1, 1);
    if (b->type == sat::btOut)
        size_mod = size_mod*-1;

    anyTransform dummy;
    if (selected)
    {
        glLineWidth(3);
        draw_grilled_box(dummy, b->from - size_mod, b->to + size_mod, VisualSettings.selection_color);
    }
    else
    {
        glLineWidth(1);
        draw_grilled_box(dummy, b->from - size_mod, b->to + size_mod, b->type == sat::btIn ? VisualSettings.in_barrier_color : VisualSettings.out_barrier_color);
    }

    glLineWidth(1);
}


void GLWidget::draw_all_barriers()
/**
  Draws all barriers.
*/
{
    anyBarrier *b = scene::FirstBarrier;
    while (b)
    {
        draw_barrier(b, MainWindowPtr->is_object_selected(b));
        b = (anyBarrier *)b->next;
    }
}


void GLWidget::draw_cell_block(anyCellBlock const *b, bool selected)
/**
  Draws block of cells - if cells are already generated, color is blended.
*/
{
    if (selected)
    {
        glLineWidth(3);
        draw_grilled_box(b->trans, b->from, b->to, VisualSettings.selection_color);
    }
    else
    {
        glLineWidth(1);
        draw_grilled_box(b->trans, b->from, b->to, b->tissue->color);
    }

}


void GLWidget::draw_all_cell_blocks()
/**
  Draws boundary of all cell blocks.
*/
{
    anyCellBlock *b = scene::FirstCellBlock;
    while (b)
    {
        draw_cell_block(b, MainWindowPtr->is_object_selected(b));
        b = (anyCellBlock *)b->next;
    }
}


void GLWidget::draw_tube_line(anyTubeLine const*, bool)
/**
  Draws tube line - if tubes are already generated, color is blended.
*/
{
    //@@@
}


void GLWidget::draw_tube_bundle(anyTubeBundle const *, bool)
/**
  Draws tube bundle - if tube lines are already generated, color is blended.
*/
{
    //@@@
}


void GLWidget::draw_all_tube_lines()
/**
  Draws boundary of all tube lines.
*/
{
    anyTubeLine *vl = scene::FirstTubeLine;
    while (vl)
    {
        draw_tube_line(vl, MainWindowPtr->is_object_selected(vl));
        vl = (anyTubeLine *)vl->next;
    }
}


void GLWidget::draw_all_tube_bundles()
/**
  Draws boundary of all tube bundles.
*/
{
    anyTubeBundle *vb = scene::FirstTubeBundle;
    while (vb)
    {
        draw_tube_bundle(vb, MainWindowPtr->is_object_selected(vb));
        vb = (anyTubeBundle *)vb->next;
    }
}


void GLWidget::draw_grilled_box(const anyTransform &trans, const anyVector &from, const anyVector &to, const anyColor &color)
{
    anyTransform m_matrix;
    anyVector scale = (to - from)*0.5;
    m_matrix.translate((to + from)*0.5);
    m_matrix.scale(scale);
    m_matrix = m_matrix*trans;

    boxModel.program()->use();
    boxModel.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    boxModel.program()->setUniformColorRGB("colorRGB", color);
    boxModel.draw();
}


void GLWidget::draw_all_boxes(bool clip)
/**
  Draws boundary of all non-empty boxes.
*/
{
    //char hstr[1000];
    int first_cell = 0;
    int box_id = 0;
    anyTransform dummy;
    for (int box_z = 0; box_z < SimulationSettings.no_boxes_z; box_z++)
        for (int box_y = 0; box_y < SimulationSettings.no_boxes_y; box_y++)
            for (int box_x = 0; box_x < SimulationSettings.no_boxes_x; box_x++, box_id++)
            {
                int no_cells = scene::Cells[first_cell].no_cells_in_box + 1;
                anyVector box_center = anyVector(box_x, box_y, box_z)*SimulationSettings.box_size + SimulationSettings.comp_box_from +
                        anyVector(SimulationSettings.box_size/2, SimulationSettings.box_size/2, SimulationSettings.box_size/2);

                if (no_cells &&
                        (!clip ||
                         box_center.x*VisualSettings.clip_plane[0] +
                         box_center.y*VisualSettings.clip_plane[1] +
                         box_center.z*VisualSettings.clip_plane[2] +
                         VisualSettings.clip_plane[3] > 0))
                {
                    draw_grilled_box(dummy,
                                     anyVector(box_x + 0.05, box_y + 0.05, box_z + 0.05)*SimulationSettings.box_size + SimulationSettings.comp_box_from,
                                     anyVector(box_x + 0.95, box_y + 0.95, box_z + 0.95)*SimulationSettings.box_size + SimulationSettings.comp_box_from,
                                     VisualSettings.boxes_color);
                    //snprintf(hstr, 100, "%d (%s)", no_cells, anyVector(box_x, box_y, box_z).to_string());
                    //rglDrawStringOrtho(hstr, anyVector(box_x + 0.05, box_y + 1, box_z + 1)*SimulationSettings.box_size + SimulationSettings.comp_box_from, 1, 0, 1);
                }

                first_cell += SimulationSettings.max_cells_per_box;
            }
}


void GLWidget::draw_cell(anyCell const *c, float /*p_min*/, float /*p_max*/)
/**
 Draws cell.

 \param c -- pointer to cell
 \param p_min -- minimum pressure
 \param p_max -- maximum pressure
*/
{
    if (c->tissue->type == sat::ttNormal && !MainWindowPtr->get_show_elements(SHOW_NORMAL))
        return;
    if (c->tissue->type == sat::ttTumor && !MainWindowPtr->get_show_elements(SHOW_TUMOR))
        return;

    anyColor color;
    color.set(0, 0, 0, 1, 0);
    int color_mode = MainWindowPtr->get_coloring_mode();

    // tissue color...
    if (color_mode & COLOR_MODE_TISSUE_COLOR)
        color.add(c->tissue->color);

    // state...
    if (color_mode & COLOR_MODE_STATE)
    {
        if (c->state == sat::csAlive || c->state == sat::csAdded)
            color.add(VisualSettings.cell_alive_color);
        else if (c->state == sat::csHypoxia)
            color.add(VisualSettings.cell_hypoxia_color);
        else if (c->state == sat::csApoptosis)
            color.add(VisualSettings.cell_apoptosis_color);
        else if (c->state == sat::csNecrosis)
            color.add(VisualSettings.cell_necrosis_color);
    }

    // pressure...
    if (color_mode & COLOR_MODE_PRESSURE)
    {
        float pr = c->pressure_prev;
        float p;
        if (pressure_max == pressure_min)
            color.add(0.5, 0.5, 0.5);
        else
        {
            if (pr < 0)
                p = 0;
            else if (pr > 1)
                p = 1;
            else
                p = (pr - pressure_min)/(pressure_max - pressure_min);
            if (pr > c->tissue->max_pressure)
                color.add(p, p*0.75, p*0.75);
            else
                color.add(p, p, p);
        }
    }

    // O2 concentration...
    if (color_mode & COLOR_MODE_O2)
    {
        float pr = c->concentrations[sat::dsO2][!(SimulationSettings.step % 2)];
        float p;
        if (pr < 0)
            p = 0;
        else if (pr > 1)
            p = 1;
        else
            p = pr;
        color.add(0, 0, p);
    }

    // TAF concentration...
    if (color_mode & COLOR_MODE_TAF)
    {
        float pr = c->concentrations[sat::dsTAF][!(SimulationSettings.step % 2)];
        float p;
        if (pr < 0)
            p = 0;
        else if (pr > 1)
            p = 1;
        else
            p = pr;
        color.add(0, p, 0);
    }

    // Pericytes concentration...
    if (color_mode & COLOR_MODE_PERICYTES)
    {
        float pr = c->concentrations[sat::dsPericytes][!(SimulationSettings.step % 2)];
        float p;
        if (pr < 0)
            p = 0;
        else if (pr > 1)
            p = 1;
        else
            p = pr;
        color.add(p, p, 0);
    }


    if (cell_instance_to_draw_cnt < cell_instance_cnt)
    {
        cell_instance[cell_instance_to_draw_cnt].x = c->pos.x;
        cell_instance[cell_instance_to_draw_cnt].y = c->pos.y;
        cell_instance[cell_instance_to_draw_cnt].z = c->pos.z;
        cell_instance[cell_instance_to_draw_cnt].radius = c->r;
        cell_instance[cell_instance_to_draw_cnt].red = color.r();
        cell_instance[cell_instance_to_draw_cnt].green = color.g();
        cell_instance[cell_instance_to_draw_cnt].blue = color.b();
        cell_instance_to_draw_cnt++;
    }
}


void GLWidget::draw_all_cells(bool clip)
/**
 Draws all cells.
*/
{
    if (!scene::Cells) return;

    // calculate number of cells...
    int total_no_cells = 0;
    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        total_no_cells += scene::Cells[first_cell].no_cells_in_box;
        first_cell += SimulationSettings.max_cells_per_box;
    }
    if (!total_no_cells)
        return;

    // alloc instance data...
    if (cell_instance_cnt < total_no_cells)
    {
        if (cell_instance != 0)
            delete [] cell_instance;
        int n = total_no_cells*3/2;
        cell_instance = new anyCellInstance[n];
        cell_instance_cnt = n;
    }


    first_cell = 0;
    float p_avg = 0;
    int p_cnt = 0;
    cell_instance_to_draw_cnt = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = scene::Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            // draw only active cells...
            if (scene::Cells[first_cell + i].state != sat::csRemoved)
            {
                p_avg += scene::Cells[first_cell + i].pressure_avg;
                p_cnt++;
                if (!clip ||
                        scene::Cells[first_cell + i].pos.x*VisualSettings.clip_plane[0] +
                        scene::Cells[first_cell + i].pos.y*VisualSettings.clip_plane[1] +
                        scene::Cells[first_cell + i].pos.z*VisualSettings.clip_plane[2] +
                        VisualSettings.clip_plane[3] > 0
                        )
                    draw_cell(scene::Cells + first_cell + i, pressure_min, pressure_max);
            }

        first_cell += SimulationSettings.max_cells_per_box;
    }


    shaderInstanced.use();

    glBindBuffer(GL_ARRAY_BUFFER, cell_instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(anyCellInstance)*cell_instance_to_draw_cnt, cell_instance, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    shaderInstanced.setUniformMatrix4x4("pv_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix);
    shaderInstanced.setUniformFloat("ambient", 0.2f);
    shaderInstanced.setUniformFloat("diffuse", 0.8f);
    shaderInstanced.setUniformVector("lightDir", VisualSettings.light_dir_r);

    rGlModel *sphere;
    if (MainWindowPtr->get_run_simulation() || MainWindowPtr->is_mouse_pressed())
        sphere = &sphereModelSimple;
    else
        sphere = &sphereModelNice;

    glBindVertexArray(sphere->vao);
    glBindBuffer(GL_ARRAY_BUFFER, cell_instance_vbo);

    shaderInstanced.setAttrPointer("instancePosition", 3, sizeof(anyCellInstance), 0);
    shaderInstanced.setAttrPointer("instanceRadius", 1, sizeof(anyCellInstance), 3);
    shaderInstanced.setAttrPointer("instanceColor", 3, sizeof(anyCellInstance), 4);

    sphere->drawInstanced(cell_instance_to_draw_cnt);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    p_avg = p_cnt > 0 ? p_avg/p_cnt : 0;
    pressure_min = 0;
    pressure_max = p_avg*2;
}


void GLWidget::draw_cylinder(anyVector const &p1, anyVector const &p2, float r, anyColor const &color)
{
    anyTransform m_matrix;

    anyVector vec = p2 - p1;

    if (p1.z == p2.z)
        vec.z = 0.0001f;

    float l = vec.length();
    float fi = -vec.z/l;
    if (fi > 1) fi = 1;
    else if (fi < -1) fi = -1;
    fi = 180/M_PI*acos(fi);
    if (vec.z < 0)
        fi = -fi;

    m_matrix.translate(p1);
    if (vec.x != 0.0 || vec.y != 0.0)
        m_matrix.rotate(fi, vectorZero, anyVector(vec.y*vec.z, -vec.x*vec.z, 0));
    else
        m_matrix.rotate(fi, vectorZero, anyVector(1, 0, 0));

    m_matrix.scale(anyVector(r, r, l));

    cylinderModel.program()->use();
    cylinderModel.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    cylinderModel.program()->setUniformMatrix3x3("norm_matrix", m_matrix.normMatrix());
    cylinderModel.program()->setUniformVector("lightDir", VisualSettings.light_dir_r);
    cylinderModel.program()->setUniformColorRGBA("colorRGBA", color);
    cylinderModel.program()->setUniformFloat("diffuse", 1.0f);
    cylinderModel.program()->setUniformFloat("ambient", 0.2f);
    cylinderModel.draw();


    m_matrix.setToTranslation(p1);
    if (vec.x != 0.0 || vec.y != 0.0)
        m_matrix.rotate(fi, vectorZero, anyVector(vec.y*vec.z, -vec.x*vec.z, 0));
    else
        m_matrix.rotate(fi, vectorZero, anyVector(1, 0, 0));
    m_matrix.scale(anyVector(r, r, r));

    sphereModelTube.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    sphereModelTube.program()->setUniformMatrix3x3("norm_matrix", m_matrix.normMatrix());
    sphereModelTube.draw();

    m_matrix.setToTranslation(p2);
    if (vec.x != 0.0 || vec.y != 0.0)
        m_matrix.rotate(fi, vectorZero, anyVector(vec.y*vec.z, -vec.x*vec.z, 0));
    else
        m_matrix.rotate(fi, vectorZero, anyVector(1, 0, 0));
    m_matrix.scale(anyVector(r, r, r));

    sphereModelTube.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    sphereModelTube.program()->setUniformMatrix3x3("norm_matrix", m_matrix.normMatrix());
    sphereModelTube.draw();
}


void GLWidget::draw_tube(anyTube *v)
/**
 Draws tube.
*/
{
    anyColor c = VisualSettings.tube_color;
    c.add(c);
    if (v->fixed_blood_pressure)
        c.add(1, 1, 1);
    else if (!v->blood_flow)
        c.add(0, 0, 0);

    draw_cylinder(v->pos1, v->pos2, v->r, c);
}



void GLWidget::draw_all_tubes()
/**
 Draws all tubes.
*/
{
    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *v = scene::TubeChains[i];
        while (v)
        {
            draw_tube(v);
            v = v->next;
        }
    }
}


void GLWidget::draw_axes()
/**
  Draws XYZ axes.
*/
{
    anyTransform m_matrix;

    anyVector scale = (SimulationSettings.comp_box_to - SimulationSettings.comp_box_from)*0.7f;
    float max_scale = qMax(qMax(scale.x, scale.y), scale.z);
    scale.x = max_scale;
    scale.y = max_scale;
    scale.z = max_scale;
    m_matrix.scale(scale);

    arrowModel.program()->use();
    arrowModel.program()->setUniformVector("lightDir", VisualSettings.light_dir_r);
    arrowModel.program()->setUniformFloat("diffuse", 0.3f);
    arrowModel.program()->setUniformFloat("ambient", 0.7f);

    // Y...

    arrowModel.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    arrowModel.program()->setUniformMatrix3x3("norm_matrix", m_matrix.normMatrix());
    arrowModel.program()->setUniformColorRGBA("colorRGBA", anyColor(0.0f, 0.8f, 0.0f, 1.0f));
    arrowModel.draw();

    // Z...

    m_matrix.rotateX(-90);
    arrowModel.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    arrowModel.program()->setUniformMatrix3x3("norm_matrix", m_matrix.normMatrix());
    arrowModel.program()->setUniformColorRGBA("colorRGBA", anyColor(0.0f, 0.5f, 1.0f, 1.0f));
    arrowModel.draw();

    // X...

    m_matrix.rotateZ(-90);
    arrowModel.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    arrowModel.program()->setUniformMatrix3x3("norm_matrix", m_matrix.normMatrix());
    arrowModel.program()->setUniformColorRGBA("colorRGBA", anyColor(1.0, 0.0, 0.0, 1.0));
    arrowModel.draw();
}



void GLWidget::draw_navigator()
/**
  Draws navigator.
*/
{
    glDisable(GL_DEPTH_TEST);

    overlaySquare.program()->use();
    overlaySquare.program()->setUniformFloat2("scale", 500.0/widget_width, 500.0/widget_height);
    overlaySquare.program()->setUniformFloat2("move", 0, 0);
    overlaySquare.program()->setTexture("tex", 0, mouse_buttons == Qt::LeftButton ? texNavigatorRotate : texNavigatorMove);
    overlaySquare.program()->setUniformColorRGB("navColor", VisualSettings.navigator_color);
    overlaySquare.draw();

    glEnable(GL_DEPTH_TEST);
}



void GLWidget::draw_legend_item(int pos, anyColor const &color, char const *label)
{
    overlaySquare.program()->use();
    overlaySquare.program()->setUniformFloat2("scale", 24.0/widget_width, 24.0/widget_height);
    overlaySquare.program()->setUniformFloat2("move", -1.0 + 40.0/widget_width, -1.0 + (40.0 + pos*60)/widget_height);
    overlaySquare.program()->setTexture("tex", 0, texLegendItem);
    overlaySquare.program()->setUniformColorRGB("navColor", VisualSettings.navigator_color);
    overlaySquare.program()->setUniformColorRGB("intColor", color);
    overlaySquare.draw();

    draw_string(35, pos*30 + 14, label, 8, VisualSettings.navigator_color);
}


void GLWidget::draw_legend_caption(int pos, QString label)
{
    draw_string(10, pos*30 + 6, label, 10, VisualSettings.navigator_color);
}


void GLWidget::draw_legend()
{
    int color_mode = MainWindowPtr->get_coloring_mode();
    int pos = 0;

    glDisable(GL_DEPTH_TEST);

    if (color_mode & COLOR_MODE_TISSUE_COLOR)
    {
        // tissue colors...
        anyTissueSettings *ts = scene::FirstTissueSettings;
        int ts_cnt = 0;
        while (ts)
        {
            draw_legend_item(pos++, ts->color, ts->name);
            ts_cnt++;

            ts = ts->next;
        }

        if (ts_cnt)
            draw_legend_caption(pos++, "Tissue colors:");
    }


    if (color_mode & COLOR_MODE_PRESSURE)
    {
        // pressure...
        draw_legend_item(pos++, anyColor(.0f, .0f, .0f), "min");
        draw_legend_item(pos++, anyColor(0.33f, 0.33f, 0.33f), "");
        draw_legend_item(pos++, anyColor(0.66f, 0.66f, 0.66f), "");
        draw_legend_item(pos++, anyColor(1, 1, 1), "max");
        draw_legend_caption(pos++, "Pressure:");
    }

    if (color_mode & COLOR_MODE_STATE)
    {
        // states...
        for (int i = sat::csAlive; i <= sat::csNecrosis; i++)
        {
            if (i == sat::csAlive)
                draw_legend_item(pos++, VisualSettings.cell_alive_color, "ALIVE");
            else if (i == sat::csHypoxia)
                draw_legend_item(pos++, VisualSettings.cell_hypoxia_color,"HYPOXIA");
            else if (i == sat::csApoptosis)
                draw_legend_item(pos++, VisualSettings.cell_apoptosis_color,"APOPTOSIS");
            else if (i == sat::csNecrosis)
                draw_legend_item(pos++, VisualSettings.cell_necrosis_color,"NECROSIS");
        }
        draw_legend_caption(pos++, "States:");
    }

    if (color_mode & COLOR_MODE_O2)
    {
        // O2...
        draw_legend_item(pos++, anyColor(0, 0, 0), "0%");
        draw_legend_item(pos++, anyColor(0, 0, 0.25), "25%");
        draw_legend_item(pos++, anyColor(0, 0, 0.5), "50%");
        draw_legend_item(pos++, anyColor(0, 0, 0.75), "75%");
        draw_legend_item(pos++, anyColor(0, 0, 1), "100%");
        draw_legend_caption(pos++, "O2 concentration:");
    }

    if (color_mode & COLOR_MODE_TAF)
    {
        // TAF...
        draw_legend_item(pos++, anyColor(0, 0, 0), "0%");
        draw_legend_item(pos++, anyColor(0, 0.25, 0), "25%");
        draw_legend_item(pos++, anyColor(0, 0.5, 0), "50%");
        draw_legend_item(pos++, anyColor(0, 0.75, 0), "75%");
        draw_legend_item(pos++, anyColor(0, 1, 0), "100%");
        draw_legend_caption(pos++, "TAF concentration:");
    }

    if (color_mode & COLOR_MODE_PERICYTES)
    {
        // TAF...
        draw_legend_item(pos++, anyColor(0, 0, 0), "0%");
        draw_legend_item(pos++, anyColor(0.25, 0.25, 0), "25%");
        draw_legend_item(pos++, anyColor(0.5, 0.5, 0), "50%");
        draw_legend_item(pos++, anyColor(0.75, 0.75, 0), "75%");
        draw_legend_item(pos++, anyColor(1, 1, 0), "100%");
        draw_legend_caption(pos++, "Pericytes concentration:");
    }

    glEnable(GL_DEPTH_TEST);
}


void GLWidget::draw_clipping_plane()
{
    anyTransform m_matrix = VisualSettings.clip;
    m_matrix.scale(anyVector(1, 1, 1)*SimulationSettings.farest_point);
    clipModel.program()->use();
    clipModel.program()->setUniformMatrix4x4("pvm_matrix", VisualSettings.p_matrix*VisualSettings.v_matrix*m_matrix);
    clipModel.program()->setUniformColorRGB("colorRGB", VisualSettings.clip_plane_color);
    clipModel.draw();
}


void GLWidget::selected_object_move(QMouseEvent *event)
{
    int x = event->pos().x() - size().width()/2;
    int y = -(event->pos().y() - size().height()/2);
    mouse_buttons = event->buttons();

    if (event->buttons() & Qt::RightButton)
    {
        // translation...
        anyVector dir(x - mouse_last_x, y - mouse_last_y, 0);
        float dl = (VisualSettings.v_matrix*anyVector(1, 0, 0) - VisualSettings.v_matrix*vectorZero).length2()*5;

        if (dl != 0) dir = dir*(1.0/dl);
        dir = VisualSettings.v_matrix*dir;
        dir.z *= -1;

      MainWindowPtr->selected_object->trans_event(dir, MainWindowPtr->X_checked(), MainWindowPtr->Y_checked(), MainWindowPtr->Z_checked());
    }
    else if (event->buttons() & Qt::LeftButton)
    {
        // rotation...
        anyVector d;

        if (mouse_press_x*mouse_press_x + mouse_press_y*mouse_press_y > 200*200)
        {
            if (x > y && x < -y)
                d.set(0, 0, x - mouse_last_x);
            else if (x <= y && x >= -y)
                d.set(0, 0, mouse_last_x - x);
            else if (x > y && x > -y)
                d.set(0, 0, y - mouse_last_y);
            else
                d.set(0, 0, mouse_last_y - y);
        }
        else
            d.set(mouse_last_y - y, x - mouse_last_x, 0);

        d *= 0.5;
        d = VisualSettings.v_matrix.inverse()*d - VisualSettings.v_matrix.inverse()*vectorZero;

        MainWindowPtr->selected_object->rotation_event(VisualSettings.v_matrix*d, MainWindowPtr->X_checked(), MainWindowPtr->Y_checked(), MainWindowPtr->Z_checked());
    }

    scene::UpdateSimulationBox();
    SaveNeeded(true);
}


void GLWidget::scene_move_or_rotate(QMouseEvent *event)
{
    int x, y;
    x = event->pos().x() - size().width()/2;
    y = -(event->pos().y() - size().height()/2);
    mouse_buttons = event->buttons();

    if (event->buttons() & Qt::LeftButton)
    {
        // rotation...
        if (mouse_press_x*mouse_press_x + mouse_press_y*mouse_press_y > 200*200)
        {
            if (x > y && x < -y)
            {
                VisualSettings.v_matrix.rotate((x - mouse_last_x)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
                VisualSettings.r_matrix.rotate((x - mouse_last_x)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
            }
            else if (x <= y && x >= -y)
            {
                VisualSettings.v_matrix.rotate((mouse_last_x - x)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
                VisualSettings.r_matrix.rotate((mouse_last_x - x)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
            }
            if (x > y && x > -y)
            {
                VisualSettings.v_matrix.rotate((y - mouse_last_y)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
                VisualSettings.r_matrix.rotate((y - mouse_last_y)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
            }
            else if (x <= y && x <= -y)
            {
                VisualSettings.v_matrix.rotate((mouse_last_y - y)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
                VisualSettings.r_matrix.rotate((mouse_last_y - y)*0.3,
                                               vectorZero,
                                               VisualSettings.v_matrix.getZ());
            }
        }
        else
        {
            VisualSettings.v_matrix.rotate(x - mouse_last_x,
                                           vectorZero,
                                           VisualSettings.v_matrix.getY());
            VisualSettings.v_matrix.rotate(mouse_last_y - y,
                                           vectorZero,
                                           VisualSettings.v_matrix.getX());
            VisualSettings.r_matrix.rotate(x - mouse_last_x,
                                           vectorZero,
                                           VisualSettings.v_matrix.getY());
            VisualSettings.r_matrix.rotate(mouse_last_y - y,
                                           vectorZero,
                                           VisualSettings.v_matrix.getX());
        }
        VisualSettings.comp_light_dir();
    }
    else
    {
        // translation...
        anyVector dir(x - mouse_last_x, y - mouse_last_y, 0);
        float dl = (VisualSettings.v_matrix*anyVector(1, 0, 0) - VisualSettings.v_matrix*vectorZero).length2()*5;

        if (dl != 0)
            dir = dir*(1.0/dl);

        VisualSettings.v_matrix.translate(VisualSettings.v_matrix.getX()*dir.x);
        VisualSettings.v_matrix.translate(VisualSettings.v_matrix.getY()*dir.y);
    }
}


void GLWidget::make_mouse_matrix(int /*x*/, int /*y*/, Qt::MouseButtons /*buttons*/, anyTransform &/*trans*/)
{
    //@@@
}


void GLWidget::clip_plane_move(QMouseEvent *event)
{
    int x, y;
     x = event->pos().x() - size().width()/2;
     y = -(event->pos().y() - size().height()/2);

     mouse_buttons = event->buttons();
     if (event->buttons() & Qt::LeftButton)
      {
         if (mouse_press_x*mouse_press_x + mouse_press_y*mouse_press_y > 200*200)
         {
             anyVector rot_vec_z = (VisualSettings.v_matrix*VisualSettings.clip).getZ();

             if (x > y && x < -y)
                 VisualSettings.clip.rotate((x - mouse_last_x)*0.3, vectorZero, rot_vec_z);
             else if (x <= y && x >= -y)
                 VisualSettings.clip.rotate((mouse_last_x - x)*0.3, vectorZero, rot_vec_z);
             if (x > y && x > -y)
                 VisualSettings.clip.rotate((y - mouse_last_y)*0.3, vectorZero, rot_vec_z);
             else if (x <= y && x <= -y)
                 VisualSettings.clip.rotate((mouse_last_y - y)*0.3, vectorZero, rot_vec_z);
         }
         else
         {
             anyVector rot_vec_x = (VisualSettings.v_matrix*VisualSettings.clip).getX();
             anyVector rot_vec_y = (VisualSettings.v_matrix*VisualSettings.clip).getY();

             VisualSettings.clip.rotate(x - mouse_last_x, vectorZero, rot_vec_y);
             VisualSettings.clip.rotate(mouse_last_y - y, vectorZero, rot_vec_x);
         }
      }

     mouse_last_x = x;
     mouse_last_y = y;

     VisualSettings.comp_clip_plane();
}


void GLWidget::user_scene_zoom(QWheelEvent *event)
{
    if (event->delta() > 0)
        user_scene_zoom(1.1);
    else if (event->delta() < 0)
        user_scene_zoom(1/1.1);
}


void GLWidget::user_scene_zoom(double zoom)
{
    VisualSettings.v_matrix.scale(anyVector(zoom, zoom, zoom));
}


void GLWidget::clip_plane_move_z(QWheelEvent *event)
{
    if (event->delta() > 0)
    {
        VisualSettings.clip.translate(anyVector(0, 0, 5));
    }
    else if (event->delta() < 0)
    {
        VisualSettings.clip.translate(anyVector(0, 0, -5));
    }
    VisualSettings.comp_clip_plane();
}


void GLWidget::selection_init()
{
    //@@@
}


void GLWidget::selection_start()
{
    //@@@
}


void GLWidget::selection_finish()
{
    //@@@
}


void GLWidget::draw_string(int x, int y, QString const s, int font_size, anyColor const &color)
{
    QPainter painter(this);
    painter.setPen(QColor(color.r255(), color.g255(), color.b255(), color.a255()));
    painter.setFont(QFont("Helvetica", font_size));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(x, height() - y, s);
    painter.end();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(1);
}
