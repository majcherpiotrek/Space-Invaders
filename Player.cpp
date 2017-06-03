//
// Created by piotrek on 03.06.17.
//

#include "Player.h"

Player::Player(int _row, int _column, int _min_x, int _max_x) {
    row = _row;
    pos_x = _column;
    min_x = _min_x;
    max_x = _max_x;
}

/**
 * Draw players shape
 *
 * Player's shape
 *
 *  |_/$\_|
 *
 */
void Player::drawPlayer() {
    mvprintw(row, pos_x,"|");
    mvprintw(row, pos_x+1, "_");
    mvprintw(row, pos_x+2, "/");
    mvprintw(row, pos_x+3, "$");
    mvprintw(row, pos_x+4, "\\");
    mvprintw(row, pos_x+5, "_");
    mvprintw(row, pos_x+6, "|");
}

/**
 * Move player left or right
 * @param move_x move vector x value, <0 left, >0 right
 */
void Player::move(int move_x) {
    if (move_x < 0) {
        /// Check if we don't move out of the area on the left
        if (pos_x + move_x >= min_x) {
            pos_x += move_x;
        }
    } else {
        /// Check if we don't move out of the area on the right
        if (pos_x + width  + move_x <= max_x) {
            pos_x += move_x;
        }
    }
}
