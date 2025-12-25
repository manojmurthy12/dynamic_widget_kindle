#pragma once
#include <gtk/gtk.h>
#include <string>
#include <fstream>
#include <cmath>
#include "ModularWidget.h"

class BatteryWidget : public ModularWidget
{
public:
    int percentage;
    bool is_charging;
    guint timer_id;

    // Common Kindle battery paths (adjust if needed for your specific model)
    // K3/K4 often use mc13892_bat, Paperwhites often use 'max77696-battery' or just 'battery'
    const char* BATTERY_CAPACITY_PATH = "/sys/class/power_supply/max77696-battery/capacity";
    const char* BATTERY_STATUS_PATH   = "/sys/class/power_supply/max77696-battery/status";

    BatteryWidget(int col_, int row_,
                  int width_blocks_, int height_blocks_,
                  int total_blocks_x_ = 4, int total_blocks_y_ = 4)
        : ModularWidget(col_, row_, width_blocks_, height_blocks_,
                        total_blocks_x_, total_blocks_y_),
          percentage(50), is_charging(false), timer_id(0)
    {
        // Use a drawing area for custom Cairo graphics
        gtkWidget = gtk_drawing_area_new();
        
        // Connect the expose event (drawing loop)
        g_signal_connect(G_OBJECT(gtkWidget), "expose-event",
                         G_CALLBACK(on_expose_static), this);

        // Try to read real battery level immediately
        read_system_battery();

        // Optional: Update every 60 seconds
        timer_id = g_timeout_add(60000, on_update_static, this);

        initialize();
    }

    ~BatteryWidget()
    {
        if (timer_id > 0) g_source_remove(timer_id);
    }

    // Call this manually if you want to set specific values (e.g. from your server)
    void set_values(int level, bool charging)
    {
        percentage = std::max(0, std::min(100, level));
        is_charging = charging;
        gtk_widget_queue_draw(gtkWidget);
    }

private:
    // ---------------- Drawing Logic (Cairo) ----------------
    static gboolean on_expose_static(GtkWidget *widget, GdkEventExpose *event, gpointer data)
    {
        return static_cast<BatteryWidget *>(data)->on_expose(widget, event);
    }

    gboolean on_expose(GtkWidget *widget, GdkEventExpose *event)
    {
        cairo_t *cr = gdk_cairo_create(widget->window);
        
        int w = widget->allocation.width;
        int h = widget->allocation.height;

        // 1. Setup Layout Dimensions
        // Keep some padding so the line thickness doesn't clip
        double pad = 10.0;
        double avail_w = w - (pad * 2);
        double avail_h = h - (pad * 2);
        
        // Define battery body size (3:1 aspect ratio roughly looks good)
        double bat_h = avail_h * 0.5; 
        double bat_w = bat_h * 2.2;
        
        // Center the battery
        double x = (w - bat_w) / 2.0;
        double y = (h - bat_h) / 2.0;

        double terminal_w = bat_w * 0.08; // The little bump on the right
        double terminal_h = bat_h * 0.4;
        
        // Adjust main body width to make room for terminal
        double body_w = bat_w - terminal_w;

        cairo_set_line_width(cr, 4.0);
        cairo_set_source_rgb(cr, 0, 0, 0); // Black ink

        // 2. Draw Battery Body (Rounded Rect)
        double r = 5.0; // corner radius
        // Top-Left
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + r, y + r, r, M_PI, 3 * M_PI / 2);
        // Top-Right
        cairo_arc(cr, x + body_w - r, y + r, r, 3 * M_PI / 2, 0);
        // Bottom-Right
        cairo_arc(cr, x + body_w - r, y + bat_h - r, r, 0, M_PI / 2);
        // Bottom-Left
        cairo_arc(cr, x + r, y + bat_h - r, r, M_PI / 2, M_PI);
        cairo_close_path(cr);
        cairo_stroke(cr);

        // 3. Draw Terminal Bump
        cairo_rectangle(cr, x + body_w, y + (bat_h - terminal_h)/2.0, terminal_w, terminal_h);
        cairo_fill(cr);

        // 4. Draw Fill Level
        // Calculate fill width based on percentage (minus a little margin inside)
        double margin = 6.0;
        double max_fill_w = body_w - (margin * 2);
        double current_fill_w = max_fill_w * (percentage / 100.0);

        if (current_fill_w > 0)
        {
            cairo_rectangle(cr, x + margin, y + margin, current_fill_w, bat_h - (margin*2));
            cairo_fill(cr);
        }

        // 5. Draw Text Overlay (Inverse color if over filled part? Simplified to separate label here)
        // Let's draw the text *below* the battery or centered. 
        // For high contrast, let's draw it centered but we need to change color if inside black.
        // E-ink approach: Just draw a label below the battery icon for clarity.
        
        std::string label_txt = std::to_string(percentage) + "%";
        if (is_charging) label_txt += " ⚡";

        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, bat_h * 0.5);
        
        cairo_text_extents_t extents;
        cairo_text_extents(cr, label_txt.c_str(), &extents);

        // Position text centered X, and below battery Y
        double text_x = (w - extents.width) / 2.0 - extents.x_bearing;
        double text_y = y + bat_h + extents.height + 10; 

        cairo_move_to(cr, text_x, text_y);
        cairo_show_text(cr, label_txt.c_str());

        cairo_destroy(cr);
        return FALSE;
    }

    // ---------------- System Integration ----------------
    static gboolean on_update_static(gpointer data)
    {
        static_cast<BatteryWidget*>(data)->read_system_battery();
        return TRUE;
    }

    void read_system_battery()
    {
        // Try reading capacity
        std::ifstream cap_file(BATTERY_CAPACITY_PATH);
        if (cap_file.is_open()) {
            cap_file >> percentage;
        }

        // Try reading status (Charging/Discharging/Full)
        std::ifstream stat_file(BATTERY_STATUS_PATH);
        if (stat_file.is_open()) {
            std::string status;
            stat_file >> status;
            is_charging = (status == "Charging");
        }
        
        gtk_widget_queue_draw(gtkWidget);
    }
};