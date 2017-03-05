#define CMD_BRIGHTNESS 0x10
#define CMD_RAW        0x11
#define CMD_BAUD_RATE  0x12
#define CMD_FACTORY_RESET 0x13

#define MODE_COUNT 3
#define MODE_TIME 0
#define MODE_GPS  1
#define MODE_EPOCH 2

uint8_t _mode = MODE_EPOCH;

#define LOOP_TIMEOUT 500

#define GPS_VIEWS 3
uint8_t _gps_view = 0;
#define GPS_CYCLES 5
uint8_t _gps_cycle = 0;

#define BTN_GROUND 17
#define BTN_1 15
#define BTN_2 16
#define BTN_BOUNCE_TIME 100
long _btn_1 = 0;
long _btn_2 = 0;

#define BRIGHT_MAX 10
uint8_t _brightness = BRIGHT_MAX;

bool _disp_on = true;
long _disp_on_time = 0;
#define DISP_TIMEOUT 30000
