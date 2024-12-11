#include "../include/workout.h"


namespace CoxBox {
    int logging_period_ms = 500;
    unsigned long last_log;
    File workouts_file;
    File current_workout_file;

    String workoutDataToString(WorkoutData data) {
        return String(data.distance_m, 7) + "," + String(data.speed_mps, 7) + "," + String(data.spm) + "," + String(data.time_elapsed);
    }
    WorkoutData stringToWorkoutData(String data_string) {
        int comma1 = data_string.indexOf(',');
        int comma2 = data_string.indexOf(',', comma1 + 1);
        int comma3 = data_string.indexOf(',', comma2 + 1);
        WorkoutData data;
        data.distance_m = data_string.substring(0, comma1).toFloat();
        data.speed_mps = data_string.substring(comma1 + 1, comma2).toFloat();
        data.spm = data_string.substring(comma2 + 1, comma3).toInt();
        data.time_elapsed = data_string.substring(comma3 + 1).toInt();
        return data;
    }
    

    Workout::Workout(String name) : name(name) {
        this->start = millis();
        last_log = this->start;
        this->max_speed_mps = 0.0;
        this->min_speed_mps = 0.0;
        this->max_spm = 0;
        this->min_spm = 0;

        SD.mkdir(name.c_str());
        current_workout_file = SD.open((this->name + "/data.txt").c_str(), FILE_WRITE);
    }

    float spm_sum = 0.0;
    void Workout::addData(WorkoutData data) {
        
        
        if (millis() - last_log >= logging_period_ms) {
            this->data_point_count++;
            last_log = millis();

            current_workout_file.println(workoutDataToString(data));

            if (data.speed_mps != -1.0) {
                this->distance_m += data.distance_m;
            }
            spm_sum += data.spm;
            this->distance_m += data.distance_m;
            if (this->max_speed_mps == 0.0 || data.speed_mps > this->max_speed_mps)
                this->max_speed_mps = data.speed_mps;
            if (this->min_speed_mps == 0.0 || data.speed_mps < this->min_speed_mps)
                this->min_speed_mps = data.speed_mps;
            if (this->max_spm == 0 || data.spm > this->max_spm)
                this->max_spm = data.spm;
            // if (this->min_spm == 0 || data.spm < this->min_spm)
            //     this->min_spm = data.spm;
        }
    }

    void Workout::loadData() {
        Serial.println("loading data");
        current_workout_file = SD.open((this->name + "/data.txt").c_str());
        while (current_workout_file.available()) {
            this->data.push_back(stringToWorkoutData(current_workout_file.readStringUntil('\n')));
        }
        current_workout_file.close();
        Serial.println("loaded " + String(this->data.size()) + " data points to " + this->name);
    }

    void Workout::endWorkout() {
        this->end = millis();
        this->duration = millisToTimeString(this->end - this->start);
        this->mean_speed_mps = this->distance_m / (1000.0 * (this->end - this->start));
        this->mean_spm = spm_sum / this->data_point_count;

        current_workout_file.close();
        workouts_file = SD.open("workouts.txt", FILE_WRITE);
        workouts_file.println(this->fileSummary());
        workouts_file.close();
    }

    String Workout::fileSummary() {
        return this->name + "," + 
            String(this->distance_m) + "," + 
            this->duration + "," + 
            String(this->max_speed_mps, 7) + "," + 
            String(this->min_speed_mps, 7) + "," + 
            String(this->mean_speed_mps, 7) + "," + 
            this->max_spm + "," + 
            this->min_spm + "," + 
            String(this->mean_spm, 7);
    }

    Workout Workout::parseFileSummary(String file_summary) {
        Serial.println("file summary: " + file_summary);
        int comma1 = file_summary.indexOf(',');
        int comma2 = file_summary.indexOf(',', comma1 + 1);
        int comma3 = file_summary.indexOf(',', comma2 + 1);
        int comma4 = file_summary.indexOf(',', comma3 + 1);
        int comma5 = file_summary.indexOf(',', comma4 + 1);
        int comma6 = file_summary.indexOf(',', comma5 + 1);
        int comma7 = file_summary.indexOf(',', comma6 + 1);
        int comma8 = file_summary.indexOf(',', comma7 + 1);
        String name = file_summary.substring(0, comma1);
        Workout workout(name);
        workout.distance_m = file_summary.substring(comma1 + 1, comma2).toFloat();
        workout.duration = file_summary.substring(comma2 + 1, comma3);
        workout.max_speed_mps = file_summary.substring(comma3 + 1, comma4).toFloat();
        workout.min_speed_mps = file_summary.substring(comma4 + 1, comma5).toFloat();
        workout.mean_speed_mps = file_summary.substring(comma5 + 1, comma6).toFloat();
        workout.max_spm = file_summary.substring(comma6 + 1, comma7).toFloat();
        workout.min_spm= file_summary.substring(comma7 + 1, comma8).toFloat();
        workout.mean_spm = file_summary.substring(comma8 + 1).toFloat();
        return workout;
    }

    vector<Workout> Workout::readWorkoutsFromSD() {
        vector<Workout> workouts;
        workouts_file = SD.open("workouts.txt");
        Serial.println("opened workouts file");
        if (workouts_file) {
            while (workouts_file.available()) {
                workouts.push_back(parseFileSummary(workouts_file.readStringUntil('\n')));
            }
        } else {
            Serial.println("error opening workouts.txt");
        }
        workouts_file.close();
        return workouts;
    }

    String Workout::mpsToSplit(float mps) {
        if (mps == -1.0)
            return "-";
        float seconds_per_m = 1.0 / mps;
        float seconds_per_500m = seconds_per_m * 500.0;
        int mins = (int)(seconds_per_500m / 60);
        int secs = (int)seconds_per_500m % 60;
        String mins_string = mins >= 10 ? String(mins) : String(mins);
        String secs_string = secs >= 10 ? String(secs) : "0" + String(secs);
        return mins_string + ":" + secs_string;
    }

    String Workout::millisToTimeString(unsigned long millis) {
        int hours = millis / (60 * 60 * 1000);
        String hours_string = hours == 0 ? "" : String(hours) + ":";
        int minutes = (millis % (60 * 60 * 1000)) / (60 * 1000);
        String minutes_string = minutes >= 10 || hours == 0 || hours == 0 && minutes == 0 ? String(minutes) + ":" : "0" + String(minutes) + ":";
        int seconds = (millis % (60 * 1000)) / 1000;
        String seconds_string = seconds >= 10 ? String(seconds) : "0" + String(seconds);
        return hours_string + minutes_string + seconds_string;
    }
}