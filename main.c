#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LineNetwork.h"
#include "Train.h"

#define STATION_NAMES_BUFFER_SIZE 8196
#define STATION_NAME_BUFFER_SIZE 256

char** get_station_names(char* input, unsigned int num_stations);
unsigned int** get_station_link_costs(unsigned int num_stations);
float* get_station_popularity(unsigned int num_stations);
char*** get_stations_in_lines(unsigned int num_lines, unsigned int* num_stations_per_line);
unsigned int* get_num_trains_per_line(unsigned int num_lines);
void run_train();

int main() {
    const unsigned int TRAIN_SPEED = 1;
    const unsigned int NUM_LINES = 3;
    const char LINE_PREFIXES[] = {'g','y','b'};
    const char* STATION_PRINT_TEMPLATE = "%s%u-s%u";
    const char* LINK_PRINT_TEMPLATE = "%s%u-s%u->s%u";
    const char* TIME_UPDATE_PREFIX = "%u: ";

    unsigned int num_stations;
    scanf("%u\n", &num_stations);
    printf("%u\n", num_stations);

    char raw_stations_string[STATION_NAMES_BUFFER_SIZE];
    scanf("%s", raw_stations_string);

    char** station_names = NULL;
    station_names = get_station_names(raw_stations_string, num_stations);
    for (int i = 0; i < num_stations; i++) {
        printf("%s,", station_names[i]);
    }
    printf("\n");

    unsigned int** link_costs = NULL;
    link_costs = get_station_link_costs(num_stations);
    for (int i = 0; i < num_stations; i++) {
        for (int j = 0; j < num_stations; j++) {
            printf("%u ", link_costs[i][j]);
        }
        printf("\n");
    }

    float* station_popularity_list = NULL;
    station_popularity_list = get_station_popularity(num_stations);
    for (int i = 0; i < num_stations; i++) {
        printf("%f,", station_popularity_list[i]);
    }
    printf("\n");

    char*** stations_in_lines = NULL;
    unsigned int* num_stations_per_line = malloc(NUM_LINES * sizeof(unsigned int));
    stations_in_lines = get_stations_in_lines(NUM_LINES, num_stations_per_line);
    for (unsigned int i = 0; i < NUM_LINES; i++) {
        for (unsigned int j = 0; j < num_stations_per_line[i]; j++) {
            printf("%s,", stations_in_lines[i][j]);
        }
        printf("%u\n", num_stations_per_line[i]);
    }

    unsigned int num_ticks;
    scanf("%d", &num_ticks);
    printf("%d\n", num_ticks);

    unsigned int* num_trains_per_line = NULL;
    num_trains_per_line = get_num_trains_per_line(NUM_LINES);
    for (int i = 0; i < NUM_LINES; i++) {
        printf("%d,",num_trains_per_line[i]);
    }
    printf("\n");

    ///////////////////
    // Model Problem //
    ///////////////////


    LineNetwork** networks = malloc(sizeof(LineNetwork*) * NUM_LINES);
    for (unsigned int i = 0; i < NUM_LINES; i++) {
        networks[i] = get_line_network(stations_in_lines[i], station_names, num_stations, num_stations_per_line[i]);
    }
    for (int i = 0; i < NUM_LINES; i++) {
        for (int j = 0; j < networks[i]->num_nodes; j++) {
            printf("%u,", networks[i]->station_numbers[j]);
        }
        printf("\n");
    }


    Train** trains = malloc(sizeof(Train*) * NUM_LINES);
    for (unsigned int i = 0; i < NUM_LINES; i++) {
        trains[i] = malloc(sizeof(Train) * num_trains_per_line[i]);
        for (unsigned int j = 0; j < num_trains_per_line[i]; j++) {
            trains[i][j] = (Train){i, j, 0, STATION};
        }
    }
/*
    for (unsigned int t = 0; t < num_ticks; t++) {
        for (unsigned int i = 0; i < NUM_LINES; i++) {
            for (unsigned int j = 0; j < num_trains_per_line[i]; j++) {

                // Perform train operations here.
                trains[i][j].time_left -= 1;

                if (trains[i][j].time_left <= 0) {
                    // Perform transitions here.
                    unsigned int next_loc = get_next_node_index(networks[i], trains[i][j].line_idx);
                    unsigned int next_station_idx = get_station_number(networks[i], next_loc);


                    if (trains[i][j].loc == STATION) {
                        unsigned int curr_station_idx = get_station_number(networks[i], trains[i][j].loc);
                        trains[i][j].time_left = link_costs[curr_station_idx][next_station_idx];
                        trains[i][j].loc = LINK;
                    } else {
                        trains[i][j].line_idx = next_loc;
                        trains[i][j].time_left = rand() * station_popularity_list[next_station_idx];
                        trains[i][j].loc = STATION;
                    }
                }
            }
        }


        // I decide not to use multithreading for printing to console since it is a simple operation
        // Further multithreading might cause more overhead and printing to console is not very fast.
        printf(TIME_UPDATE_PREFIX, t);
        for (unsigned int i = 0; i < NUM_LINES; i++) {
            for (unsigned int j = 0; j < num_trains_per_line[i]; j++) {
                unsigned int current_station_idx = get_station_number(networks[i], trains[i][j].loc);
                if (trains[i][j].loc == STATION) {
                    printf(STATION_PRINT_TEMPLATE, LINE_PREFIXES[i], j, current_station_idx);
                } else {
                    unsigned int next_loc = get_next_node_index(networks[i], trains[i][j].loc);
                    unsigned int next_station_idx = get_station_number(networks[i], next_loc);
                    printf(LINK_PRINT_TEMPLATE, LINE_PREFIXES[i], j, current_station_idx, next_station_idx);
                }
                if (i != (NUM_LINES - 1) && j != (num_trains_per_line[i] - 1)) {
                    printf(", ");
                }
            }
        }
    }*/

    // Final Update here.



    ////////////////////////////////////
    // Clean up memory on termination //
    ////////////////////////////////////
    for (int i = 0; i < NUM_LINES; i++) {
        free(trains[i]);
    }
    free(trains);
    trains = NULL;

    for (unsigned int i = 0; i < NUM_LINES; i++) {
        delete_line_network(networks[i]);
    }
    free(networks);
    networks = NULL;

    for (unsigned int i = 0; i < NUM_LINES; i++) {
        for (unsigned int j = 0; j < num_stations_per_line[i]; j++) {
            free(stations_in_lines[i][j]);
        }
        free(stations_in_lines[i]);
    }
    free(stations_in_lines);
    free(num_stations_per_line);
    stations_in_lines = NULL;
    num_stations_per_line = NULL;

    for (int i = 0; i < num_stations; i++) {
        free(station_names[i]);
    }
    free(station_names);
    station_names = NULL;

    for (int i = 0; i < num_stations; i++) {
        free(link_costs[i]);
    }
    free(link_costs);
    link_costs = NULL;

    free(station_popularity_list);
    station_popularity_list = NULL;

    return EXIT_SUCCESS;
}

unsigned int* get_num_trains_per_line(unsigned int num_lines)
{
    unsigned int* num_trains_per_line = NULL;
    num_trains_per_line = malloc(sizeof(unsigned int) * num_lines);
    char ch;
    for (int i = 0; i < (num_lines - 1); i++) {
        scanf("%u%c", &num_trains_per_line[i], &ch);
    }
    scanf("%u", &num_trains_per_line[num_lines - 1]);

    return num_trains_per_line;
}

unsigned int count_char(char* string, char ch)
{
    unsigned int idx = 0;
    unsigned int count = 1;

    while (string[idx] != 0) {
        if (string[idx] == ch) {
            count++;
        }
        idx++;
    }

    return count;
}

char*** get_stations_in_lines(unsigned int num_lines, unsigned int* num_stations_per_line)
{
    char*** stations_in_lines = malloc(sizeof(char**) * num_lines);

    for (int i = 0; i < num_lines; i++) {
        char* raw_station_list_string = NULL;
        raw_station_list_string = malloc(sizeof(char) * STATION_NAMES_BUFFER_SIZE);
        scanf("%s", raw_station_list_string);

        unsigned int num_stations_in_line = count_char(raw_station_list_string, ',');
        stations_in_lines[i] = malloc(sizeof(char**) * num_stations_in_line);

        char* station_name = NULL;
        station_name = strtok(raw_station_list_string, ",");

        unsigned int j = 0;
        while (station_name != NULL) {
            stations_in_lines[i][j] = malloc(sizeof(char) * STATION_NAME_BUFFER_SIZE);
            strcpy(stations_in_lines[i][j], station_name);
            station_name = strtok (NULL, (const char*) ",");
            j++;
        }

/*        for (int j = 0; j < num_stations_in_line; j++) {
            printf("%s\n", stations_in_lines[i][j]);
        }*/
        num_stations_per_line[i] = num_stations_in_line;
        free(raw_station_list_string);
    }

    return stations_in_lines;
}

char** get_station_names(char* raw_input, unsigned int num_stations)
{
    char** station_names = malloc(sizeof(char*) * num_stations);
    char* station_name = NULL;
    station_name = strtok(raw_input, ",");
    int count = 0;
    for (unsigned int i = 0; i < num_stations; i++) {
        station_names[i] = malloc(sizeof(char) * STATION_NAME_BUFFER_SIZE);
        strcpy(station_names[i], station_name);
        station_name = strtok (NULL, (const char*) ",");
    }

    return station_names;
}

unsigned int** get_station_link_costs(unsigned int num_stations)
{
    unsigned int** link_costs = malloc(sizeof(unsigned int*) * num_stations);
    for (int i = 0; i < num_stations; i++) {
        link_costs[i] = malloc(sizeof(unsigned int) * num_stations);
        for (int j = 0; j < num_stations; j++) {
            scanf("%u", &link_costs[i][j]);
        }
    }

    return link_costs;
}

float* get_station_popularity(unsigned int num_stations)
{
    float* station_popularity = malloc(sizeof(float) * num_stations);
    for (unsigned int i = 0; i < (num_stations); i++) {
        char ch;
        scanf("%f%c", &station_popularity[i], &ch);
    }

    return station_popularity;
}