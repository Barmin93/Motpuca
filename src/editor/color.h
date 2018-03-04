#ifndef COLOR_H
#define COLOR_H

#ifdef QT_CORE_LIB
#include <QGLWidget>
#define col_float GLfloat
#else
#define col_float float
#endif

#include <cstdio>

extern int Color_Mode;
void SetColorMode(int cm);

class anyColor
{
public:
    col_float rgba_array[4];  ///< rgba_array
    float cnt;                ///< number of colors added

    anyColor(float _r = 0, float _g = 0, float _b = 0, float _a = 1, int _cnt = 0) { set(_r, _g, _b, _a, _cnt); }

    void set(float _r, float _g, float _b, float _a = 1, int _cnt = 1)
    {
        rgba_array[0] = _r;
        rgba_array[1] = _g;
        rgba_array[2] = _b;
        rgba_array[3] = _a;
        cnt = _cnt;
    }

    void set(anyColor const &c, int _cnt = 1)
    {
        for (int i = 0; i < 4; i++)
            rgba_array[i] = c.rgba_array[i];
        cnt = _cnt;
    }

    void set_skip_alpha(anyColor const &c, int _cnt = 1)
    {
        for (int i = 0; i < 3; i++)
            rgba_array[i] = c.rgba_array[i];
        cnt = _cnt;
    }

    void add(float _r, float _g, float _b, float _a = 1)
    {
        rgba_array[0] = (cnt*rgba_array[0] + _r)/(cnt + 1);
        rgba_array[1] = (cnt*rgba_array[1] + _g)/(cnt + 1);
        rgba_array[2] = (cnt*rgba_array[2] + _b)/(cnt + 1);
        rgba_array[3] = (cnt*rgba_array[3] + _a)/(cnt + 1);
        cnt++;
    }

    void add(anyColor const &c)
    {
        rgba_array[0] = (cnt*rgba_array[0] + c.rgba_array[0])/(cnt + 1);
        rgba_array[1] = (cnt*rgba_array[1] + c.rgba_array[1])/(cnt + 1);
        rgba_array[2] = (cnt*rgba_array[2] + c.rgba_array[2])/(cnt + 1);
        rgba_array[3] = (cnt*rgba_array[3] + c.rgba_array[3])/(cnt + 1);
        cnt++;
    }

    void rgb_mul(float x)
    {
        for (int i = 0; i < 3; i++)
            rgba_array[i] *= x;
    }

    char *to_HTML() const
    {
        static char html[9];
        snprintf(html, 8, "#%02x%02x%02x", int(rgba_array[0]*255), int(rgba_array[1]*255), int(rgba_array[2]*255));
        return html;
    }

    char *to_string_rgb() const
    {
        static char h[10][100];
        static int h_indx = 0;
        h_indx = (h_indx + 1) % 10;
        snprintf(h[h_indx], 100, "<%g, %g, %g>", rgba_array[0], rgba_array[1], rgba_array[2]);
        return h[h_indx];
    }

    char *to_string_rgba() const
    {
        static char h[10][100];
        static int h_indx = 0;
        h_indx = (h_indx + 1) % 10;
        snprintf(h[h_indx], 100, "<%g, %g, %g, %g>", rgba_array[0], rgba_array[1], rgba_array[2], rgba_array[3]);
        return h[h_indx];
    }

    col_float const *rgba() const
    {
        static col_float rgba_mod[4];

        switch (Color_Mode)
        {
        case 1:
            rgba_mod[0] = rgba_mod[1] = rgba_mod[2] = (rgba_array[0]*0.299 + rgba_array[1]*0.587 + rgba_array[2]*0.114);
            rgba_mod[3] = rgba_array[3];
            return rgba_mod;
        default:
            return rgba_array;
        }

    }

    float r() const { return rgba()[0]; }
    float g() const { return rgba()[1]; }
    float b() const { return rgba()[2]; }
    float a() const { return rgba()[3]; }

    int r255() const { return r()*255; }
    int g255() const { return g()*255; }
    int b255() const { return b()*255; }
    int a255() const { return a()*255; }
};

#endif // COLOR_H
