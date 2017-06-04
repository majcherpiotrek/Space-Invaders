//
// Created by piotrek on 03.06.17.
//

#ifndef SPACE_INVADERS_BULLET_H
#define SPACE_INVADERS_BULLET_H

#include <ncurses.h>
#include "Game_actor.h"

class SmallBullet : public Game_actor{

public:
    SmallBullet(short _pos_x, short _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};



#endif //SPACE_INVADERS_BULLET_H
