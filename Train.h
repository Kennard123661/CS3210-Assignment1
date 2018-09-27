//
// Created by kennard on 22/09/18.
//

#ifndef CS3210_ASSIGNMENT1_TRAIN_H
#define CS3210_ASSIGNMENT1_TRAIN_H

#endif //CS3210_ASSIGNMENT1_TRAIN_H

enum TrainStatus {WAIT_TO_LOAD, LOADING, LOADED, LINK};

typedef struct {
    unsigned int line_id;
    unsigned int train_id;
    unsigned int node_idx;
    float time_left;
    enum TrainStatus train_status;
    unsigned char has_acted;
} Train;