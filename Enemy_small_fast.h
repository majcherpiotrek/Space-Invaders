//
// Created by piotrek on 04.06.17.
//

#ifndef SPACE_INVADERS_ENEMY_SMALL_FAST_H
#define SPACE_INVADERS_ENEMY_SMALL_FAST_H

#include <ncurses.h>
#include "Game_actor.h"

class Enemy_small_fast : public Game_actor {
public:
    Enemy_small_fast(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};


#endif //SPACE_INVADERS_ENEMY_SMALL_FAST_H
