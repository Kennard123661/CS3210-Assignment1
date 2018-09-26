//
// Created by kennard on 25/09/18.
//

#ifndef CS3210_ASSIGNMENT1_STATIONWAIT_H
#define CS3210_ASSIGNMENT1_STATIONWAIT_H

#endif //CS3210_ASSIGNMENT1_STATIONWAIT_H

typedef struct {
    float average_wait_time;// = 0;
    unsigned int min_wait_time;// = 0xFFFFFFFF; // hex code for maximum value that can be represented by 32 bits.
    unsigned int max_wait_time;// = 0;
    unsigned int num_trains_arrive;// = 0;
    unsigned int prev_time_stamp;// = 0;
} StationWait;

void train_arrive(unsigned int time, StationWait* station_wait);
void train_leave(unsigned int time, StationWait* stationWait);
StationWait* get_station_wait();