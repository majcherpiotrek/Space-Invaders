//
// Created by piotrek on 04.06.17.
//

#include "Enemy_big_slow.h"

Enemy_big_slow::Enemy_big_slow(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y)
    : Game_actor(_pos_x, _pos_y, 9, 3, _min_x, _max_x, _min_y, _max_y){
    hit_points = 10;
}

/**
 * Big slow enemy's shape
 * $_______$
 * |_______|
 *    |||
 */
void Enemy_big_slow::drawActor() {
    //first pos_y
    mvprintw(pos_y, pos_x, "$");
    mvprintw(pos_y, pos_x+1, "_");
    mvprintw(pos_y, pos_x+2, "_");
    mvprintw(pos_y, pos_x+3, "_");
    mvprintw(pos_y, pos_x+4, "_");
    mvprintw(pos_y, pos_x+5, "_");
    mvprintw(pos_y, pos_x+6, "_");
    mvprintw(pos_y, pos_x+7, "_");
    mvprintw(pos_y, pos_x+8, "$");
    //second pos_y
    mvprintw(pos_y+1, pos_x, "|");
    mvprintw(pos_y+1, pos_x+1, "_");
    mvprintw(pos_y+1, pos_x+2, "_");
    mvprintw(pos_y+1, pos_x+3, "_");
    mvprintw(pos_y+1, pos_x+4, "_");
    mvprintw(pos_y+1, pos_x+5, "_");
    mvprintw(pos_y+1, pos_x+6, "_");
    mvprintw(pos_y+1, pos_x+7, "_");
    mvprintw(pos_y+1, pos_x+8, "|");
    // third pos_y
    mvprintw(pos_y+2, pos_x+3, "|");
    mvprintw(pos_y+2, pos_x+4, "|");
    mvprintw(pos_y+2, pos_x+5, "|");
}
