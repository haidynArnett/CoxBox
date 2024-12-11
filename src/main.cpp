#include <Arduino.h>
#include <display.hpp>
#include <debouncedButton.hpp>
#include <capacitiveTouchButton.h>
#include <TinyGPSPlus.h>
using namespace CoxBox;
#include <SD.h>
#include <TimerOne.h>

#define SHARP_CLK       19
#define SHARP_DI        20
#define SHARP_CS        21
#define DISPLAY_WIDTH   400
#define DISPLAY_HEIGHT  240
#define HALL_PIN        7
#define GPS_BAUD        9600
#define SD_PIN          10

WorkoutData getWorkoutData();
void readGPS();
String formatTime();


Display display(SHARP_CLK, SHARP_DI, SHARP_CS, DISPLAY_WIDTH, DISPLAY_HEIGHT);
TinyGPSPlus gps;
CapacitiveTouchButton start_button(3);
CapacitiveTouchButton stop_button(2);
CapacitiveTouchButton up_button(5);
CapacitiveTouchButton down_button(4);

enum Screen {
  HOME,
  WORKOUT,
  LOGBOOK,
  LOG
};

vector<Workout> workouts;


void isr() {
  readGPS();
}

volatile double gps_speed;
volatile int num_satellites;
volatile int hour;
volatile int minute;
volatile int second;
volatile int satellites;
volatile double current_latitude;
volatile double current_longitude;
volatile u_int16_t year;
volatile u_int8_t month;
volatile u_int8_t day;
void readGPS() {
  int count = 0;
  // do {
    while (Serial1.available() > 0) {
      if (gps.encode(Serial1.read())) {
        if (gps.location.isValid())
        {
          current_latitude = gps.location.lat();
          current_longitude = gps.location.lng();
        }
        if (gps.speed.isValid()) {
          gps_speed = gps.speed.mps();
        } else {
          gps_speed = -1.0;
        }
        if (gps.satellites.isValid())
          num_satellites = gps.satellites.value();
        if (gps.time.isValid()) {
          hour = gps.time.hour();
          minute = gps.time.minute();
          second = gps.time.second();
        }
        if (gps.date.isValid() && gps.date.isUpdated()) {
          year = gps.date.year();
          month = gps.date.month();
          day = gps.date.day();
        }
      }
    }
    count++;
  // } while (satellites == 0 && count < 100);
  
}


void setup()
{
  // Serial.begin(9600);
  Serial1.begin(GPS_BAUD);
  display.showHomeScreen();
  pinMode(HALL_PIN, INPUT);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(isr);

  if (!SD.begin(SD_PIN)) {
      Serial.println("NO SD CARD!");
  }
  // setup GPS
  // bool valid_location = false;
  // while (!valid_location) {
  //   while (Serial1.available() > 0) {
  //     if (gps.encode(Serial1.read())) {
  //       if (gps.location.isValid())
  //         valid_location = true;
  //     }
  //   }
  // }

}

Screen current_screen = HOME;
int logbook_start = 0;
int logbook_selected = 0;
WorkoutGraphType graph_type = SPLIT;
Workout *current_workout;
void loop()
{
  start_button.takeReading();
  stop_button.takeReading();
  up_button.takeReading();
  down_button.takeReading();
  bool start = start_button.getPressed();
  bool stop = stop_button.getPressed();
  bool up = up_button.getPressed();
  bool down = down_button.getPressed();

  if (current_screen == HOME) {
    display.showHomeScreen();
    if (start) {
      current_screen = WORKOUT;
      current_workout = new Workout(String(year) + "/" + String(month) + "/" + String(day) + "/" + String(hour) + "." + String(minute) + "." + String(second));
    } else if (stop) {
      current_screen = LOGBOOK;
      workouts = Workout::readWorkoutsFromSD();
    }
  } else if (current_screen == WORKOUT) {
    // aquire data for current workout and add to workout object
    WorkoutData workout_data = getWorkoutData();
    current_workout->addData(workout_data);
    display.showCurrentWorkoutData(formatTime(), num_satellites, workout_data);
    if (stop) {
      // save workout to SD card
      current_workout->endWorkout();
      workouts.push_back(*current_workout);
      current_screen = HOME;
    }
  } else if (current_screen == LOGBOOK) {
    display.showLogBook(formatTime(), num_satellites, workouts, logbook_start, logbook_selected);
    if (stop) {
      current_screen = HOME;
    } else if (start) {
      // Serial.println("showing log");
      workouts[logbook_selected].loadData();
      current_screen = LOG;
    } else if (down) {
      logbook_selected++;
      if (logbook_selected == workouts.size()) {
        logbook_selected = 0;
        logbook_start = 0;
      } else if (logbook_selected == logbook_start + 5) {
        logbook_start++;
      }
    } else if (up) {
      logbook_selected--;
      if (logbook_selected == -1) {
        logbook_selected = workouts.size() - 1;
        logbook_start = logbook_selected - 4 >= 0 ? logbook_selected - 4 : 0;
      } else if (logbook_selected == logbook_start - 1) {
        logbook_start--;
      }
    }
  } else if (current_screen == LOG) {
    display.showWorkoutGraph(workouts[logbook_selected], graph_type);
    if (stop) {
      current_screen = LOGBOOK;
      workouts[logbook_selected].data.clear();
    } else if (up || down) {
      if (graph_type == SPLIT)
        graph_type = SPM;
      else
        graph_type = SPLIT;
    }
  }

}

int magnet_detected = 0;
bool magnet_over = false;
unsigned long stroke_start = millis();
int stroke_halves = 0;
int strokes_per_minute;
void calculateSPM(bool magnet_detected) {
  if (magnet_detected && !magnet_over) {
    magnet_over = true;
  } else if (!magnet_detected && magnet_over) {
    magnet_over = false;
    stroke_halves++;
  }
  if (stroke_halves == 2) {
    stroke_halves = 0;
    unsigned long stroke_time_ms = millis() - stroke_start;
    stroke_start = millis();
    float strokes_per_minute_fl = 1.0 / (stroke_time_ms / (60.0 * 1000.0));
    strokes_per_minute = (int)strokes_per_minute_fl;
  }
}

double distance_m;
float last_lat = -1.0;
float last_long = -1.0;
unsigned long last_timestamp;
double calculateSpeed() {
  if (last_lat == -1.0) {
    last_lat = current_latitude;
    last_long = current_longitude;
    return gps_speed;
  }
  double distance_from_last = gps.distanceBetween(last_lat, last_long, current_latitude, current_longitude);
  if (abs(distance_from_last) > 10) {
    distance_m += abs(distance_from_last);
    last_lat = current_latitude;
    last_long = current_longitude;
    return 1.0;
  } else {
    return -1.0;
  }
}

String formatTime() {
  String hour_s = (hour - 5 + 12) % 12;
  String minute_s = minute >= 10 ? String(minute) : "0" + String(minute);
  String am_s = (hour - 5 + 24) % 24 > 11 ? " PM" : " AM";
  return hour_s + ":" + minute_s + am_s;
}

WorkoutData getWorkoutData() {
  // get split from GPS
  // get stroke rate from sensor
  calculateSPM(1 - digitalRead(HALL_PIN));
  calculateSpeed();
  return {
    gps_speed, // speed mps
    strokes_per_minute, // stroke rate
    millis() - current_workout->start, // current duration
    distance_m // distance
  };
}
