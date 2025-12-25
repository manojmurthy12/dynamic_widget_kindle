#include <fstream>
#include <string>

class HapticFeedback {
public:
    enum Effect {
        SHARP_CLICK = 1,
        LONG_BUZZ = 2
    };

    static void play(Effect effect = SHARP_CLICK) {
        const char* path = "/sys/devices/system/drv26xx_haptics/drv26xx_haptics0/play_waveform";
        std::ofstream f(path);
        if (f.is_open()) {
            f << std::to_string(static_cast<int>(effect));
            f.close();
        }
    }
    static void play_sequence(const std::vector<Effect>& ids, int gap_ms) {
        for (Effect id : ids) {
            play(id);
            // We must sleep so the chip has time to finish the waveform 
            // before the next command arrives
            usleep(gap_ms * 1000); 
        }
    }
};