#pragma once
#include <gtk/gtk.h>
#include <cmath>
#include "ModularWidget.h"

class SpeakerGrill : public ModularWidget
{
public:
    int radius;
    int rows, cols;
    int total_dots;
    int filled_dots;

    SpeakerGrill(int col_, int row_,
                 int width_blocks_, int height_blocks_,
                 int radius_,
                 int total_blocks_x_ = 4, int total_blocks_y_ = 4)
        : ModularWidget(col_, row_,
                        width_blocks_, height_blocks_,
                        total_blocks_x_, total_blocks_y_),
          radius(radius_), rows(0), cols(0),
          total_dots(0), filled_dots(-1) // -1 means "fill all"
    {
        gtkWidget = gtk_drawing_area_new();
        g_signal_connect(G_OBJECT(gtkWidget), "expose-event",
                         G_CALLBACK(on_expose_static), this);
        initialize();
    }

    GtkWidget *get_widget() { return gtkWidget; }

protected:
    static gboolean on_expose_static(GtkWidget *widget, GdkEventExpose *event, gpointer data)
    {
        return static_cast<SpeakerGrill *>(data)->on_expose(widget, event);
    }

    virtual gboolean on_expose(GtkWidget *widget, GdkEventExpose *event)
    {
        int width = widget->allocation.width;
        int height = widget->allocation.height;

        double r = static_cast<double>(radius);
        double s = 4.0 * r;

        cols = std::floor((width - r) / s);
        rows = std::floor((height - r) / s);

        if (cols < 1) cols = 1;
        if (rows < 1) rows = 1;

        total_dots = rows * cols;

        if (filled_dots < 0 || filled_dots > total_dots)
            filled_dots = total_dots;

        double offset_x = (width - (cols - 1) * s - 2 * r) / 2.0 + r;
        double offset_y = (height - (rows - 1) * s - 2 * r) / 2.0 + r;

        cairo_t *cr = gdk_cairo_create(widget->window);

        int count = 0;
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                double x = offset_x + j * s;
                double y = offset_y + i * s;

                if (count < filled_dots)
                    cairo_set_source_rgb(cr, 0, 0, 0); // filled
                else
                    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5); // empty

                cairo_arc(cr, x, y, r, 0, 2 * M_PI);
                cairo_fill(cr);

                count++;
            }
        }

        cairo_destroy(cr);
        return FALSE;
    }
};
