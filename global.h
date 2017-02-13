#define CMD_BRIGHTNESS 0x10
#define CMD_RAW        0x11
#define CMD_BAUD_RATE  0x12
#define CMD_FACTORY_RESET 0x13

#define MODE_TIME 0
#define MODE_GPS  1
#define MODE_EPOCH 2

uint8_t _mode = MODE_EPOCH;

#define LOOP_TIMEOUT 500

#define GPS_VIEWS 3
uint8_t _gps_view = 0;
#define GPS_CYCLES 5
uint8_t _gps_cycle = 0;
