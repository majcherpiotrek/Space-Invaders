//
// Created by piotrek on 04.06.17.
//

#include "BigBullet.h"

BigBullet::BigBullet(short _pos_x, short _pos_y, int _min_x, int _max_x, int _min_y, int _max_y)
        : Game_actor(_pos_x, _pos_y, 3, 3, _min_x, _max_x, _min_y, _max_y){

}

void BigBullet::drawActor() {
    mvprintw(pos_y, pos_x+1, "#");
    mvprintw(pos_y+1, pos_x, "#");
    mvprintw(pos_y+1, pos_x+1, "#");
    mvprintw(pos_y+1, pos_x+2, "#");
    mvprintw(pos_y+2, pos_x+1, "#");
}
