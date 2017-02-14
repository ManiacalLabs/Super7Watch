#include "global.h"
#include "TimerOne.h"
#include "TimeLib.h"
#include "Adafruit_GPS.h"
#include <SoftwareSerial.h>

#define TIME_ZONE -5
#define TZ_OFFSET TIME_ZONE * SECS_PER_HOUR
// what's the name of the hardware serial port?
#define GPSSerial Serial1

SoftwareSerial S7 =  SoftwareSerial(10, 9);

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

uint32_t timer = millis();

time_t getGPSTime()
{
    static uint8_t hr, min, sec, yr, mnth, dy;
    static tmElements_t gps_time;
    if(GPS.fix){
        hr = GPS.hour;
        min = GPS.minute;
        sec = GPS.seconds;
        yr = GPS.year;
        mnth = GPS.month;
        dy = GPS.day;

        if( yr > 99)
          yr = yr - 1970;
        else
          yr += 30;
        gps_time.Year = yr;
        gps_time.Month = mnth;
        gps_time.Day = dy;
        gps_time.Hour = hr;
        gps_time.Minute = min;
        gps_time.Second = sec;
        Serial.println("Set Time");
        return makeTime(gps_time) + TZ_OFFSET;
    }

    return 0; // return 0 if unable to get the time
}

void gps_read(){
    GPS.read();
}

void BTN_1_Press(){
    if(_btn_1 == 0)
        _btn_1 = millis();
}

void BTN_2_Press(){
    if(_btn_2 == 0)
        _btn_2 = millis();
}

void setup()
{
    pinMode(BTN_GROUND, OUTPUT);
    digitalWrite(BTN_GROUND, LOW);

    pinMode(BTN_1, INPUT_PULLUP);
    pinMode(BTN_2, INPUT_PULLUP);

    Serial.begin(115200);

    S7.begin(38400);

    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
    GPS.sendCommand(PGCMD_ANTENNA); // antenna status

    delay(1000);

    Timer1.initialize(1000);
    Timer1.attachInterrupt(gps_read);

    setSyncProvider(getGPSTime);
    setSyncInterval(10);

    attachInterrupt(digitalPinToInterrupt(BTN_1), BTN_1_Press, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_2), BTN_2_Press, FALLING);
}

String zp(uint8_t val){
    String r;
    if(val < 10) r += '0';
    return r + String(val);
}

String _pad(String s, char pad, int count){
    if(count){
        for(int i=0; i<count; i++) s = pad + s;
    }
    return s;
}
String left_pad(String s, char pad, int width){
    width = width - s.length();
    return _pad(s, pad, width);
}

String center_text(String s, char pad, int width){
    width = (width - s.length()) / 2;

    return _pad(s, pad, width);
}

inline void show_time(){
    String t = zp(hour()) +
               '.' + zp(minute()) +
               '.' + zp(second());
    String d(dayShortStr(weekday()));
    t = " " + d + " " + t + "\n";
    S7.print(t);
}

inline void show_epoch_time(){
    S7.write(" ");
    S7.print(now());
    S7.write("\n");
}

inline void show_gps(){
    if(GPS.fix){
        String out;
        if(_gps_view == 0){
            out = String(GPS.latitudeDegrees, 8);
            //13 because . doesn't count on the S7
            out = left_pad(out, ' ', 13);
        }
        else if(_gps_view == 1){
            out = String(GPS.longitudeDegrees, 8);
            //13 because . doesn't count on the S7
            out = left_pad(out, ' ', 13);
        }
        else if(_gps_view == 2){
            out = ("SC" + String((int)GPS.satellites));
            out += (" SP" + String(int(GPS.speed * 1.1508)));
            out = center_text(out, ' ', 12);
        }

        S7.print(out + "\n");
        // Serial.println(out);

        _gps_cycle++;
        if(_gps_cycle >= GPS_CYCLES){
            _gps_cycle = 0;
            _gps_view++;
            if(_gps_view >= GPS_VIEWS)
                _gps_view = 0;
        }
    }
    else{
        S7.write("no fix\n");
    }
}

void loop()
{
    if(_btn_1 > 0 && (millis() - _btn_1) > BTN_BOUNCE_TIME){
        _btn_1 = 0;
        if(digitalRead(BTN_1) == LOW){
            _mode++;
            if(_mode >= MODE_COUNT) _mode = 0;

            // In case enter GPS mode.
            // Always start with lat
            _gps_view = 0;
        }
    }

    if(_btn_2 > 0 && (millis() - _btn_2) > BTN_BOUNCE_TIME){
        _btn_2 = 0;
        if(digitalRead(BTN_2) == LOW){
            _brightness += 2;
            if(_brightness > BRIGHT_MAX)
                _brightness = 2;

            S7.write(CMD_BRIGHTNESS);
            S7.write(_brightness);
            S7.write('\n');
        }
    }

    if (GPS.newNMEAreceived()) {
        if (!GPS.parse(GPS.lastNMEA()))
            return;
    }

    // Reset if overflow
    if (timer > millis()) timer = millis();

    if (millis() - timer > LOOP_TIMEOUT)
    {
        timer = millis(); // reset the timer

        switch (_mode) {
            case MODE_TIME:
                show_time();
                break;
            case MODE_GPS:
                show_gps();
                break;
            case MODE_EPOCH:
                show_epoch_time();
                break;
        }
    }
}
