//
// Created by kennard on 22/09/18.
//

#ifndef CS3210_ASSIGNMENT1_TRAIN_H
#define CS3210_ASSIGNMENT1_TRAIN_H

#endif //CS3210_ASSIGNMENT1_TRAIN_H

enum train_loc {STATION, LINK, LOAD_STATION};

typedef struct {
    unsigned int line_idx;
    unsigned int time_left;
    enum train_loc loc;
    unsigned char hasActed;
} Train;