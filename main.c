#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "LineNetwork.h"
#include "Train.h"
#include "StationWait.h"

#define STATION_NAMES_BUFFER_SIZE 8196
#define STATION_NAME_BUFFER_SIZE 256

char** get_station_names(char* input, unsigned int num_stations);
unsigned int** get_station_link_costs(unsigned int num_stations);
float* get_station_popularity(unsigned int num_stations);
char*** get_stations_in_lines(unsigned int num_lines, unsigned int* num_stations_per_line);
unsigned int* get_num_trains_per_line(unsigned int num_lines);

void read_inputs(const unsigned int NUM_LINES, unsigned int *num_stations, char ***station_names,
        unsigned int ***link_costs, float **station_popularity_list, char ****stations_in_lines,
        unsigned int **num_stations_per_line, unsigned int *num_ticks, unsigned int **num_trains_per_line);

int main() {
    const unsigned int TRAIN_SPEED = 1;
    const unsigned int NUM_LINES = 3;
    const char LINE_PREFIXES[] = {'g','y','b'};
    const char* STATION_PRINT_TEMPLATE = "%c%u-s%u";
    const char* LINK_PRINT_TEMPLATE = "%c%u-s%u->s%u";
    const char* TIME_UPDATE_PREFIX = "%u: ";

    int STATION_WAITING_RANGE = 10;
    int STATION_WAITING_MIN = 1;

    srand((unsigned) time(NULL));

    unsigned int num_stations;
    char **station_names;
    unsigned int **link_costs;
    float *station_popularity;
    char ***stations_in_lines;
    unsigned int *num_stations_per_line;
    unsigned int num_ticks;
    unsigned int *num_trains_per_line;

    read_inputs(NUM_LINES, &num_stations, &station_names, &link_costs, &station_popularity, &stations_in_lines,
                &num_stations_per_line, &num_ticks, &num_trains_per_line);

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

    // Each thread represents a train.
    unsigned int total_num_trains = 0;
    for (unsigned int i = 0; i < NUM_LINES; i++) {
        total_num_trains += num_trains_per_line[i];
    }

    Train* trains = malloc(sizeof(Train) * total_num_trains);
    unsigned int count = 0;
    for (unsigned int i = 0; i < NUM_LINES; i++) {
        for (unsigned int j = 0; j < num_trains_per_line[i]; j++) {
            trains[count++] = (Train){i, j, (j % 2 == 0) ? 0 : (num_stations_per_line[i] - 1), 1, STATION, 0};
        }
    }


    char** have_trains_acted;
    have_trains_acted = malloc(sizeof(char*) * NUM_LINES);
    for (unsigned int i = 0; i < NUM_LINES; i++) {
        have_trains_acted[i] = malloc(sizeof(char) * num_trains_per_line[i]);
    }

    // Create mutexes for locking the link between each station.
    omp_lock_t** link_mutexes = malloc(sizeof(omp_lock_t*) * num_stations);
    for (unsigned int i = 0; i < num_stations; i++) {
        link_mutexes[i] = malloc(sizeof(omp_lock_t) * num_stations);
        for (unsigned int j = 0; j < num_stations; j++) {
            omp_init_lock(&link_mutexes[i][j]);
        }
    }

    // Create mutexes for locking the loading of each station.
    omp_lock_t* station_loading_lock = malloc(sizeof(omp_lock_t) * num_stations * 2);
    for (unsigned int i = 0; i < (num_stations * 2); i++) {
        omp_init_lock(&station_loading_lock[i]);
    }

    // Denotes pointers owned by trains.
    omp_lock_t** train_lock_ptrs = malloc(sizeof(omp_lock_t*) * total_num_trains);
    for (unsigned int i = 0; i < total_num_trains; i++) {
        train_lock_ptrs[i] = NULL;
    }

    // Model Problem for Part 2
    StationWait* station_waits = malloc(sizeof(StationWait) * num_stations * 2);
    for (unsigned int i = 0; i < num_stations; i++) {
        station_waits[i] = (StationWait){0.0, (unsigned int) 0xFFFFFFFF, 0, 0, 0};
    }

    omp_set_num_threads(total_num_trains);
    // omp_set_dynamic(0);


    for (unsigned int t = 0; t < num_ticks; t++) {

        #pragma omp parallel for num_threads(total_num_trains)
        for (unsigned int i = 0; i < total_num_trains; i++) {
            // printf("Openmp inner threads %d\n", omp_get_thread_num());

            // Step 1: Update trains that previously acquired locks but are have their time limit exceeded
            if ((trains[i].loc == OPENING) || (trains[i].loc == LINK)) {
                // Release lock and transition
                if (trains[i].time_left <= 0) {
                    omp_lock_t *lock_ptr = train_lock_ptrs[i];
                    train_lock_ptrs[i] = NULL;
                    omp_unset_lock(lock_ptr);
                    unsigned int next_node = get_next_node_index(networks[trains[i].network_idx], trains[i].line_idx);
                    unsigned int curr_station_idx = get_station_idx(networks[trains[i].network_idx], trains[i].line_idx);
                    unsigned int next_station_idx = get_station_idx(networks[trains[i].network_idx], next_node);


                    if ((trains[i].loc == OPENING) && (curr_station_idx != next_station_idx)) {
                        // Finish serving commuters at the station.
                        trains[i].loc = OPENED;
                    } else {
                        // Finish the line so proceed to the next link in the network.
                        trains[i].loc = STATION;
                        trains[i].line_idx = next_node;
                    }
                }
            }
        }

        #pragma omp parallel for num_threads(total_num_trains)
        for (unsigned int i = 0; i < total_num_trains; i++) {
            // Step 2: Trains holding lock release first if possible.
            if ((trains[i].loc == OPENING) || (trains[i].loc == LINK)) {
                trains[i].time_left -= 1;

                // Release lock and transition
                if (trains[i].time_left <= 0) {
                    omp_lock_t *lock_ptr = train_lock_ptrs[i];
                    train_lock_ptrs[i] = NULL;
                    omp_unset_lock(lock_ptr);

                    unsigned int next_node = get_next_node_index(networks[trains[i].network_idx], trains[i].line_idx);
                    unsigned int curr_station_idx = get_station_idx(networks[trains[i].network_idx], trains[i].line_idx);
                    unsigned int next_station_idx = get_station_idx(networks[trains[i].network_idx], next_node);

                    if ((trains[i].loc == OPENING) && (curr_station_idx != next_station_idx)) {
                        // Finish serving commuters at the station.
                        trains[i].loc = OPENED;
                    } else {
                        // Finish the line so proceed to the next link in the network.
                        trains[i].loc = STATION;
                        trains[i].line_idx = next_node;
                    }
                }

                trains[i].hasActed = 1;
            }
        }

        #pragma omp parallel for num_threads(total_num_trains)
        for (unsigned int i = 0; i < total_num_trains; i++) {
            // Step 3: Other trains acquire lock if possible and make move.

            if (!trains[i].hasActed && (trains[i].train_idx < ((t+1) * 2))) {
                if (trains[i].loc == STATION) {
                    // Try to occupy the station's lock
                    unsigned int station_idx = get_station_idx(networks[trains[i].network_idx], trains[i].line_idx);
                    unsigned int station_num = station_idx;
                    station_idx += is_reverse_direction(&networks[trains[i].network_idx], station_idx) ? num_stations : 0;

                    omp_lock_t* lock_ptr = &station_loading_lock[station_idx];
                    if (omp_test_lock(lock_ptr)) {
                        // Successfully acquired lock, begin loading
                        trains[i].loc = OPENING;
                        trains[i].time_left = station_popularity[station_num] *
                                ((float) ((rand() % STATION_WAITING_RANGE) + STATION_WAITING_MIN)) - 1;
                        train_lock_ptrs[i] = lock_ptr;
                    }

                } else if (trains[i].loc == OPENED) {
                    unsigned int curr_station_idx = get_station_idx(networks[trains[i].network_idx], trains[i].line_idx);
                    unsigned int next_loc = get_next_node_index(networks[trains[i].network_idx], trains[i].line_idx);
                    unsigned int next_station_idx = get_station_idx(networks[trains[i].network_idx], next_loc);

                    omp_lock_t* link_loc_ptr = &link_mutexes[curr_station_idx][next_station_idx];
                    if (omp_test_lock(link_loc_ptr)) {
                        // Suceessfully acquired lock.
                        trains[i].loc = LINK;
                        train_lock_ptrs[i] = link_loc_ptr;
                        trains[i].time_left = link_costs[curr_station_idx][next_station_idx] - 1;
                    }
                }

                trains[i].hasActed = 1;
            }

            trains[i].hasActed = 0; // Reset hasActed for the next iteration
        }

        // I decide not to use multithreading for printing to console since it is a simple operation
        // Further multithreading might cause more overhead and printing to console is not very fast.
        printf(TIME_UPDATE_PREFIX, t);
        for (unsigned int i = 0; i < total_num_trains; i++) {
            if (trains[i].train_idx >= (t+1) * 2) {
                continue;
            }

            unsigned int current_station_idx = get_station_idx(networks[trains[i].network_idx], trains[i].line_idx);
            if ((trains[i].loc == STATION) || (trains[i].loc == OPENING) || (trains[i].loc == OPENED)) {
                printf(STATION_PRINT_TEMPLATE, LINE_PREFIXES[trains[i].network_idx], trains[i].train_idx, current_station_idx);
            } else {
                unsigned int next_loc = get_next_node_index(networks[trains[i].network_idx], trains[i].line_idx);
                unsigned int next_station_idx = get_station_idx(networks[trains[i].network_idx], next_loc);
                printf(LINK_PRINT_TEMPLATE, LINE_PREFIXES[trains[i].network_idx], trains[i].train_idx, current_station_idx, next_station_idx);
            }
            if (i != (total_num_trains - 1)) {
                printf(", ");
            }
        }
        printf("\n");
    }


    // Final Update here.



    ////////////////////////////////////
    // Clean up memory on termination //
    ////////////////////////////////////
    free(train_lock_ptrs);
    train_lock_ptrs = NULL;

    free(station_loading_lock);
    station_loading_lock = NULL;

    for (unsigned int i = 0; i < num_stations; i++) {
        free(link_mutexes[i]);
    }
    free(link_mutexes);
    link_mutexes = NULL;

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

    free(station_popularity);
    station_popularity = NULL;

    return EXIT_SUCCESS;
}

void read_inputs(const unsigned int NUM_LINES, unsigned int *num_stations, char ***station_names,
        unsigned int ***link_costs, float **station_popularity_list, char ****stations_in_lines,
        unsigned int **num_stations_per_line, unsigned int *num_ticks, unsigned int **num_trains_per_line) {

    (*station_names) = NULL;
    (*link_costs) = NULL;
    (*station_popularity_list) = NULL;
    (*stations_in_lines) = NULL;
    (*num_stations_per_line) = malloc(NUM_LINES * sizeof(unsigned int));
    (*num_trains_per_line) = NULL;
    scanf("%u\n", num_stations);
    printf("%u\n", (*num_stations));

    char raw_stations_string[STATION_NAMES_BUFFER_SIZE];
    scanf("%s", raw_stations_string);
    (*station_names) = get_station_names(raw_stations_string, (*num_stations));
    for (int i = 0; i < (*num_stations); i++) {
        printf("%s,", (*station_names)[i]);
    }
    printf("\n");

    (*link_costs) = get_station_link_costs((*num_stations));
    for (int i = 0; i < (*num_stations); i++) {
        for (int j = 0; j < (*num_stations); j++) {
            printf("%u ", (*link_costs)[i][j]);
        }
        printf("\n");
    }

    (*station_popularity_list) = get_station_popularity((*num_stations));
    for (int i = 0; i < (*num_stations); i++) {
        printf("%f,", (*station_popularity_list)[i]);
    }
    printf("\n");

    (*stations_in_lines) = get_stations_in_lines(NUM_LINES, (*num_stations_per_line));
    for (unsigned int i = 0; i < NUM_LINES; i++) {
        for (unsigned int j = 0; j < (*num_stations_per_line)[i]; j++) {
            printf("%s,", (*stations_in_lines)[i][j]);
        }
        printf("%u\n", (*num_stations_per_line)[i]);
    }

    scanf("%d", num_ticks);
    printf("%d\n", (*num_ticks));
    (*num_trains_per_line) = get_num_trains_per_line(NUM_LINES);
    for (int i = 0; i < NUM_LINES; i++) {
        printf("%d,", (*num_trains_per_line)[i]);
    }
    printf("\n");
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