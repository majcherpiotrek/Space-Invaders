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

static const std::chrono::milliseconds frame_durtion(25); // 40 FPS
static const std::chrono::milliseconds t_between_big_enemies(8000); // new big enemy every 8 seconds
static const std::chrono::milliseconds t_between_small_enemies(4000); // new small enemy every 4 seconds
static const int t_big_enemies_bullets= 2000;
static const int t_small_enemies_bullets= 1000;
static const int small_bullets_speed = 40; // rows per second
static const int big_bullets_speed = 15; //rows per sec_ond
static const int big_slow_enemy_speed = 20; // columns per second
static const int small_fast_enemy_speed = 40; // columns per second
static const int SPACE = 32;
static std::atomic_bool exit_condition(false);
static std::atomic_bool game_over(false);
static std::default_random_engine generator;
static std::uniform_int_distribution<int> distribution(1,100);
static auto dice = std::bind ( distribution, generator );

/// Bullets' vector
static std::vector<BigBullet*> big_bullets_vector;
static std::vector<SmallBullet*> small_bullets_vector;

/// Enemies's vector
static std::vector<Enemy_big_slow*> big_slow_enemies_vector;
static std::vector<Enemy_small_fast*> small_fast_enemies_vector;

/// Mutexes
static std::mutex small_bullets_mutex;
static std::mutex big_bullets_mutex;
static std::mutex big_enemies_mutex;
static std::mutex small_enemies_mutex;
static std::mutex player_mutex;

/// Colors' modes
static const short MODE_GREEN = 1;
static const short MODE_RED = 2;

void remove_used_bullets();
void draw_bullets();
void shoot_small_bullets();
void player_shoots(Game_actor &player);
void draw_enemies();
/// Big enemies functions
void move_big_slow_enemies();
void create_big_slow_enemies_bullets();
void shoot_big_bullets();
void big_slow_enemy_shoots(Enemy_big_slow &enemy);
void create_big_enemy();
//////////////////////////////////////////////////

/// Small enemies functions
void move_small_fast_enemies();
void create_small_fast_enemies_bullets();
void small_fast_enemy_shoots(Enemy_small_fast &enemy);
void create_small_enemy();
//////////////////////////////////////////////////

/**
 * A method to be executed in a separate thread. Used to refresh the view.
 * @param exit exit condition
 * @param player a reference to player object
 */
void refresh_view(std::atomic_bool &exit, Player &player) {

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
        player_mutex.lock();
        player.drawActor();
        player_mutex.unlock();

        draw_enemies();
        attroff( A_BOLD );

        remove_used_bullets();
        mvprintw(0,0, "small bullets: %d", small_bullets_vector.size());
        mvprintw(1,0, "big bullets: %d", big_bullets_vector.size());
        mvprintw(2,0, "big enemies: %d", big_slow_enemies_vector.size());

        draw_bullets();

        refresh();

        if (game_over) {
            exit_condition = true;
            attron( A_BOLD );
            attron( COLOR_PAIR(MODE_RED));
            mvprintw( getmaxy( stdscr )/2, getmaxx( stdscr) / 2 - 5, "GAME OVER!");
            attroff( COLOR_PAIR(MODE_RED));
            attroff( A_BOLD );
            refresh();
            getch();
            break;
        }
        std::this_thread::sleep_for(frame_durtion);
    }
    game_over = true;
    clear();
    mvprintw( 0, 0, "Finishing threads...");
    refresh();
    big_enemies_creation_thread.join();
    mvprintw( 1, 0, "- big enemies creation thread: FINISHED");
    small_enemies_creation_thread.join();
    mvprintw( 2, 0, "- small enemies creation thread: FINISHED");
    move_big_slow_enemies_thread.join();
    mvprintw( 3, 0, "- big enemies motion thread: FINISHED");
    move_small_fast_enemies_thread.join();
    mvprintw( 4, 0, "- small enemies motion thread: FINISHED");
    big_slow_enemies_shooting_thread.join();
    mvprintw( 5, 0, "- big enemies shooting thread: FINISHED");
    small_fast_enemies_shooting_thread.join();
    mvprintw( 6, 0, "- small enemies shooting thread: FINISHED");
    big_bullets_thread.join();
    mvprintw( 7, 0, "- big bullets motion thread: FINISHED");
    small_bullets_thread.join();
    mvprintw( 8, 0, "- small bullets motion thread: FINISHED");
    mvprintw( 9, 0, "Finished all tasks!");
    refresh();
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
}
/**
 * Prints the bullets shot by the player
 */
void draw_bullets() {
    for (SmallBullet* bullet : small_bullets_vector) {
        if (!bullet->isDone()) {
            attron( A_BOLD );
            if ( has_colors() ) {
                attron( COLOR_PAIR(MODE_GREEN));
            }
            bullet->drawActor();
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
            bullet->drawActor();
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
                bullet->move(0, bullet->move_direction == DOWN ? short(1) : short(-1));
            }
        }
        small_bullets_mutex.unlock();
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
    small_bullets_mutex.lock(); // Critical section - adding data to the small bullets vectors
    small_bullets_vector.push_back(bullet);
    small_bullets_mutex.unlock(); // End of critical section
}
/**
 * Draws the enemies on the screen
 *
 * Contains a enemies_mutex critical section
 */
void draw_enemies() {
    big_enemies_mutex.lock();
    for(Game_actor* enemy : big_slow_enemies_vector) {
        enemy->drawActor();
    }
    big_enemies_mutex.unlock();

    small_enemies_mutex.lock();
    for (Game_actor* enemy : small_fast_enemies_vector) {
        enemy->drawActor();
    }
    small_enemies_mutex.unlock();
};

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
            }
            if (enemy->move_direction == RIGHT) {
                if (enemy->getPos_x() + enemy->getWidth() < enemy->getMax_x()) {
                    enemy->move(1, 0);
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
//////////////////////////////////////////////////////////

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
            }
            if (enemy->move_direction == RIGHT) {
                if (enemy->getPos_x() + enemy->getWidth() < enemy->getMax_x()) {
                    enemy->move(1, 0);
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = LEFT;
                }
//                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
//                    game_over = true;
//                }
                continue;
            }
            if (enemy->move_direction == LEFT) {
                if (enemy->getPos_x() > enemy->getMin_x()) {
                    enemy->move(-1, 0);
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = RIGHT;
                }
//                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
//                    game_over = true;
//                }
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
    /// Create player and enemies
    int stdscr_maxx = getmaxx( stdscr );
    int stdscr_maxy = getmaxy( stdscr );

    Player* player = new Player(stdscr_maxx/2 - 3, stdscr_maxy - 1, 0, stdscr_maxx, 0, stdscr_maxy);

    /// Launch view refresh thread
    std::thread refresh_thread( refresh_view, std::ref(exit_condition), std::ref(*player));

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
    printw("\n\nExited all - Press any key to close the program!\n");
    getch();
    endwin();
    return 0;
}