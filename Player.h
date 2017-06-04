//
// Created by piotrek on 03.06.17.
//

#ifndef SPACE_INVADERS_PLAYER_H
#define SPACE_INVADERS_PLAYER_H

#include <ncurses.h>
#include "Game_actor.h"

class Player : public Game_actor{
public:
    Player(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};


#endif //SPACE_INVADERS_PLAYER_H
