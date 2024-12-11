#include "../include/display.hpp"

#define BLACK 0
#define WHITE 1
#define char_width 6
#define data_size 4
#define word_size 2

namespace CoxBox {
    Display::Display(uint8_t CLK, uint8_t DI, uint8_t CS, uint16_t width, uint16_t height)
        : display(CLK, DI, CS, width, height)
    {
        display.begin();
        display.clearDisplay();
        display.cp437(true);
        display.setTextColor(BLACK);
    }

    void Display::showHomeScreen() {
        display.clearDisplayBuffer();
        display.setTextSize(10);
        display.setCursor(25,50);
        display.printf("CoxBox");

        display.drawLine(0, 180, display.width() - 1, 180, BLACK);
        display.drawLine(200, 180, 200, display.height() - 1, BLACK);

        display.setTextSize(word_size);
        display.setCursor(100 - (7 * char_width * word_size / 2), 205);
        display.printf("LOGBOOK");

        display.setCursor(300 - (5 * char_width * word_size / 2), 205);
        display.printf("START");

        display.refresh();
    }

    void Display::showLogBook(String time, int num_satellites, vector<Workout> workouts, unsigned int start_index, unsigned int selected_index) {
        display.clearDisplayBuffer();
        this->showTopBanner(time, num_satellites);
        if (workouts.size() == 0) {
            display.setTextSize(data_size);
            display.setCursor(20,70);
            display.printf("No workouts yet");
        } else {
            // highlight selected workout
            display.fillRect(0, (selected_index - start_index + 1) * 40, display.width(), 40, BLACK);

            // display all workouts
            for (unsigned int i=start_index; i < start_index + 5 && i < workouts.size(); i++) {
                if (i == selected_index)
                    display.setTextColor(WHITE);

                display.setTextSize(word_size);
                display.setCursor(100 - (workouts[i].name.length() * char_width * word_size / 2), 40 + 15 + (i - start_index) * 40);
                display.printf(workouts[i].name.c_str());
                
                display.setCursor(250 - ((String(workouts[i].distance_m).length() + 1) * char_width * word_size / 2), 40 + 15 + (i - start_index) * 40);
                display.printf("%dm", workouts[i].distance_m);

                display.setCursor(350 - (workouts[i].duration.length() * char_width * word_size / 2), 40 + 15 + (i - start_index) * 40);
                display.printf(workouts[i].duration.c_str());

                display.drawLine(0, (i - start_index + 2) * 40, display.width() - 1, (i - start_index + 2) * 40, BLACK);


                if (i == selected_index)
                    display.setTextColor(BLACK);
            }
        }
        display.refresh();
    }

    void Display::showWorkoutGraph(Workout workout, WorkoutGraphType type) {
        display.clearDisplayBuffer();

        display.setTextSize(word_size);
        display.setCursor(160 - (workout.name.length() * char_width * word_size / 2), 15);
        display.printf(workout.name.c_str());
        display.drawLine(0, 40, display.width() - 1, 40, BLACK);

        String type_text;
        if (type == SPLIT)
            type_text = "SPLIT";
        else
            type_text = "SPM";
        display.setCursor(360 - (type_text.length() * char_width * word_size / 2), 15);
        display.printf(type_text.c_str());
        display.drawLine(320, 0, 320, 40, BLACK);

        int graph_width = 365;
        int num_values = workout.data.size();

        // draw baseline
        display.drawLine(15, 235, 395, 235, BLACK);

        // draw markings
        display.drawLine(0, 45, 30, 45, BLACK);
        int mean_height;
        if (type == SPLIT)
            mean_height = (int)(190.0 * (workout.mean_speed_mps - workout.min_speed_mps) / (workout.max_speed_mps - workout.min_speed_mps));
        else
            mean_height = (int)(190.0 * (workout.mean_spm - workout.min_spm) / (workout.max_spm - workout.min_spm));
        display.drawLine(0, 235 - mean_height, 30, 235 - mean_height, BLACK);
        display.drawLine(0, 235, 30, 235, BLACK);
        display.setTextSize(1);
        if (type == SPLIT) {
            display.setCursor(3, 48);
            display.print(Workout::mpsToSplit(workout.max_speed_mps));

            display.setCursor(3, mean_height + 5);
            display.print(Workout::mpsToSplit(workout.mean_speed_mps));

            display.setCursor(3, 225);
            display.print(Workout::mpsToSplit(workout.min_speed_mps));
        } else {
            display.setCursor(3, 50);
            display.printf("%d", workout.max_spm);

            display.setCursor(3, 235 - mean_height + 5);
            Serial.println("mean spm: " + String(mean_height));
            display.printf("%d", (int)workout.mean_spm);

            display.setCursor(3, 225);
            display.printf("%d", workout.min_spm);
        }

        for (int i = 0; i < graph_width; i++) {
            float percent_through = i / (float)graph_width;
            float next_percent_through = (i + 1) / (float)graph_width;

            float data_value_for_pixel;
            if (num_values <= graph_width) {
                if (type == SPLIT)
                    data_value_for_pixel = workout.data[(int)(percent_through * num_values)].speed_mps;
                else
                    data_value_for_pixel = workout.data[(int)(percent_through * num_values)].spm;
            } else {
                float sum = 0;
                int count = 0;
                for (int j = percent_through * num_values; j < next_percent_through * num_values; j++) {
                    if (type == SPLIT) {
                        if (workout.data[j].speed_mps == -1.0)
                            continue;
                        sum += workout.data[j].speed_mps;
                    } else {
                        sum += workout.data[j].spm;
                    }
                    count++;
                }
                data_value_for_pixel = count == 0 ? 0 : sum / count;
            }
            int height; // [0, 190]
            if (type == SPLIT)
                height = (int)(190.0 * (data_value_for_pixel - workout.min_speed_mps) / (workout.max_speed_mps - workout.min_speed_mps));
            else
                height = (int)(190.0 * (data_value_for_pixel - workout.min_spm) / (workout.max_spm - workout.min_spm));
            display.drawLine(30 + i, 235 - height, 30 + i, 235, BLACK);
        }

        display.refresh();
    }

    void Display::showCurrentWorkoutData(String time, int num_satellites, WorkoutData workout_data) {
        display.clearDisplayBuffer();
        this->showTopBanner(time, num_satellites);
        display.drawLine(0, 140, this->display.width() - 1, 140, BLACK);
        display.drawLine(display.width() / 2, 40, display.width() / 2, display.height(), BLACK);

        // display split
        display.setTextSize(data_size);
        String split = Workout::mpsToSplit(workout_data.speed_mps);
        display.setCursor(100 - ((split.length() * char_width * data_size) / 2),70);
        display.printf(split.c_str());

        display.setTextSize(word_size);
        display.setCursor(140,124);
        display.printf("SPLIT");

        // display spm
        display.setTextSize(data_size);
        display.setCursor(300 - ((String(workout_data.spm).length() * char_width * data_size) / 2), 70);
        display.printf("%d", workout_data.spm);

        display.setTextSize(word_size);
        display.setCursor(364,124);
        display.printf("SPM");

        // display time elapsed
        String time_elapsed = Workout::millisToTimeString(workout_data.time_elapsed);
        display.setTextSize(data_size);
        display.setCursor(100 - ((time_elapsed.length() * char_width * data_size) / 2),170);
        display.printf(time_elapsed.c_str());

        display.setTextSize(word_size);
        display.setCursor(56,224);
        display.printf("TIME ELAPSED");

        // display total distance
        display.setTextSize(data_size);
        display.setCursor(300 - ((String((int)workout_data.distance_m).length() * char_width * data_size) / 2),170);
        display.printf("%d", (int)workout_data.distance_m);

        display.setTextSize(word_size);
        display.setCursor(268,224);
        display.printf("DISTANCE(m)");

        display.refresh();
    }


    void Display::showTopBanner(String time, int num_satellites) {
        display.drawLine(0, 40, this->display.width() - 1, 40, BLACK);

        // display time
        display.setTextSize(word_size);
        display.setCursor(100 - (time.length() * char_width * word_size / 2), 15);
        display.printf(time.c_str());

        // display number of satellites
        display.setTextSize(word_size);
        display.setCursor(300 - (((String(num_satellites).length() + 11) * char_width * word_size) / 2), 15);
        display.printf("%d satellites", num_satellites);

    }
}