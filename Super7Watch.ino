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
        return makeTime(gps_time) + TZ_OFFSET;
    }

    Serial.println("NO FIX");
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

    S7.write(CMD_BRIGHTNESS);
    S7.write(10);
    S7.write('\n');

    Timer1.initialize(1000);
    Timer1.attachInterrupt(gps_read);

    setSyncProvider(getGPSTime);
    setSyncInterval(10);
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
    Serial.print(":");
    if (digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

String zp(uint8_t val){
    String r;
    if(val < 10) r += '0';
    return r + String(val);
}

void loop()
{

    if (GPS.newNMEAreceived()) {
        if (!GPS.parse(GPS.lastNMEA()))
            return;
    }

    if (timer > millis()) timer = millis();

    if (millis() - timer > 500)
    {
        timer = millis(); // reset the timer

        Serial.print(hour());
        printDigits(minute());
        printDigits(second());
        Serial.print(" ");
        Serial.print(month());
        Serial.print(".");
        Serial.print(day());
        Serial.print(".");
        Serial.print(year());
        Serial.println();

        String t = zp(hour()) +
                   '.' + zp(minute()) +
                   '.' + zp(second());
        String d(dayShortStr(weekday()));
        S7.print(" " + d + " " + t + "\n");
        Serial.println(d);

        // S7.print(hour());
        // S7.print('.');
        // S7.print(minute());
        // S7.print('.');
        // S7.print(second());
        // S7.print('\n');


        Serial.print("Fix: "); Serial.print((int)GPS.fix);
        Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
        if (GPS.fix) {
            Serial.print("Location: ");
            static uint8_t lat, lon;
            static float f_lat, f_lon;
            lat = int(GPS.latitude / 100);
            f_lat = GPS.latitude - (lat * 100);
            lon = int(GPS.longitude / 100);
            f_lon = GPS.longitude - (lon * 100);
            String s_lat;
            if(GPS.lat == 'S')
                s_lat += '-';
            s_lat += String(lat); s_lat += " "; s_lat += String(f_lat, 4);
            Serial.print(s_lat);
            Serial.print(", ");
            String s_lon;
            if(GPS.lon == 'W')
                s_lon += '-';
            s_lon += String(lon); s_lon += " "; s_lon += String(f_lon, 4);
            Serial.print(s_lon);
            Serial.println("");

            // Serial.print(lat); Serial.print(" "); Serial.print(f_lat, 4);
            // Serial.print(", ");
            // if(GPS.lon == 'W') Serial.print('-');
            // Serial.print(lon); Serial.print(" "); Serial.print(f_lon, 4);
            // Serial.println("");
            Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
            Serial.print(", ");
            Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
            Serial.print("Speed (knots): "); Serial.println(GPS.speed);
            Serial.print("Angle: "); Serial.println(GPS.angle);
            Serial.print("Altitude: "); Serial.println(GPS.altitude);
            Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
        }

        Serial.println("");
    }
    // // if millis() or timer wraps around, we'll just reset it
    // if (timer > millis()) timer = millis();
    //
    // // approximately every 2 seconds or so, print out the current stats
    // if (millis() - timer > 2000) {
    //     timer = millis(); // reset the timer
    //     if(GPS.fix){
    //         Serial.print("\nTime: ");
    //         Serial.print(GPS.hour, DEC); Serial.print(':');
    //         Serial.print(GPS.minute, DEC); Serial.print(':');
    //         Serial.print(GPS.seconds, DEC); Serial.print('.');
    //         Serial.println(GPS.milliseconds);
    //         Serial.print("Date: ");
    //         Serial.print(GPS.day, DEC); Serial.print('/');
    //         Serial.print(GPS.month, DEC); Serial.print("/20");
    //         Serial.println(GPS.year, DEC);
    //         Serial.print("Fix: "); Serial.print((int)GPS.fix);
    //         Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    //         if (GPS.fix) {
    //             Serial.print("Location: ");
    //             Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    //             Serial.print(", ");
    //             Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    //             Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    //             Serial.print("Angle: "); Serial.println(GPS.angle);
    //             Serial.print("Altitude: "); Serial.println(GPS.altitude);
    //             Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    //         }
    //     }
    //     else{
    //         Serial.println("NO FIX");
    //     }
    //
    // }
}
