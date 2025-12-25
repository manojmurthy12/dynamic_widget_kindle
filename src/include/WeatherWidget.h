#pragma once
#include <gtk/gtk.h>
#include <string>
#include "ModularWidget.h"

class WeatherWidget : public ModularWidget
{
public:
    GtkWidget *icon_label;
    GtkWidget *temp_label;
    GtkWidget *cond_label;

    WeatherWidget(int col_, int row_,
                  int width_blocks_, int height_blocks_, const std::string &icon = "☀", int temp = 25, const std::string &cond = "Sunny",
                  int total_blocks_x_ = 4, int total_blocks_y_ = 4) : ModularWidget(col_, row_,
                                                                                    width_blocks_, height_blocks_,
                                                                                    total_blocks_x_, total_blocks_y_)
    {
        gtkWidget = gtk_vbox_new(FALSE, 5);

        // Weather icon (big)
        icon_label = gtk_label_new(NULL);
        std::string icon_markup = "<span size='24000'>" + icon + "</span>";
        gtk_label_set_markup(GTK_LABEL(icon_label), icon_markup.c_str());
        gtk_box_pack_start(GTK_BOX(gtkWidget), icon_label, FALSE, FALSE, 0);

        // Temperature (bold, large)
        temp_label = gtk_label_new(NULL);
        std::string temp_markup = "<span size='20000' weight='bold'>" + std::to_string(temp) + "°C</span>";
        gtk_label_set_markup(GTK_LABEL(temp_label), temp_markup.c_str());
        gtk_box_pack_start(GTK_BOX(gtkWidget), temp_label, FALSE, FALSE, 0);

        // Condition (italic, smaller)
        cond_label = gtk_label_new(NULL);
        std::string cond_markup = "<span size='14000' style='italic'>" + cond + "</span>";
        gtk_label_set_markup(GTK_LABEL(cond_label), cond_markup.c_str());
        gtk_box_pack_start(GTK_BOX(gtkWidget), cond_label, FALSE, FALSE, 0);

        gtk_widget_show_all(gtkWidget);
        initialize();
    }

    GtkWidget *get_widget() { return gtkWidget; }

    // Update values later (e.g. from API or manual input)
    void update_weather(const std::string &icon, int temp, const std::string &cond)
    {
        std::string icon_markup = "<span size='24000'>" + icon + "</span>";
        gtk_label_set_markup(GTK_LABEL(icon_label), icon_markup.c_str());

        std::string temp_markup = "<span size='20000' weight='bold'>" + std::to_string(temp) + "°C</span>";
        gtk_label_set_markup(GTK_LABEL(temp_label), temp_markup.c_str());

        std::string cond_markup = "<span size='14000' style='italic'>" + cond + "</span>";
        gtk_label_set_markup(GTK_LABEL(cond_label), cond_markup.c_str());
    }
};
