#pragma once
#include <gtk/gtk.h>
#include <vector>
#include <algorithm>
#include <cmath>

class BatteryWidget
{
    GtkWidget *drawing_area;
    int percent;

public:
    BatteryWidget(int initial_percent = 75) : percent(initial_percent)
    {
        drawing_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(drawing_area, 100, 40); // fixed size for battery

        g_signal_connect(G_OBJECT(drawing_area), "expose-event",
                         G_CALLBACK(on_expose_static), this);

        gtk_widget_show_all(drawing_area);
    }

    GtkWidget *get_widget() { return drawing_area; }

    void update(int new_percent)
    {
        percent = std::max(0, std::min(new_percent, 100));
        gtk_widget_queue_draw(drawing_area); // trigger redraw
    }

private:
    static gboolean on_expose_static(GtkWidget *widget, GdkEventExpose *event, gpointer data)
    {
        return static_cast<BatteryWidget *>(data)->on_expose(widget, event);
    }

    gboolean on_expose(GtkWidget *widget, GdkEventExpose *event)
    {
        cairo_t *cr = gdk_cairo_create(widget->window);

        int w = widget->allocation.width;
        int h = widget->allocation.height;

        // Draw battery outer rectangle
        cairo_set_line_width(cr, 2);
        cairo_rectangle(cr, 1, 1, w - 10, h - 2); // leave space for tip
        cairo_stroke(cr);

        // Fill battery according to percentage
        double fill_width = (w - 12) * (percent / 100.0);
        cairo_set_source_rgb(cr, (percent < 20 ? 1.0 : 0.0), (percent >= 20 ? 0.7 : 0.0), 0.0);
        cairo_rectangle(cr, 2, 2, fill_width, h - 4);
        cairo_fill(cr);

        // Draw percentage text smaller
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, h / 10); // smaller text size

        std::string text = std::to_string(percent) + "%";
        cairo_text_extents_t extents;
        cairo_text_extents(cr, text.c_str(), &extents);

        double x = (w - extents.width) / 2 - extents.x_bearing;
        double y = (h - extents.height) / 2 - extents.y_bearing;

        cairo_move_to(cr, x, y);
        cairo_show_text(cr, text.c_str());

        cairo_destroy(cr);
        return TRUE;
    }
};

class QuoteWidget
{
    GtkWidget *frame;
    GtkWidget *quote_label;

public:
    QuoteWidget(const std::string &quote = "Stay hungry, stay foolish.")
    {
        frame = gtk_frame_new(NULL);

        quote_label = gtk_label_new(NULL);
        std::string markup = "<span size='11000' style='italic'>" + quote + "</span>";
        gtk_label_set_markup(GTK_LABEL(quote_label), markup.c_str());

        gtk_container_add(GTK_CONTAINER(frame), quote_label);
        gtk_widget_show_all(frame);
    }

    GtkWidget *get_widget() { return frame; }

    void update(const std::string &quote)
    {
        std::string markup = "<span size='11000' style='italic'>" + quote + "</span>";
        gtk_label_set_markup(GTK_LABEL(quote_label), markup.c_str());
    }
};

class NotificationWidget
{
    GtkWidget *frame;
    GtkWidget *title_label;
    GtkWidget *msg_label;

public:
    NotificationWidget(const std::string &title, const std::string &msg)
    {
        frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

        GtkWidget *vbox = gtk_vbox_new(FALSE, 2);

        title_label = gtk_label_new(NULL);
        std::string title_markup = "<span size='12000' weight='bold'>" + title + "</span>";
        gtk_label_set_markup(GTK_LABEL(title_label), title_markup.c_str());
        gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);

        msg_label = gtk_label_new(NULL);
        std::string msg_markup = "<span size='10000'>" + msg + "</span>";
        gtk_label_set_markup(GTK_LABEL(msg_label), msg_markup.c_str());
        gtk_box_pack_start(GTK_BOX(vbox), msg_label, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_widget_show_all(frame);
    }

    GtkWidget *get_widget() { return frame; }

    void update(const std::string &title, const std::string &msg)
    {
        std::string title_markup = "<span size='12000' weight='bold'>" + title + "</span>";
        gtk_label_set_markup(GTK_LABEL(title_label), title_markup.c_str());
        std::string msg_markup = "<span size='10000'>" + msg + "</span>";
        gtk_label_set_markup(GTK_LABEL(msg_label), msg_markup.c_str());
    }
};

class MusicWidget
{
    GtkWidget *hbox;
    GtkWidget *track_label;
    GtkWidget *play_button;

public:
    MusicWidget(const std::string &track = "No music")
    {
        hbox = gtk_hbox_new(FALSE, 5);

        track_label = gtk_label_new(NULL);
        std::string markup = "<span size='11000'>" + track + "</span>";
        gtk_label_set_markup(GTK_LABEL(track_label), markup.c_str());
        gtk_box_pack_start(GTK_BOX(hbox), track_label, TRUE, TRUE, 0);

        play_button = gtk_button_new_with_label("▶");
        gtk_box_pack_start(GTK_BOX(hbox), play_button, FALSE, FALSE, 0);

        gtk_widget_show_all(hbox);
    }

    GtkWidget *get_widget() { return hbox; }

    void update(const std::string &track)
    {
        std::string markup = "<span size='11000'>" + track + "</span>";
        gtk_label_set_markup(GTK_LABEL(track_label), markup.c_str());
    }
};
