//
// Created by kennard on 22/09/18.
//

#ifndef CS3210_ASSIGNMENT1_TRAIN_H
#define CS3210_ASSIGNMENT1_TRAIN_H

#endif //CS3210_ASSIGNMENT1_TRAIN_H

enum train_loc {STATION, LINK, OPENING, OPENED};

typedef struct {
    unsigned int network_idx;
    unsigned int train_idx;
    unsigned int line_idx;
    float time_left;
    enum train_loc loc;
    unsigned char hasActed;
} Train;