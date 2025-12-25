#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <utility>
#include <gtk/gtk.h>
#include "SpeakerGrill.h"

struct Dot
{
    double x, y;   // current position
    double tx, ty; // target position
};

class SpeakerGrillDice : public SpeakerGrill
{
public:
    SpeakerGrillDice(int col_, int row_,
                     int width_blocks_ = 1, int height_blocks_ = 1,
                     int radius_ = 22,
                     int total_blocks_x_ = 4, int total_blocks_y_ = 4)
        : SpeakerGrill(col_, row_, width_blocks_, height_blocks_, radius_,
                       total_blocks_x_, total_blocks_y_)
    {
        std::srand(std::time(NULL));
        gtk_widget_set_events(gtkWidget, GDK_BUTTON_PRESS_MASK);
        g_signal_connect(G_OBJECT(gtkWidget), "button-press-event",
                         G_CALLBACK(on_click_static), this);

        GdkEventButton ev{};
        ev.type = GDK_BUTTON_PRESS;
        ev.button = 1;
        ev.x = 0;
        ev.y = 0;
        on_click(gtkWidget, &ev);
    }

protected:
    gboolean on_expose(GtkWidget *widget, GdkEventExpose *event) override
    {
        cairo_t *cr = gdk_cairo_create(widget->window);

        if (show_noise)
        {
            // Draw rolling noise: random squares
            cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
            int square_size = radius;
            for (int i = 0; i < 30; i++)
            {
                double x = std::rand() % (widget->allocation.width - square_size);
                double y = std::rand() % (widget->allocation.height - square_size);
                cairo_rectangle(cr, x, y, square_size, square_size);
                cairo_fill(cr);
            }
        }
        else
        {
            // Draw final dice dots
            cairo_set_source_rgb(cr, 0, 0, 0);
            for (auto &d : dots)
            {
                cairo_arc(cr, d.x, d.y, radius, 0, 2 * M_PI);
                cairo_fill(cr);
            }
        }

        cairo_destroy(cr);
        return FALSE;
    }

private:
    std::vector<Dot> dots;
    guint animation_timer = 0;
    guint noise_timer = 0;
    bool show_noise = false;

    // Timer for animation
    static gboolean animate_static(gpointer data)
    {
        return static_cast<SpeakerGrillDice *>(data)->animate();
    }

    gboolean animate()
    {
        bool moving = false;
        for (auto &d : dots)
        {
            double dx = d.tx - d.x;
            double dy = d.ty - d.y;
            double dist = sqrt(dx * dx + dy * dy);

            if (dist > 1.0)
            {
                d.x += dx * 0.2; // smooth step
                d.y += dy * 0.2;
                moving = true;
            }
            else
            {
                d.x = d.tx;
                d.y = d.ty;
            }
        }

        gtk_widget_queue_draw(gtkWidget);

        if (!moving)
        {
            animation_timer = 0;
            return FALSE; // stop
        }
        return TRUE; // keep going
    }

    // Timer for noise
    static gboolean noise_static(gpointer data)
    {
        return static_cast<SpeakerGrillDice *>(data)->show_noise_step();
    }

    gboolean show_noise_step()
    {
        gtk_widget_queue_draw(gtkWidget);
        return TRUE; // keep noise drawing until stopped
    }

    // Compute dice-face positions with padding
    std::vector<std::pair<double, double>> dice_positions(int roll, int width, int height)
    {
        std::vector<std::pair<double, double>> positions;

        double cx = width / 2.0;
        double cy = height / 2.0;
        double pad = radius * 2.5; // padding from center

        switch (roll)
        {
        case 1:
            positions.push_back({cx, cy});
            break;
        case 2:
            positions.push_back({cx - pad, cy - pad});
            positions.push_back({cx + pad, cy + pad});
            break;
        case 3:
            positions.push_back({cx - pad, cy - pad});
            positions.push_back({cx, cy});
            positions.push_back({cx + pad, cy + pad});
            break;
        case 4:
            positions.push_back({cx - pad, cy - pad});
            positions.push_back({cx + pad, cy - pad});
            positions.push_back({cx - pad, cy + pad});
            positions.push_back({cx + pad, cy + pad});
            break;
        case 5:
            positions.push_back({cx - pad, cy - pad});
            positions.push_back({cx + pad, cy - pad});
            positions.push_back({cx, cy});
            positions.push_back({cx - pad, cy + pad});
            positions.push_back({cx + pad, cy + pad});
            break;
        case 6:
            positions.push_back({cx - pad, cy - pad});
            positions.push_back({cx + pad, cy - pad});
            positions.push_back({cx - pad, cy});
            positions.push_back({cx + pad, cy});
            positions.push_back({cx - pad, cy + pad});
            positions.push_back({cx + pad, cy + pad});
            break;
        }
        return positions;
    }

    // Handle click
    static gboolean on_click_static(GtkWidget *widget, GdkEventButton *event, gpointer data)
    {
        return static_cast<SpeakerGrillDice *>(data)->on_click(widget, event);
    }

    gboolean on_click(GtkWidget *widget, GdkEventButton *event)
    {
        if (event->type == GDK_BUTTON_PRESS)
        {
            int roll = (std::rand() % 6) + 1;
            g_print("🎲 Rolling... -> %d\n", roll);

            // Show noise first
            show_noise = true;
            if (noise_timer == 0)
                noise_timer = g_timeout_add(80, noise_static, this); // refresh noise

            // After delay, stop noise and animate dice face
            g_timeout_add(1200, [](gpointer data) -> gboolean
                          {
                              auto *self = static_cast<SpeakerGrillDice *>(data);
                              if (self->noise_timer)
                              {
                                  g_source_remove(self->noise_timer);
                                  self->noise_timer = 0;
                              }
                              self->show_noise = false;

                              // prepare final positions
                              int w = self->gtkWidget->allocation.width;
                              int h = self->gtkWidget->allocation.height;
                              auto pos = self->dice_positions(self->last_roll, w, h);

                              self->dots.resize(pos.size());
                              for (size_t i = 0; i < pos.size(); i++)
                              {
                                  // start from center
                                  self->dots[i].x = w / 2.0;
                                  self->dots[i].y = h / 2.0;
                                  self->dots[i].tx = pos[i].first;
                                  self->dots[i].ty = pos[i].second;
                              }

                              if (self->animation_timer == 0)
                                  self->animation_timer = g_timeout_add(16, animate_static, self);

                              return FALSE; // run once
                          },
                          this);

            last_roll = roll;
        }
        return TRUE;
    }

    int last_roll = 1;
};
