//
// Created by piotrek on 03.06.17.
//

#ifndef SPACE_INVADERS_BULLET_H
#define SPACE_INVADERS_BULLET_H


class Bullet {
private:
    short row;
    short column;
    const char* sign;
    int color;
public:
    Bullet(short _row, short _column, const char* _sign, int _color);
    void move(short move_x, short move_y);
    int getRow() { return row; }
    int getColumn() { return column; }
    const char* getSign() { return sign; }
    int getColor() { return color; }
};



#endif //SPACE_INVADERS_BULLET_H
