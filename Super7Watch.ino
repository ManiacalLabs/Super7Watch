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

void setup()
{
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
        Serial.println(out);

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
