//
// Created by kennard on 22/09/18.
//

#include "StationWait.h"

#ifndef CS3210_ASSIGNMENT1_LINENETWORK_H
#define CS3210_ASSIGNMENT1_LINENETWORK_H

#endif //CS3210_ASSIGNMENT1_LINENETWORK_H


typedef struct {
    unsigned int num_nodes;
    unsigned int* station_numbers;
    StationWait* station_waits;
} LineNetwork;

unsigned int get_next_node_index(LineNetwork* network, unsigned int index);
unsigned int get_station_idx(LineNetwork *network, unsigned int index);
LineNetwork* get_line_network(
        char** line_station_names,
        char** all_station_names,
        unsigned int num_stations,
        unsigned int num_stations_in_line
);
void delete_line_network(LineNetwork* network);
