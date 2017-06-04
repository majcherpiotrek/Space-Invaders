//
// Created by piotrek on 04.06.17.
//

#include "Shield.h"
Shield::Shield(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y)
        : Game_actor(_pos_x, _pos_y, 20, 3, _min_x, _max_x, _min_y, _max_y){
    hit_points = 200;
}
/** Shield's shape
 * ####################
 * ####################
 * ####################
 */
void Shield::drawActor() {
    if(!done) {
        for (int i = 0; i < 3; i++) {
            if (i == 0 && hit_points <= 100) continue;
            if (i == 1 && hit_points <= 50) continue;
            for (int j=0; j < 20; j++) {
                mvprintw(pos_y+i, pos_x+j, "#");
            }
        }
    }
}
