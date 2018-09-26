//
// Created by kennard on 25/09/18.
//
#include "StationWait.h"

void train_arrive(unsigned int time, StationWait* station_wait)
{
    unsigned int time_diff = time - station_wait->prev_time_stamp;
    station_wait->num_trains_arrive++;

    // incremental average
    station_wait->average_wait_time = station_wait->average_wait_time * ((float) station_wait->num_trains_arrive - 1) /
                                      (station_wait->num_trains_arrive) + ((float) time) / station_wait->num_trains_arrive;
    station_wait->max_wait_time = (time > station_wait->max_wait_time) ? time : station_wait->max_wait_time;
    station_wait->min_wait_time = (time < station_wait->min_wait_time) ? time : station_wait->min_wait_time;
}

void train_leave(unsigned int time, StationWait* station_wait) {
    station_wait->prev_time_stamp = station_wait;
}