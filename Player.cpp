//
// Created by piotrek on 03.06.17.
//

#include "Player.h"

Player::Player(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y)
    : Game_actor(_pos_x, _pos_y, 7, 1, _min_x, _max_x, _min_y, _max_y){
}

/**
 * Draw players shape
 *
 * Player's shape
 *
 *  |_/$\_|
 *
 */
void Player::drawActor() {
    mvprintw(pos_y, pos_x,"|");
    mvprintw(pos_y, pos_x+1, "_");
    mvprintw(pos_y, pos_x+2, "/");
    mvprintw(pos_y, pos_x+3, "$");
    mvprintw(pos_y, pos_x+4, "\\");
    mvprintw(pos_y, pos_x+5, "_");
    mvprintw(pos_y, pos_x+6, "|");
}

