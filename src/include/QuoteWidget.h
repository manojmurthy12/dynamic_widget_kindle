#pragma once
#include "ModularWidget.h"
#include <string>

class QuoteWidget : public ModularWidget
{
    GtkWidget *quote_label;
    GtkWidget *align;   // alignment container to allow vertical expansion
    std::string text;

public:
    QuoteWidget(int col_, int row_,
                int width_blocks_, int height_blocks_,
                const std::string &quote = "Stay hungry, stay foolish.",
                int total_blocks_x_ = 4, int total_blocks_y_ = 4)
        : ModularWidget(col_, row_,
                        width_blocks_, height_blocks_,
                        total_blocks_x_, total_blocks_y_),
          text(quote)
    {
        // Create frame container
        gtkWidget = gtk_frame_new(NULL);

        // Create label
        quote_label = gtk_label_new(NULL);
        gtk_label_set_line_wrap(GTK_LABEL(quote_label), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
        gtk_label_set_line_wrap_mode(GTK_LABEL(quote_label), PANGO_WRAP_WORD_CHAR);
#endif
        gtk_label_set_justify(GTK_LABEL(quote_label), GTK_JUSTIFY_CENTER);

        gtk_misc_set_padding(GTK_MISC(quote_label), 6, 6); // left/right & top/bottom
        // Wrap label in alignment to allow vertical expansion
        align = gtk_alignment_new(0.5, 0.5, 0.9, 1.0); // x, y, scale_x, scale_y
        gtk_container_add(GTK_CONTAINER(align), quote_label);

        // Add alignment to frame
        gtk_container_add(GTK_CONTAINER(gtkWidget), align);

        // Apply initial text
        set_markup(text);

        gtk_widget_show_all(gtkWidget);

        initialize(); // ModularWidget initialization (connect size-allocate)
    }

    GtkWidget *get_widget() { return gtkWidget; }

    void update(const std::string &quote)
    {
        text = quote;
        set_markup(text);
    }

protected:
    // Override to handle wrapping when the container resizes
    void on_size_allocate(GtkAllocation *allocation) override
    {
        // Let base class handle frame sizing
        ModularWidget::on_size_allocate(allocation);

        // Compute max width according to width_blocks
        int block_width = allocation->width / total_blocks_x;
        int max_width = block_width * width_blocks - 6; // leave small padding

        // Only update width if different to avoid recursion
        int current_width, current_height;
        gtk_widget_get_size_request(quote_label, &current_width, &current_height);
        if (current_width != max_width)
        {
            gtk_widget_set_size_request(quote_label, max_width, -1);
        }
    }

private:
    void set_markup(const std::string &quote)
    {
        std::string markup =
            "<span size='11000' style='italic'>" + quote + "</span>";
        gtk_label_set_markup(GTK_LABEL(quote_label), markup.c_str());
    }
};
