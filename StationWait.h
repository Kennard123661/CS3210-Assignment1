//
// Created by kennard on 25/09/18.
//

#ifndef CS3210_ASSIGNMENT1_STATIONWAIT_H
#define CS3210_ASSIGNMENT1_STATIONWAIT_H

#endif //CS3210_ASSIGNMENT1_STATIONWAIT_H

typedef struct {
    float total_wait_time;
    unsigned int min_wait_time;
    unsigned int max_wait_time;
    unsigned int num_trains_arrive;
    unsigned int prev_time_stamp;
} StationWait;

void train_arrive(unsigned int time, StationWait* station_wait);
void train_leave(unsigned int time, StationWait* stationWait);
