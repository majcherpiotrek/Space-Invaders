//
// Created by piotrek on 04.06.17.
//

#ifndef SPACE_INVADERS_GAME_ACTOR_H
#define SPACE_INVADERS_GAME_ACTOR_H

#include "Direction.h"
class Game_actor {
protected:
    int pos_x;
    int pos_y;
    int max_x;
    int min_x;
    int max_y;
    int min_y;
    int width;
    int height;
    bool done;
public:
    Direction move_direction = RIGHT;

    virtual void drawActor() = 0;

    Game_actor(int _pos_x, int _pos_y, int _width, int _height, int _min_x, int _max_x, int _min_y, int _max_y);

    void move(int move_x, int move_y);

    int getWidth() { return width; }

    int getHeight() { return height; }

    int getPos_x() const;

    int getPos_y() const;

    int getMax_x() const;

    int getMin_x() const;

    int getMax_y() const;

    int getMin_y() const;

    void setDone() { done = true; }
    bool isDone() { return  done; }
};

#endif //SPACE_INVADERS_GAME_ACTOR_H

