#pragma once
#include <gtk/gtk.h>
#include <vector>
#include <algorithm>
#include <cmath>

// ----------------- WidgetFactory -----------------
class ModularWidget
{
public:
    GtkWidget *container; // custom parent container
    GtkWidget *gtkWidget; // the actual widget
    int col, row;
    int width_blocks, height_blocks;
    int total_blocks_x, total_blocks_y; // total blocks in grid

    ModularWidget(int col_, int row_,
                  int width_blocks_, int height_blocks_,
                  int total_blocks_x_ = 4, int total_blocks_y_ = 4)
        : col(col_), row(row_),
          width_blocks(width_blocks_), height_blocks(height_blocks_),
          total_blocks_x(total_blocks_x_), total_blocks_y(total_blocks_y_)
    {
    }

    void initialize()
    {
        // Parent container
        container = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(container), gtkWidget);

        // Track size allocation
        g_signal_connect(G_OBJECT(container), "size-allocate",
                         G_CALLBACK(on_size_allocate_static), this);
    }
    GtkWidget *get_widget() { return container; }

    // Update gtkWidget manually if needed
    void update_child_size(int parent_width, int parent_height)
    {
        double block_width = static_cast<double>(parent_width) / total_blocks_x;
        double block_height = static_cast<double>(parent_height) / total_blocks_y;

        int w = width_blocks * block_width;
        int h = height_blocks * block_height;

        gtk_widget_set_size_request(gtkWidget, w, h);
    }

protected:
    static void on_size_allocate_static(GtkWidget *widget, GtkAllocation *allocation, gpointer data)
    {
        static_cast<ModularWidget *>(data)->on_size_allocate(allocation);
    }

    virtual void on_size_allocate(GtkAllocation *allocation)
    {
        // Determine gtkWidget size based on number of blocks
        double block_width = static_cast<double>(allocation->width) / total_blocks_x;
        double block_height = static_cast<double>(allocation->height) / total_blocks_y;

        int w = width_blocks * block_width;
        int h = height_blocks * block_height;

        gtk_widget_set_size_request(gtkWidget, w, h);
    }
};
