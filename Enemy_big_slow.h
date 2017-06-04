//
// Created by piotrek on 04.06.17.
//

#ifndef SPACE_INVADERS_ENEMY_BIG_SLOW_H
#define SPACE_INVADERS_ENEMY_BIG_SLOW_H

#include <ncurses.h>
#include "Direction.h"
#include "Game_actor.h"

class Enemy_big_slow : public Game_actor {
public:
    Enemy_big_slow(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};


#endif //SPACE_INVADERS_ENEMY_BIG_SLOW_H
