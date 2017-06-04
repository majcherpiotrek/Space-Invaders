//
// Created by piotrek on 04.06.17.
//
#include "Game_actor.h"

/**
 * Constructs the abstarct game actor
 * @param _pos_x the x coordinate on the screen
 * @param _pos_y the y coordinate on the screen
 * @param _width the width of the actor's shape on the screen (number of columns it occupies)
 * @param _height the height of the actor's shape on the screen (number of rows it occupies)
 * @param _min_x the minimum x coordinate boundary where the actor can move
 * @param _max_x the maximum x coordinate boundary where the actor can move
 * @param _min_y the minimum  pos_y = _row;
    pos_x = _column;
    min_x = _min_x;
    max_x = _max_x; y coordinate boundary where the actor can move
 * @param _max_y the maximum y coordinate boundary where the actor can move
 */
Game_actor::Game_actor(int _pos_x, int _pos_y, int _width, int _height, int _min_x, int _max_x, int _min_y, int _max_y) {
    pos_x = _pos_x;
    pos_y = _pos_y;
    width = _width;
    height = _height;
    min_x = _min_x;
    max_x = _max_x;
    min_y = _min_y;
    max_y = _max_y;
}

/**
 * Changes the actor coordinates on the screen
 * @param move_x move vector x value, <0 left, >0 right
 * @param move_y move vector y value, <0 top, >0 bottom
 */
void Game_actor::move(int move_x, int move_y) {

    if (!done) {
        if (move_x < 0) {
            /// Check if we don't move out of the area on the left
            if (pos_x + move_x >= min_x) {
                pos_x += move_x;
            }
        } else {
            /// Check if we don't move out of the area on the right
            if (pos_x + width + move_x <= max_x) {
                pos_x += move_x;
            }
        }
        if (move_y < 0) {
            /// Check if we don't move out of the area on the top
            if (pos_y + move_y < min_y) {
                done = true;
            }
            pos_y += move_y;
        } else {
            /// Check if we don't move out of the area on the bottom
            if (pos_y + height + move_y > max_y) {
                done = true;
            }
            pos_y += move_y;
        }
    }
}

int Game_actor::getMax_x() const {
    return max_x;
}

int Game_actor::getMin_x() const {
    return min_x;
}

int Game_actor::getMax_y() const {
    return max_y;
}

int Game_actor::getMin_y() const {
    return min_y;
}

int Game_actor::getPos_x() const {
    return pos_x;
}

int Game_actor::getPos_y() const {
    return pos_y;
}


