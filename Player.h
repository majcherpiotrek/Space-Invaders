//
// Created by piotrek on 03.06.17.
//

#ifndef SPACE_INVADERS_PLAYER_H
#define SPACE_INVADERS_PLAYER_H

#include <ncurses.h>

class Player {
private:
    int pos_x;
    int max_x;
    int min_x;
    const int width = 6;
    int row;
public:
    Player(int _row, int _column, int _min_x, int _max_x);
    void drawPlayer();
    void move(int move_x);
    int getWidth() { return width; }
    int getRow() { return row; }
    int getPosX() { return pos_x; }
};


#endif //SPACE_INVADERS_PLAYER_H
