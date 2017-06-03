//
// Created by piotrek on 03.06.17.
//

#include "Bullet.h"

Bullet::Bullet(short _row, short _column, const char* _sign, int _color) {
    row = _row;
    column = _column;
    sign = _sign;
    color = _color;
    done = false;
}

void Bullet::move(short move_x, short move_y)  {
    row += move_y;
    column += move_x;
}