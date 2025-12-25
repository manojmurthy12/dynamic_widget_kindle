#pragma once
#include "SpeakerGrill.h"

class SpeakerGrillCounter : public SpeakerGrill
{
    unsigned int timer_id;
    int elapsed_seconds;
    int total_seconds = 60;
    bool resetTimerAfterDone = true;
    bool syncWithClock = true;
    int prev_filled_dots = 0;

public:
    SpeakerGrillCounter(int col_, int row_,
                        int width_blocks_, int height_blocks_,
                        int radius_,
                        int total_blocks_x_ = 4, int total_blocks_y_ = 4)
        : SpeakerGrill(col_, row_, width_blocks_, height_blocks_, radius_,
                       total_blocks_x_, total_blocks_y_),
          timer_id(0), elapsed_seconds(0)
    {
        // Start timer: 1 second interval
        timer_id = g_timeout_add(1000, on_tick_static, this);
    }

    ~SpeakerGrillCounter()
    {
        if (timer_id > 0)
            g_source_remove(timer_id);
    }

private:
    static gboolean on_tick_static(gpointer data)
    {
        return static_cast<SpeakerGrillCounter *>(data)->on_tick();
    }

    gboolean on_tick()
    {
        int seconds = 0;

        if (syncWithClock)
        {
            // ✅ Sync with wall clock
            time_t now = time(NULL);
            struct tm *lt = localtime(&now);
            seconds = lt->tm_sec + 1; // 0–59
        }
        else
        {
            // ✅ Relative mode
            elapsed_seconds++;
            if (elapsed_seconds > total_seconds)
                elapsed_seconds = total_seconds; // clamp at 60
            seconds = elapsed_seconds;
        }

        if (total_dots > 0)
        {
            double dots_per_second = static_cast<double>(total_dots) / 60.0;
            filled_dots = static_cast<int>(floor(seconds * dots_per_second));

            // Clamp in case rounding overshoots
            if (filled_dots > total_dots)
                filled_dots = total_dots;
            if(filled_dots > prev_filled_dots)
            {
                gtk_widget_queue_draw(gtkWidget);
            }
            prev_filled_dots = filled_dots;
        }

        // reset timer after total_seconds
        if (!syncWithClock && seconds == total_seconds)
        {
            seconds = 0;
            gtk_widget_queue_draw(gtkWidget);
        }
        else if(!resetTimerAfterDone)
        {
            return false;
        }
        return true;
    }
};
