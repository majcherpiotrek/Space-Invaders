#include <iostream>
#include <chrono>
#include <thread>
#include <ncurses.h>
#include <mutex>
#include <vector>
#include <atomic>
#include <random>
#include "SmallBullet.h"
#include "Direction.h"
#include "Player.h"
#include "Enemy_big_slow.h"
#include "BigBullet.h"
#include "Enemy_small_fast.h"
#include "Shield.h"

static const std::chrono::milliseconds frame_durtion(40); // 40 FPS
static const std::chrono::milliseconds t_between_big_enemies(12000); // new big enemy every 8 seconds
static const std::chrono::milliseconds t_between_small_enemies(4000); // new small enemy every 4 seconds
static const int t_big_enemies_bullets= 4000;
static const int t_small_enemies_bullets= 500;
static const int small_bullets_speed = 30; // rows per second
static const int big_bullets_speed = 15; //rows per sec_ond
static const int big_slow_enemy_speed = 10; // columns per second
static const int small_fast_enemy_speed = 20; // columns per second
static const int SPACE = 32;
static std::atomic_bool exit_condition(false);
static std::atomic_bool game_over(false);
static std::default_random_engine generator;
static std::uniform_int_distribution<int> distribution(1,100);
static auto dice = std::bind ( distribution, generator );
static int POINTS = 0;
static int BIG_SHIPS_DESTROYED = 0;
static int SMALL_SHIPS_DESTROYED = 0;

/// Bullets' vector
static std::vector<BigBullet*> big_bullets_vector;
static std::vector<SmallBullet*> small_bullets_vector;
static std::vector<SmallBullet*> player_bullets_vector;

/// Enemies's vector
static std::vector<Enemy_big_slow*> big_slow_enemies_vector;
static std::vector<Enemy_small_fast*> small_fast_enemies_vector;

/// Shield
static Shield* shield;

/// Mutexes
static std::mutex player_bullets_mutex;
static std::mutex small_bullets_mutex;
static std::mutex big_bullets_mutex;
static std::mutex big_enemies_mutex;
static std::mutex small_enemies_mutex;
static std::mutex player_mutex;
static std::mutex ncurses_mutex;

/// Colors' modes
static const short MODE_GREEN = 1;
static const short MODE_RED = 2;

bool isHit(Game_actor* bullet, Game_actor* actor);
void handle_bullet_hits(Player &player);
void remove_destroyed_enemies();
void remove_used_bullets();
void draw_bullets();
void shoot_small_bullets();
void player_shoots(Game_actor &player);
void draw_enemies();
void draw_health(Player &player);
/// Big enemies functions
void move_big_slow_enemies();
void create_big_slow_enemies_bullets();
void shoot_big_bullets();
void big_slow_enemy_shoots(Enemy_big_slow &enemy);
void create_big_enemy();

/// Small enemies functions
void move_small_fast_enemies();
void create_small_fast_enemies_bullets();
void small_fast_enemy_shoots(Enemy_small_fast &enemy);
void create_small_enemy();


/// Main view rendering function and game loop
/**
 * A method to be executed in a separate thread. Used to refresh the view.
 * @param exit exit condition
 * @param player a reference to player object
 */
void refresh_view(Player &player) {

    int row = getmaxy( stdscr )/2 - 2;
    int col = getmaxx( stdscr) / 2 - 8;

    /// Launch big enemies creation thread
    std::thread big_enemies_creation_thread(create_big_enemy);

    /// Launch small enemies creation thread
    std::thread small_enemies_creation_thread(create_small_enemy);

    /// Launch big enemies movement thread
    std::thread move_big_slow_enemies_thread( move_big_slow_enemies );

    /// Launch small enemies movement thread
    std::thread move_small_fast_enemies_thread( move_small_fast_enemies );

    /// Launch small fast enemies shooting thread
    std::thread small_fast_enemies_shooting_thread( create_small_fast_enemies_bullets );

    /// Launch small bullets shooting thread
    std::thread small_bullets_thread(shoot_small_bullets);

    /// Launch big slow enemies shooting thread
    std::thread big_slow_enemies_shooting_thread( create_big_slow_enemies_bullets );

    /// Launch big bullets shooting thread
    std::thread big_bullets_thread( shoot_big_bullets );

    while (!exit_condition) {
        clear();
        attron( A_BOLD );
        shield->drawActor();
        player_mutex.lock();
        ncurses_mutex.lock();
        player.drawActor();
        ncurses_mutex.unlock();
        player_mutex.unlock();
        draw_enemies();
        attroff( A_BOLD );

        remove_used_bullets();
        handle_bullet_hits(player);
        if (player.isDone()) {
            game_over = true;
        }
        remove_destroyed_enemies();

        draw_health(player);
        mvprintw(1,0, "Bombers destroyed: %d", BIG_SHIPS_DESTROYED);
        mvprintw(2,0, "Small fighters destroyed: %d", SMALL_SHIPS_DESTROYED);
        mvprintw(3,0, "TOTAL SCORE: %d", POINTS);
        draw_bullets();

        refresh();

        if (game_over) {
            clear();
            exit_condition = true;
            attron( A_BOLD );
            attron( COLOR_PAIR(MODE_RED));
            mvprintw( row, col, "GAME OVER!");
            mvprintw(row + 1, col, "Bombers destroyed: %d", BIG_SHIPS_DESTROYED);
            mvprintw(row + 2, col, "Small fighters destroyed: %d", SMALL_SHIPS_DESTROYED);
            mvprintw(row + 3, col, "TOTAL SCORE: %d", POINTS);
            attroff( COLOR_PAIR(MODE_RED));
            attroff( A_BOLD );
            refresh();
            break;
        } else {
            std::this_thread::sleep_for(frame_durtion);
        }
    }
    refresh();
    game_over = true;
    mvprintw(row + 4, col, "Finishing threads...");
    refresh();
    big_enemies_creation_thread.join();
    mvprintw(row + 5, col, "- big enemies creation thread: FINISHED");
    small_enemies_creation_thread.join();
    mvprintw(row + 6, col, "- small enemies creation thread: FINISHED");
    move_big_slow_enemies_thread.join();
    mvprintw(row + 7, col, "- big enemies motion thread: FINISHED");
    move_small_fast_enemies_thread.join();
    mvprintw(row + 8, col, "- small enemies motion thread: FINISHED");
    big_slow_enemies_shooting_thread.join();
    mvprintw(row + 9, col, "- big enemies shooting thread: FINISHED");
    small_fast_enemies_shooting_thread.join();
    mvprintw(row + 10, col, "- small enemies shooting thread: FINISHED");
    big_bullets_thread.join();
    mvprintw(row + 11, col, "- big bullets motion thread: FINISHED");
    small_bullets_thread.join();
    mvprintw(row + 12, col, "- small bullets motion thread: FINISHED");
    mvprintw(row + 13, col, "Finished all tasks!");
    mvprintw(row + 14, col, "Press 'q' to quit...");
    refresh();
}
//////////////////////////////////////////////

bool isHit(Game_actor* bullet, Game_actor* actor) {
    int bullet_x = bullet->getPos_x();
    int bullet_y = bullet->getPos_y();
    int bullet_w = bullet->getWidth();
    int bullet_h = bullet->getHeight();
    int actor_x_min = actor->getPos_x();
    int actor_x_max = actor_x_min + actor->getWidth();
    int actor_y_min = actor->getPos_y();
    int actor_y_max = actor_y_min + actor->getHeight();

    actor_x_min -= bullet_w;
    actor_y_min -= bullet_h;

    return bullet_x > actor_x_min
           && bullet_x < actor_x_max
            && bullet_y > actor_y_min
            && bullet_y < actor_y_max;
}

void handle_bullet_hits(Player &player) {
    small_bullets_mutex.lock();
    for (SmallBullet* bullet : small_bullets_vector) {
        if (!shield->isDone() && isHit(bullet, shield)) {
            bullet->setDone();
            shield->setDamage(1);
            continue;
        }
        if (isHit(bullet, &player)) {
            bullet->setDone();
            player.setDamage(1);
        }
    }
    small_bullets_mutex.unlock();

    big_bullets_mutex.lock();
    for (BigBullet* bullet : big_bullets_vector) {
        if (!shield->isDone() && isHit(bullet, shield)) {
            bullet->setDone();
            shield->setDamage(5);
            continue;
        }
        if (isHit(bullet, &player)) {
            bullet->setDone();
            player.setDamage(5);
        }
    }
    big_bullets_mutex.unlock();

    player_bullets_mutex.lock();
    for (SmallBullet* bullet : player_bullets_vector) {
        if (!shield->isDone() && isHit(bullet, shield)) {
            bullet->setDone();
            shield->setDamage(1);
            continue;
        }
        big_enemies_mutex.lock();
        for (Enemy_big_slow* enemy : big_slow_enemies_vector) {
            if (isHit(bullet, enemy)) {
                bullet->setDone();
                enemy->setDamage(1);
                if (enemy->isDone()){
                    BIG_SHIPS_DESTROYED++;
                }
                POINTS++;
            }
        }
        big_enemies_mutex.unlock();
        small_enemies_mutex.lock();
        for (Enemy_small_fast* enemy : small_fast_enemies_vector) {
            if (isHit(bullet, enemy)) {
                bullet->setDone();
                enemy->setDamage(1);
                SMALL_SHIPS_DESTROYED++;
                POINTS++;
            }
        }
        small_enemies_mutex.unlock();
    }
    player_bullets_mutex.unlock();
}
void remove_destroyed_enemies() {
    big_enemies_mutex.lock();
    if (big_slow_enemies_vector.size() > 0) {
        std::vector<Enemy_big_slow*>::iterator it = big_slow_enemies_vector.begin();
        int j = 0;
        while (it != big_slow_enemies_vector.end()) {
            if (big_slow_enemies_vector[j]->isDone()) {
                it = big_slow_enemies_vector.erase(it);
            } else {
                j++; it++;
            }
        }
    }
    big_enemies_mutex.unlock();

    small_enemies_mutex.lock();
    if (small_fast_enemies_vector.size() > 0) {
        std::vector<Enemy_small_fast*>::iterator it = small_fast_enemies_vector.begin();
        int j = 0;
        while (it != small_fast_enemies_vector.end()) {
            if (small_fast_enemies_vector[j]->isDone()) {
                it = small_fast_enemies_vector.erase(it);
            } else {
                j++; it++;
            }
        }
    }
    small_enemies_mutex.unlock();
}
/**
 * Removes the bullets, which have reached their destination,
 * from the player_bullets.
 * Joins the threads connected with those bullets and removes
 * them from the player_bullet_threads_vector as well.
 *
 * Contains bullets_vector_mutex critical section
 */
void remove_used_bullets() {
    small_bullets_mutex.lock(); // Critical section - erasing data from the small bullets vectors
    if (small_bullets_vector.size() > 0) {
        std::vector<SmallBullet*>::iterator it = small_bullets_vector.begin();
        int j = 0;
        while (it != small_bullets_vector.end()) {
            if (small_bullets_vector[j]->isDone()) {
                it = small_bullets_vector.erase(it);
            } else {
                j++; it++;
            }
        }
    }
    small_bullets_mutex.unlock(); // End of critical section

    big_bullets_mutex.lock(); // Critical section - erasing data from the small bullets vectors
    if (big_bullets_vector.size() > 0) {
        std::vector<BigBullet*>::iterator it = big_bullets_vector.begin();
        int j = 0;
        while (it != big_bullets_vector.end()) {
            if (big_bullets_vector[j]->isDone()) {
                it = big_bullets_vector.erase(it);
            } else {
                j++; it++;
            }
        }
    }
    big_bullets_mutex.unlock(); // End of critical section

    player_bullets_mutex.lock(); // Critical section - erasing data from the player bullets vectors
    if (player_bullets_vector.size() > 0) {
        std::vector<SmallBullet*>::iterator it = player_bullets_vector.begin();
        int j = 0;
        while (it != player_bullets_vector.end()) {
            if (player_bullets_vector[j]->isDone()) {
                it = player_bullets_vector.erase(it);
            } else {
                j++; it++;
            }
        }
    }
    player_bullets_mutex.unlock(); // End of critical section
}
/**
 * Prints the bullets shot by the player
 */
void draw_bullets() {
    for (SmallBullet* bullet : small_bullets_vector) {
        if (!bullet->isDone()) {
            attron( A_BOLD );
            if ( has_colors() ) {
                attron( COLOR_PAIR(MODE_RED));
            }
            ncurses_mutex.lock();
            bullet->drawActor();
            ncurses_mutex.unlock();
            if ( has_colors() ) {
                attroff( COLOR_PAIR(MODE_RED));
            }
            attroff( A_BOLD );
        }
    }
    for (SmallBullet* bullet : player_bullets_vector) {
        if (!bullet->isDone()) {
            attron( A_BOLD );
            if ( has_colors() ) {
                attron( COLOR_PAIR(MODE_GREEN));
            }
            ncurses_mutex.lock();
            bullet->drawActor();
            ncurses_mutex.unlock();
            if ( has_colors() ) {
                attroff( COLOR_PAIR(MODE_GREEN));
            }
            attroff( A_BOLD );
        }
    }
    for (BigBullet* bullet : big_bullets_vector) {
        if (!bullet->isDone()) {
            attron( A_BOLD );
            if ( has_colors() ) {
                attron( COLOR_PAIR(MODE_RED));
            }
            ncurses_mutex.lock();
            bullet->drawActor();
            ncurses_mutex.unlock();
            if ( has_colors() ) {
                attroff( COLOR_PAIR(MODE_RED));
            }
            attroff( A_BOLD );
        }
    }
}
/**
 * Shoot the bullet on a vertical course
 * @param bullet the bullet to be shot
 * @param dir the direction, "UP" or "DOWN"
 * @param speed the bullet's speed in rows per second
 */
void shoot_small_bullets() {
    int milis_per_row = 1000/small_bullets_speed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (!game_over) {
        small_bullets_mutex.lock();
        for (SmallBullet* bullet : small_bullets_vector) {
            if (!bullet->isDone()) {
                bullet->move(0, 1);
            }
        }
        small_bullets_mutex.unlock();
        player_bullets_mutex.lock();
        for (SmallBullet* bullet : player_bullets_vector) {
            if (!bullet->isDone()) {
                bullet->move(0, -1);
            }
        }
        player_bullets_mutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}
/**
 * Creates bullets to be shot by specified player
 * and shoots them by launching threads for them.
 *
 * Contains bullets_vector_mutex critical section
 * @param player the player who shoots
 */
void player_shoots(Game_actor &player) {
    // Create the bullet
    SmallBullet* bullet = new SmallBullet( short(player.getPos_x() + player.getWidth()/2), short(player.getPos_y()), 0, getmaxx( stdscr ), 0,
                                           player.getPos_y());
    bullet->move_direction = UP;
    // Shoot the bullets
    player_bullets_mutex.lock(); // Critical section - adding data to the small bullets vectors
    player_bullets_vector.push_back(bullet);
    player_bullets_mutex.unlock(); // End of critical section
}
/**
 * Draws the enemies on the screen
 *
 * Contains a enemies_mutex critical section
 */
void draw_enemies() {
    big_enemies_mutex.lock();
    for(Game_actor* enemy : big_slow_enemies_vector) {
        ncurses_mutex.lock();
        enemy->drawActor();
        ncurses_mutex.unlock();
    }
    big_enemies_mutex.unlock();

    small_enemies_mutex.lock();
    for (Game_actor* enemy : small_fast_enemies_vector) {
        ncurses_mutex.lock();
        enemy->drawActor();
        ncurses_mutex.unlock();
    }
    small_enemies_mutex.unlock();
};

void draw_health(Player &player) {
    int hp = player.getHit_points();
    int offset = 9;
    mvprintw(0,0, "HEALTH: [");
    if (has_colors()) {
        attron( COLOR_PAIR(MODE_GREEN));
    }
    for (int i = 0; i < 10 ; ++i) {
        if ( hp == 0 || i > hp / 10 ) {
            mvprintw(0, i+offset, " ");
        } else {
            mvprintw(0, i+offset, "#");
        }
    }
    if (has_colors()) {
        attroff( COLOR_PAIR(MODE_GREEN));
    }
    mvprintw(0,10+offset, "]");
}
/// Big enemies functions
/**
 * Changes the coordinates of the big slow enemies.
 * They go from left to right, or right to left. When they reach the wall,
 * they go down one row. With 1% probbility they can change the route unexpextedly and
 * go down one row. When they reach the bottom of the screen, the game is over.
 */
void move_big_slow_enemies() {
    int milis_per_column = 1000/big_slow_enemy_speed;
    std::chrono::milliseconds t_col(milis_per_column);
    while(!game_over) {
        for (Enemy_big_slow* enemy : big_slow_enemies_vector) {
            if ( dice() > 99) {
                enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                enemy->move(0, 1);
                if (isHit(enemy,shield)) {
                    enemy->move(0, -1);
                }
            }
            if (enemy->move_direction == RIGHT) {
                if (enemy->getPos_x() + enemy->getWidth() < enemy->getMax_x()) {
                    enemy->move(1, 0);
                    if (isHit(enemy,shield)) {
                        enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                        enemy->move(-2, 0);
                    }
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = LEFT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
            if (enemy->move_direction == LEFT) {
                if (enemy->getPos_x() > enemy->getMin_x()) {
                    enemy->move(-1, 0);
                    if (isHit(enemy,shield)) {
                        enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                        enemy->move(2, 0);
                    }
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = RIGHT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
        }
        std::this_thread::sleep_for(t_col);
    }
}
/**
 *
 */
void create_big_slow_enemies_bullets() {
    std::chrono::milliseconds t_bullet(t_big_enemies_bullets);
    while (!game_over) {
        for (Enemy_big_slow *enemy : big_slow_enemies_vector) {
            big_slow_enemy_shoots(*enemy);
        }
        std::this_thread::sleep_for(t_bullet);
    }
}
/**
 * Shoot the bullet on a vertical course
 * @param bullet the bullet to be shot
 * @param dir the direction, "UP" or "DOWN"
 * @param speed the bullet's speed in rows per second
 */
void shoot_big_bullets() {
    int milis_per_row = 1000/big_bullets_speed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (!game_over) {
        big_bullets_mutex.lock();
        for (BigBullet* bullet : big_bullets_vector) {
            if (!bullet->isDone()) {
                bullet->move(0, bullet->move_direction == DOWN ? short(1) : short(-1));
            }
        }
        big_bullets_mutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}
/**
 * Shoots the bullet from specified big slow enemy
 * @param enemy the enemy to shoot the bullet
 */
void big_slow_enemy_shoots(Enemy_big_slow &enemy) {
    // Create the bullets
    BigBullet* bullet = new BigBullet( short(enemy.getPos_x() + enemy.getWidth()/2 - 1), short(enemy.getPos_y()+1), 0, getmaxx( stdscr ), 0,
                                       getmaxy( stdscr ) + 3);
    bullet->move_direction = DOWN;
    // Shoot the bullets
    big_bullets_mutex.lock(); // Critical section - adding data to the bullets vectors
    big_bullets_vector.push_back(bullet);
    big_bullets_mutex.unlock(); // End of critical section
}
/**
 *
 */
void create_big_enemy() {
    int stdscr_maxx = getmaxx( stdscr );
    int stdscr_maxy = getmaxy( stdscr );
    while (!game_over) {
        Enemy_big_slow* enemy_big_slow = new Enemy_big_slow( stdscr_maxx/dice(), 0, 0, stdscr_maxx, 0, stdscr_maxy );
        enemy_big_slow->move_direction = RIGHT;
        big_enemies_mutex.lock();
        big_slow_enemies_vector.push_back(enemy_big_slow);
        big_enemies_mutex.unlock();
        std::this_thread::sleep_for(t_between_big_enemies);
    }
}

/// Small enemies functions
/**
 *
 * @param enemy
 */
void small_fast_enemy_shoots(Enemy_small_fast &enemy) {
    // Create the bullets
    SmallBullet* bullet = new SmallBullet( short(enemy.getPos_x() + enemy.getWidth()/2 ), short(enemy.getPos_y()), 0, getmaxx( stdscr ), 0,
                                           getmaxy( stdscr ));
    bullet->move_direction = DOWN;
    // Shoot the bullets
    small_bullets_mutex.lock(); // Critical section - adding data to the bullets vectors
    small_bullets_vector.push_back(bullet);
    small_bullets_mutex.unlock(); // End of critical section
}
/**
 *
 */
void create_small_fast_enemies_bullets() {
    std::chrono::milliseconds t_bullet(t_small_enemies_bullets);
    while (!game_over) {
        for (Enemy_small_fast *enemy : small_fast_enemies_vector) {
            small_fast_enemy_shoots(*enemy);
        }
        std::this_thread::sleep_for(t_bullet);
    }
}
/**
 *
 */
void create_small_enemy() {
    int stdscr_maxx = getmaxx( stdscr );
    int stdscr_maxy = getmaxy( stdscr );
    while (!game_over) {
        Enemy_small_fast* enemy_small_fast = new Enemy_small_fast( stdscr_maxx/dice(), 0, 0, stdscr_maxx, 0, stdscr_maxy );
        enemy_small_fast->move_direction = LEFT;
        small_enemies_mutex.lock();
        small_fast_enemies_vector.push_back(enemy_small_fast);
        small_enemies_mutex.unlock();
        std::this_thread::sleep_for(t_between_small_enemies);
    }
}
/**
 * Changes the coordinates of the big slow enemies.
 * They go from left to right, or right to left. When they reach the wall,
 * they go down one row. With 1% probbility they can change the route unexpextedly and
 * go down one row. When they reach the bottom of the screen, the game is over.
 */
void move_small_fast_enemies() {
    int milis_per_column = 1000/small_fast_enemy_speed;
    std::chrono::milliseconds t_col(milis_per_column);
    while(!game_over) {
        for (Enemy_small_fast* enemy : small_fast_enemies_vector) {
            if ( dice() > 95) {
                enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                enemy->move(0, 1);
                if (isHit(enemy,shield)) {
                    enemy->move(0, -1);
                }
            }
            if (enemy->move_direction == RIGHT) {
                if (enemy->getPos_x() + enemy->getWidth() < enemy->getMax_x()) {
                    enemy->move(1, 0);
                    if (isHit(enemy,shield)) {
                        enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                        enemy->move(-2, 0);
                    }
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = LEFT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
            if (enemy->move_direction == LEFT) {
                if (enemy->getPos_x() > enemy->getMin_x()) {
                    enemy->move(-1, 0);
                    if (isHit(enemy,shield)) {
                        enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                        enemy->move(2, 0);
                    }
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = RIGHT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
        }
        std::this_thread::sleep_for(t_col);
    }
}
///////////////////////////////////////////////////////////

int main() {

    /// Initialize ncurses
    initscr();
    keypad( stdscr, TRUE );
    curs_set( FALSE );
    noecho();

    if( has_colors() )
    {
        start_color();
        init_pair( MODE_GREEN, COLOR_GREEN, COLOR_BLACK );
        init_pair( MODE_RED, COLOR_RED, COLOR_BLACK );
    }
    /// Create player and shield
    int stdscr_maxx = getmaxx( stdscr );
    int stdscr_maxy = getmaxy( stdscr );

    while (true) {
        if (has_colors()) attron( COLOR_PAIR(MODE_RED));
        attron(A_BOLD);
        mvprintw(stdscr_maxy/2-4, stdscr_maxx/2 -7, "SPACE INVADERS");
        attroff(A_BOLD);
        if (has_colors()) attroff( COLOR_PAIR(MODE_RED));
        mvprintw(stdscr_maxy/2-1, stdscr_maxx/2 -14, "* Move your ship left with 'a' and right with 'd'");
        mvprintw(stdscr_maxy/2, stdscr_maxx/2 -14, "* Shoot with space");
        mvprintw(stdscr_maxy/2+1, stdscr_maxx/2 -14, "The game finishes when your health goes down to 0,");
        mvprintw(stdscr_maxy/2+2, stdscr_maxx/2-14, "or one of the invader's ships reaches the Earth!");
        mvprintw(stdscr_maxy/2+3, stdscr_maxx/2 -14, "Press 'q to quit, any other key to start!");
        mvprintw(stdscr_maxy/2+4, stdscr_maxx/2-14, "Good luck! ;)");
        int c = getch();
        if(c != ERR) {
            if (c == 'q') {
                clear();
                refresh();
                return 0;
            }
            break;
        }
    }

    Player* player = new Player(stdscr_maxx/2 - 3, stdscr_maxy - 1, 0, stdscr_maxx, 0, stdscr_maxy);
    shield = new Shield(stdscr_maxx/2 - 10, stdscr_maxy - 7, stdscr_maxx, 0, stdscr_maxy, 0);
    /// Launch view refresh thread
    std::thread refresh_thread( refresh_view, std::ref(*player));

    while (true) {
        int key = getch();
        if ( key == 'q') {
            exit_condition = true;
            break;
        }

        if ( key == SPACE ) {
            player_shoots(*player);
        }

        if ( key == 'a') {
            /// Move player left
            player_mutex.lock();
            player->move(-1, 0);
            player_mutex.unlock();
        }

        if ( key == 'd') {
            /// Move player right
            player_mutex.lock();
            player->move(1, 0);
            player_mutex.unlock();
        }

    }
    refresh_thread.join();
    endwin();
    return 0;
}