//
// Created by piotrek on 03.06.17.
//

#include "SmallBullet.h"

SmallBullet::SmallBullet(short _pos_x, short _pos_y, int _min_x, int _max_x, int _min_y, int _max_y)
    : Game_actor(_pos_x, _pos_y, 1, 1, _min_x, _max_x, _min_y, _max_y){
}

void SmallBullet::drawActor() {
    mvprintw(pos_y, pos_x, "*");
}
