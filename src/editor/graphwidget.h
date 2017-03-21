#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QPainter>

#include "model.h"
#include "config.h"
#include "statistics.h"
#include "scene.h"

#define GR_MARGIN_LEFT 75
#define GR_MARGIN_RIGHT 30
#define GR_V_SPACE 30
#define GR_MARGIN_MAX_TOP 10
#define GR_YLABEL_MARGIN 10

class GraphWidget: public QWidget
{
private:
    int frame_x1, frame_y1;
    int frame_x2, frame_y2;
    float my, sx;
public:
    GraphWidget(QWidget *parent): QWidget(parent)
    {}

    void paintEvent(QPaintEvent */*event*/)
    {
        QPainter painter(this);

        SetColorMode(0);

//        painter.fillRect(1, 1, width() - 2, height() - 2, QBrush(QColor(VisualSettings.bkg_color.r255(), VisualSettings.bkg_color.g255(), VisualSettings.bkg_color.b255())));
        painter.fillRect(1, 1, width() - 2, height() - 2, QBrush(QColor(255, 255, 255)));

        if (width() > 0 && Statistics)
        {
            int fy = (height() - GR_V_SPACE)/(NoTissueSettings + 1);

            for (int i = 0; i < NoTissueSettings + 1; i++)
            {
                set_frame(GR_MARGIN_LEFT, GR_V_SPACE + (NoTissueSettings - 1 - i + 1)*fy, width() - GR_MARGIN_RIGHT, (NoTissueSettings - i + 1)*fy);
                anyTissueSettings *t = FindTissueSettingById(i);
                if (i < NoTissueSettings)
                    paint_frame(painter, MaxStatistics.counter[i*csLast], t->name);
                else
                    paint_frame(painter, MaxStatistics.counter[i*csLast], MODEL_TUBE_NAME_PL);
                paint_graph(painter, i);
            }
        }

        painter.setClipping(false);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(QPen(QColor(128, 128, 128), 1));
        painter.drawRect(0, 0, width() - 1, height() - 1);
    }

    void set_frame(int x1, int y1, int x2, int y2)
    {
        frame_x1 = x1;
        frame_x2 = x2;
        frame_y1 = y1;
        frame_y2 = y2;
    }

    int paint_y_label(QPainter &p, int value, bool label, bool line)
    {
        p.setPen(QPen(QColor(200, 200, 200), 1));
        p.drawLine(frame_x1 - 7, height() -(value*my + frame_y1), frame_x1, height() - (value*my + frame_y1));

        QRect r;
        if (label)
            p.setPen(QPen(QColor(128, 128, 128), 1));
        else
            p.setPen(QPen(QColor(220, 220, 220), 1));
        p.drawText(0, height() -(value*my + frame_y1) - 10,
                   frame_x1 - GR_YLABEL_MARGIN, 20,
                   Qt::AlignVCenter | Qt::AlignRight,
                   QString::number(value),
                   &r);

        if (line)
        {
            p.setPen(QPen(QColor(200, 200, 200), 1));
            p.drawLine(frame_x1, height() -(value*my + frame_y1), frame_x2, height() - (value*my + frame_y1));
        }

        return r.height();
    }

    int paint_x_label(QPainter &p, int value, bool label)
    {
        p.setPen(QPen(QColor(200, 200, 200), 1));
        int x = frame_x1 + value*sx/SimulationSettings.graph_sampling;
        if (x == frame_x2 - 1)
            x = frame_x2;
        p.drawLine(x, height() - frame_y1,
                   x, height() - frame_y1 + 5);

        QRect r;
        if (label)
            p.setPen(QPen(QColor(128, 128, 128), 1));
        else
            p.setPen(QPen(QColor(220, 220, 220), 1));
        p.drawText(x - 50, height() - frame_y1 + 5,
                   100, 20,
                   Qt::AlignHCenter | Qt::AlignTop,
                   QString::number(value),
                   &r);
        return r.width();
    }

    void paint_frame(QPainter &p, float max_y, QString label)
    {
        my = (frame_y2 - frame_y1 - 10)/(max_y + 1);
        sx = float(LastStatistics->step)/SimulationSettings.graph_sampling/(frame_x2 - frame_x1);
        if (sx < 1)
            sx = 1;
        else
            sx = 1/sx;

        p.setClipping(false);
        p.setRenderHint(QPainter::Antialiasing, false);

        // Y...
        int th = paint_y_label(p, 0, true, false);
        int t = 1;
        int mn = 5;
        while (max_y/t > 10)
        {
            t *= mn;     // 1, 5, 10, 50, 100, 500, ...
            mn = 7 - mn; // 2 <-> 5
        }

        for (int i = 0; i < max_y; i += t)
            paint_y_label(p, i, (max_y - i)*my > th + 5, true);
        paint_y_label(p, max_y, true, false);

        // X...
        paint_x_label(p, 0, true);
        int tw = paint_x_label(p, LastStatistics->step, true) + 5;

        t = 1;
        mn = 5;
        while (t*sx/SimulationSettings.graph_sampling < tw)
        {
            t *= mn;     // 1, 5, 10, 50, 100, 500, ...
            mn = 7 - mn; // 2 <-> 5
        }

        for (int i = 0; i < LastStatistics->step; i += t)
            paint_x_label(p, i, (LastStatistics->step - i)*sx/SimulationSettings.graph_sampling > tw);
        paint_x_label(p, LastStatistics->step, true);

        p.setPen(QPen(QColor(128, 128, 128), 1));
        p.drawRect(frame_x1, height() - frame_y1, frame_x2 - frame_x1, frame_y1 - frame_y2);

        p.setPen(QPen(QColor(0, 0, 0), 1));
        p.setFont(QFont(p.font().family(), -1, QFont::Bold));
        p.drawText(frame_x1 + 10, height() - (frame_y2 - 15), label);
        p.setFont(QFont(p.font().family(), -1, QFont::Normal));

        p.setClipRect(frame_x1 + 1, height() - frame_y1 - 1, frame_x2 - frame_x1 - 2, frame_y1 - frame_y2 - 2);
        p.setClipping(true);
    }

    void paint_graph(QPainter &p, int counter_id)
    {
        anyStatData *sd = Statistics;

        float x = 0;
        int y = sd->counter[counter_id*csLast];

        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(QColor(0, 0, 0), 1));

        while (sd)
        {
            p.drawLine(frame_x1 + x, height() - (frame_y1 + y*my), frame_x1 + x + sx, height() - (frame_y1 + sd->counter[counter_id*csLast]*my));
            y = sd->counter[counter_id*csLast];
            x += sx;

            sd = sd->next;
        }

        for (int i = csAlive; i < csLast; i++)
        {
            switch (i)
            {
            case csAlive:
                p.setPen(QPen(QColor(VisualSettings.cell_alive_color.r255(), VisualSettings.cell_alive_color.g255(), VisualSettings.cell_alive_color.b255()), 1));
                break;
            case csHypoxia:
                p.setPen(QPen(QColor(VisualSettings.cell_hypoxia_color.r255(), VisualSettings.cell_hypoxia_color.g255(), VisualSettings.cell_hypoxia_color.b255()), 1));
                break;
            case csApoptosis:
                p.setPen(QPen(QColor(VisualSettings.cell_apoptosis_color.r255(), VisualSettings.cell_apoptosis_color.g255(), VisualSettings.cell_apoptosis_color.b255()), 1));
                break;
            case csNecrosis:
                p.setPen(QPen(QColor(VisualSettings.cell_necrosis_color.r255(), VisualSettings.cell_necrosis_color.g255(), VisualSettings.cell_necrosis_color.b255()), 1));
                break;
            }

            sd = Statistics;
            x = 0;
            y = sd->counter[counter_id*csLast + i];
            while (sd)
            {
                p.drawLine(frame_x1 + x, height() - (frame_y1 + y*my), frame_x1 + x + sx, height() - (frame_y1 + sd->counter[counter_id*csLast + i]*my));
                y = sd->counter[counter_id*csLast + i];
                x += sx;

                sd = sd->next;
            }
        }
    }
};

#endif // GRAPHWIDGET_H


























