//
// Created by kennard on 25/09/18.
//
#include "StationWait.h"

void train_arrive(unsigned int time, StationWait* station_wait)
{
    unsigned int time_diff = time - station_wait->prev_time_stamp;
    station_wait->num_trains_arrive++;
    station_wait->total_wait_time = station_wait->total_wait_time + time_diff;
    station_wait->max_wait_time = (time_diff > station_wait->max_wait_time) ? time_diff : station_wait->max_wait_time;
    station_wait->min_wait_time = (time_diff < station_wait->min_wait_time) ? time_diff : station_wait->min_wait_time;
}

void train_leave(unsigned int time, StationWait* station_wait) {
    station_wait->prev_time_stamp = time;
}