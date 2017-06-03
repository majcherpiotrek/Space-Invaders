#include <iostream>
#include <chrono>
#include <thread>
#include <ncurses.h>
#include <mutex>
#include <vector>
#include <atomic>
#include "Bullet.h"
#include "Direction.h"
#include "Player.h"

static const std::chrono::milliseconds frame_durtion(30);
static const int SPACE = 32;
static std::vector<Bullet*> player_bullets_vector;
static std::vector<std::thread> bullet_threads_vector;
static std::mutex bullets_vector_mutex;
static std::mutex bullet_mutex;
static std::mutex player_mutex;

/**
 * A method to be executed in a separate thread. Used to refresh the view.
 * @param exit exit condition
 * @param player a reference to player object
 */
void refresh_view(std::atomic_bool &exit, Player &player) {
    while (!exit) {
        clear();

        player_mutex.lock();
        player.drawPlayer();
        player_mutex.unlock();

        /// Check if there are bullets to remove
        bullets_vector_mutex.lock();
        std::vector<Bullet*>::iterator it = player_bullets_vector.begin();
        int j = 0;
        while (it != player_bullets_vector.end()) {
            if (player_bullets_vector[j]->isDone()) {
                it = player_bullets_vector.erase(it);
            } else {
                j++; it++;
            }
        }
        bullets_vector_mutex.unlock();

        ///Bullets vector access critical section start
        bullets_vector_mutex.lock();
        for (int i = 0; i < player_bullets_vector.size();) {
            bullet_mutex.lock();
            if (!player_bullets_vector[i]->isDone()) {
                attron( A_BOLD );
                mvprintw(player_bullets_vector[i]->getRow(), player_bullets_vector[i]->getColumn(), player_bullets_vector[i]->getSign());
                attroff( A_BOLD );
            }
            i++;
            bullet_mutex.unlock();
        }
        bullets_vector_mutex.unlock();
        ///Bullets vector access critical section end

        refresh();
        std::this_thread::sleep_for(frame_durtion);
    }
    clear();
    printw("exiting!");
    refresh();
}

/**
 * Shoot the bullet on a vertical course
 * @param bullet the bullet to be shot
 * @param dir the direction, "UP" or "DOWN"
 * @param speed the bullet's speed in rows per second
 */
void shoot_bullet(Bullet &bullet, Direction dir, unsigned int move_limit, unsigned int speed) {
    int milis_per_row = 1000/speed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (bullet.getRow() != move_limit) {
        bullet_mutex.lock();
        bullet.move(0, dir == DOWN ? short(1) : short(-1));
        bullet_mutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
    bullet_mutex.lock();
    bullet.setDone();
    bullet_mutex.unlock();
}

int main() {

    /// Initialize ncurses
    initscr();
    keypad( stdscr, TRUE );
    curs_set( FALSE );
    noecho();

    /// Create player and enemies
    Player* player = new Player( getmaxy( stdscr ) - 1, getmaxx( stdscr ) /2 - 3, 0, getmaxx( stdscr ) );
    std::atomic_bool exit(false);
    std::thread refresh_thread( refresh_view, std::ref(exit), std::ref(*player));
    while (true) {
        int key = getch();
        if ( key == 'q') {
            exit = true;
            break;
        }

        if ( key == SPACE ) {
            /// Player shoots
            Bullet* bullet_left = new Bullet( short(player->getRow()), short(player->getPosX()), "*", COLOR_GREEN);
            Bullet* bullet_right = new Bullet( short(player->getRow()), short(player->getPosX() + player->getWidth()), "*", COLOR_GREEN);
            bullets_vector_mutex.lock();
            player_bullets_vector.push_back(bullet_left);
            player_bullets_vector.push_back(bullet_right);
            bullets_vector_mutex.unlock();
            bullet_threads_vector.push_back(std::thread(shoot_bullet, std::ref(*bullet_left), UP, 0, 20));
            bullet_threads_vector.push_back(std::thread(shoot_bullet, std::ref(*bullet_right), UP, 0, 20));
        }

        if ( key == 'a') {
            /// Move player left
            player_mutex.lock();
            player->move(-1);
            player_mutex.unlock();
        }

        if ( key == 'd') {
            /// Move player right
            player->move(1);
        }

    }

    for (auto &t : bullet_threads_vector) {
        t.join();
    }

    refresh_thread.join();
    printw("Exited ;)");
    getch();
    endwin();
    return 0;
}