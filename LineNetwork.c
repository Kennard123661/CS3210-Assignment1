//
// Created by kennard on 22/09/18.
//

#include <malloc.h>
#include <string.h>
#include "LineNetwork.h"

/**
 * Gets the next node index location.
 * */
unsigned int get_next_node_index(LineNetwork* network, unsigned int index)
{
    return (index + 1) % network->num_nodes;
}

/**
 * Gets the station number of a train at the current index in the train network.
 * */
unsigned int get_station_idx(LineNetwork *network, unsigned int index)
{
    return network->station_numbers[index];
}

/**
 *  This builds  a network of train stations for a single line.
 *
 *  For example, suppose t1 -> t2 -> t3 -> t4;
 *  This network builds t1 -> t2 -> t3 -> t4 -> t3 -> t2
 *  Once the network reaches t1 i.e. index = (n - 1) * 2 - 1, the network wraps around like a circular buffer back to
 *  t1; This achieves the effect of a cycle.
 */
LineNetwork* get_line_network(
        char** line_station_names,
        char** all_station_names,
        unsigned int num_stations,
        unsigned int num_stations_in_line
        )
{
    unsigned int* station_nums = malloc(sizeof(unsigned int) * (num_stations_in_line - 1) * 2);
    for (unsigned int i = 0; i < num_stations_in_line; i++) {
        for (unsigned int j = 0; j < num_stations; j++) {
            if (strcmp(line_station_names[i], all_station_names[j]) == 0) {
                station_nums[i] = j;
                break;
            }
        }
    }

    for (unsigned int i = num_stations_in_line; i < (num_stations_in_line - 1) * 2; i++) {
        station_nums[i] = station_nums[(num_stations_in_line- 1) * 2 - i];
    }

    LineNetwork* network = malloc(sizeof(LineNetwork));
    network->num_nodes = (num_stations_in_line - 1) * 2;
    network->station_numbers = station_nums;

    return network;
}

/**
 * Deletes the Line network and frees up allocated memory.
 */
void delete_line_network(LineNetwork* network)
{
    free(network->station_numbers);
    free(network);
    network = NULL;
}
