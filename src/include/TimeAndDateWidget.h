#pragma once
#include <gtk/gtk.h>
#include <ctime>
#include <string>
#include "ModularWidget.h"

class TimeDateWidget : public ModularWidget
{
public:
    int blocks;
    int update_interval_ms; // refresh interval
    bool update_seconds;
    guint timer_id;

    TimeDateWidget(int col_, int row_,
                   int width_blocks_, int height_blocks_, int blocks_ = 1, bool update_secs = true,
                   int total_blocks_x_ = 4, int total_blocks_y_ = 4)
        : blocks(blocks_), update_seconds(update_secs), ModularWidget(col_, row_,
                                                                      width_blocks_, height_blocks_,
                                                                      total_blocks_x_, total_blocks_y_),
          timer_id(0)
    {
        gtkWidget = gtk_label_new(NULL);

        update_time(); // initial display

        if (update_seconds)
        {
            update_interval_ms = 1000;
            timer_id = g_timeout_add(update_interval_ms, on_timeout_static, this);
        }
        else
        {
            // compute time until next minute boundary
            time_t now = time(nullptr);
            struct tm *t = localtime(&now);
            int ms_until_next_min = (60 - t->tm_sec) * 1000;

            // first single-shot timeout until next minute boundary
            g_timeout_add(ms_until_next_min, on_align_to_minute_static, this);
        }

        initialize();
    }

    GtkWidget *get_widget() { return gtkWidget; }

private:
    static gboolean on_timeout_static(gpointer data)
    {
        return static_cast<TimeDateWidget *>(data)->update_time();
    }

    static gboolean on_align_to_minute_static(gpointer data)
    {
        auto *self = static_cast<TimeDateWidget *>(data);

        // Update at exact minute boundary
        self->update_time();

        // Now schedule regular updates every 60s
        self->update_interval_ms = 60000;
        self->timer_id = g_timeout_add(self->update_interval_ms, on_timeout_static, self);

        // Do not repeat this one-shot timer
        return FALSE;
    }

    gboolean update_time()
    {
        time_t now = time(nullptr);
        struct tm *t = localtime(&now);

        char buffer[128];
        if (update_seconds)
        {
            snprintf(buffer, sizeof(buffer),
                     "<span size='24000' weight='bold'>%02d:%02d</span>"
                     "<span size='14000'>:%02d</span>\n"
                     "<span size='16000' style='italic'>%02d-%02d-%04d</span>",
                     t->tm_hour, t->tm_min, t->tm_sec,
                     t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
        }
        else
        {
            snprintf(buffer, sizeof(buffer),
                     "<span size='24000' weight='bold'>%02d:%02d</span>\n"
                     "<span size='16000' style='italic'>%02d-%02d-%04d</span>",
                     t->tm_hour, t->tm_min,
                     t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
        }

        gtk_label_set_markup(GTK_LABEL(gtkWidget), buffer);
        return TRUE;
    }
};
