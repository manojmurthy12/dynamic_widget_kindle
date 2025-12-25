#pragma once
#include <gtk/gtk.h>
#include <vector>
#include <algorithm>
#include <cmath>
#define BLOCKS_X 4
#define BLOCKS_Y 4
#define PADDING 10

struct WidgetInfo
{
    GtkWidget *widget;
    int col, row;
    int width_blocks, height_blocks;
};

class KindleWindow
{
public:
    GtkWidget *window;
    GtkWidget *fixed_container;
    int screen_width;
    int screen_height;
    bool show_grid_overlay;
    std::vector<WidgetInfo> widgets;

    KindleWindow(int width, int height) : screen_width(width), screen_height(height)
    {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window),
                             "L:A_N:application_ID:org.kindlemodding.example-gtk-application_PC:N");
        gtk_window_set_default_size(GTK_WINDOW(window), width, height);
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        fixed_container = gtk_fixed_new();
        gtk_container_add(GTK_CONTAINER(window), fixed_container);

        g_signal_connect(G_OBJECT(fixed_container), "expose-event",
                         G_CALLBACK(on_expose_static), this);
        g_signal_connect(G_OBJECT(window), "configure-event",
                         G_CALLBACK(on_configure_static), this);
    }

    void add_widget_at_grid(GtkWidget *widget, int col, int row,
                            int width_blocks = 1, int height_blocks = 1)
    {
        col = std::max(1, std::min(col, BLOCKS_X));
        row = std::max(1, std::min(row, BLOCKS_Y));
        width_blocks = std::max(1, std::min(width_blocks, BLOCKS_X - col + 1));
        height_blocks = std::max(1, std::min(height_blocks, BLOCKS_Y - row + 1));

        WidgetInfo info{widget, col, row, width_blocks, height_blocks };
        widgets.push_back(info);

        update_widget_position(info, true);
    }

    void add_widget_at_grid(ModularWidget *modWidget)
    {
        GtkWidget *widget = modWidget->container;
        int col = modWidget->col;
        int row = modWidget->row;
        int width_blocks = modWidget->width_blocks;
        int height_blocks = modWidget->height_blocks;

        col = std::max(1, std::min(col, BLOCKS_X));
        row = std::max(1, std::min(row, BLOCKS_Y));
        width_blocks = std::max(1, std::min(width_blocks, BLOCKS_X - col + 1));
        height_blocks = std::max(1, std::min(height_blocks, BLOCKS_Y - row + 1));

        WidgetInfo info{widget, col, row, width_blocks, height_blocks};
        widgets.push_back(info);

        update_widget_position(info, true);
    }

    void show_all() { gtk_widget_show_all(window); }

    void set_grid_overlay(bool enable)
    {
        show_grid_overlay = enable;
        gtk_widget_queue_draw(fixed_container);
    }

private:
    static gboolean on_expose_static(GtkWidget *widget, GdkEventExpose *event, gpointer data)
    {
        return static_cast<KindleWindow *>(data)->on_expose(widget, event);
    }

    gboolean on_expose(GtkWidget *widget, GdkEventExpose *event)
    {
        if (!show_grid_overlay)
            return FALSE; // skip drawing overlay
        cairo_t *cr = gdk_cairo_create(widget->window);
        double block_width = static_cast<double>(screen_width) / BLOCKS_X;
        double block_height = static_cast<double>(screen_height) / BLOCKS_Y;

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 1);

        for (int i = 1; i < BLOCKS_X; i++)
        {
            double x = i * block_width;
            cairo_move_to(cr, x, 0);
            cairo_line_to(cr, x, screen_height);
        }
        for (int j = 1; j < BLOCKS_Y; j++)
        {
            double y = j * block_height;
            cairo_move_to(cr, 0, y);
            cairo_line_to(cr, screen_width, y);
        }

        cairo_stroke(cr);
        cairo_destroy(cr);
        return FALSE;
    }

    static gboolean on_configure_static(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
    {
        return static_cast<KindleWindow *>(data)->on_configure(widget, event);
    }

    gboolean on_configure(GtkWidget *widget, GdkEventConfigure *event)
    {
        screen_width = event->width;
        screen_height = event->height;

        gtk_widget_queue_draw(fixed_container);

        for (auto &info : widgets)
        {
            update_widget_position(info, false);
        }
        return FALSE;
    }

    void update_widget_position(WidgetInfo &info, bool first_time = false)
    {
        double block_width = static_cast<double>(screen_width) / BLOCKS_X;
        double block_height = static_cast<double>(screen_height) / BLOCKS_Y;

        int x = (info.col - 1) * block_width + PADDING;
        int y = (info.row - 1) * block_height + PADDING;
        int w = info.width_blocks * block_width - 2 * PADDING;
        int h = info.height_blocks * block_height - 2 * PADDING;

        gtk_widget_set_size_request(info.widget, w, h);

        if (first_time)
            gtk_fixed_put(GTK_FIXED(fixed_container), info.widget, x, y);
        else
            gtk_fixed_move(GTK_FIXED(fixed_container), info.widget, x, y);
    }
};