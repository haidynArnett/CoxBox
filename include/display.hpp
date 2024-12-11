#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include "../include/workout.h"
using namespace std;

namespace CoxBox {
    enum WorkoutGraphType {
        SPLIT,
        SPM
    };

    class Display {
        public:
            Display(uint8_t CLK, uint8_t DI, uint8_t CS, uint16_t width, uint16_t height);
            void showHomeScreen();
            void showLogBook(String time, int num_satellites, vector<Workout> workouts, unsigned int start_index, unsigned int selected_index);
            void showWorkoutGraph(Workout workout, WorkoutGraphType type);
            void showCurrentWorkoutData(String time, int num_satellites, WorkoutData workout_data);

        private:
            Adafruit_SharpMem display;
            void showTopBanner(String time, int num_satellites);
    };
}

#endif