//
// Created by piotrek on 04.06.17.
//

#include "Enemy_small_fast.h"

Enemy_small_fast::Enemy_small_fast(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y) :
    Game_actor(_pos_x, _pos_y, 5, 1, _min_x, _max_x, _min_y, _max_y ){
}

/**
 * Small fast enemy's shape:
 *
 * $=|=$
 */
void Enemy_small_fast::drawActor() {
    mvprintw(pos_y, pos_x, "$");
    mvprintw(pos_y, pos_x+1, "=");
    mvprintw(pos_y, pos_x+2, "|");
    mvprintw(pos_y, pos_x+3, "=");
    mvprintw(pos_y, pos_x+4, "$");
}
