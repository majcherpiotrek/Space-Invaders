//
// Created by piotrek on 04.06.17.
//

#ifndef SPACE_INVADERS_SHIELD_H
#define SPACE_INVADERS_SHIELD_H

#include <ncurses.h>
#include "Game_actor.h"

class Shield : public Game_actor{
public:
    Shield(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};


#endif //SPACE_INVADERS_SHIELD_H
