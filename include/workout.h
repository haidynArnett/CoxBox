#ifndef WORKOUT_H
#define WORKOUT_H


#include <vector>
#include <Arduino.h>
#include <SD.h>
using namespace std;

namespace CoxBox {
    struct WorkoutData {
        double speed_mps;
        int spm;
        unsigned long time_elapsed;
        double distance_m;
    };

    class Workout {
        public:
            String name;
            String duration;
            unsigned long start;
            unsigned long end;
            float distance_m;
            vector<WorkoutData> data;
            float max_speed_mps;
            float min_speed_mps;
            float mean_speed_mps;
            int max_spm;
            int min_spm;
            float mean_spm;
            Workout(String name);
            void addData(WorkoutData data);
            void loadData();
            void endWorkout();
            String fileSummary();
            static Workout parseFileSummary(String file_summary);
            static vector<Workout> readWorkoutsFromSD();
            static String mpsToSplit(float mps);
            static String millisToTimeString(unsigned long millis);

        private:
            unsigned long data_point_count;
    };
}

#endif