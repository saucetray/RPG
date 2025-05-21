
#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 12;
const int NUM_ENEMIES = 3;

struct Entity {
    int x, y;
    int hp;
    int attack;
    bool alive;
};

bool isWall(int x, int y) {
    return (x == 0 || y == 0 || x == MAP_WIDTH-1 || y == MAP_HEIGHT-1);
}

void drawMap(const Entity &player, const vector<Entity> &enemies) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (isWall(x, y)) {
                mvaddch(y, x, '#');
            } else if (player.x == x && player.y == y) {
                mvaddch(y, x, '@');
            } else {
                bool enemyHere = false;
                for (const auto &e : enemies) {
                    if (e.alive && e.x == x && e.y == y) {
                        mvaddch(y, x, 'E');
                        enemyHere = true;
                        break;
                    }
                }
                if (!enemyHere)
                    mvaddch(y, x, '.');
            }
        }
    }
}

void drawSidebar(const Entity &player, const vector<Entity> &enemies) {
    int offset = MAP_WIDTH + 2;
    mvprintw(1, offset, "-- Player --");
    mvprintw(2, offset, "HP: %d   ATK: %d", player.hp, player.attack);
    mvprintw(4, offset, "-- Enemies --");
    int line = 5;
    int i = 1;
    for (const auto &e : enemies) {
        mvprintw(line++, offset, "E%d (%s) HP: %d", i++, (e.alive ? "alive" : "dead"), e.hp);
    }
    mvprintw(line+1, offset, "[@=You, E=Enemy, #=Wall]");
    mvprintw(line+3, offset, "Move: arrows, Attack: walk into E, Quit: q");
}

int battle_ncurses(Entity &player, Entity &enemy) {
    clear();
    mvprintw(0, 0, "You encountered an enemy! Battle begins!");
    int logrow = 2;
    while (player.hp > 0 && enemy.hp > 0) {
        mvprintw(1, 0, "Player HP: %d   Enemy HP: %d   [Press ANY KEY to attack]          ", player.hp, enemy.hp);
        getch();
        enemy.hp -= player.attack;
        mvprintw(logrow++, 0, "You hit the enemy for %d damage.", player.attack);
        if (enemy.hp <= 0) break;
        player.hp -= enemy.attack;
        mvprintw(logrow++, 0, "Enemy strikes back for %d damage.", enemy.attack);
    }
    logrow++;
    if (player.hp <= 0) {
        mvprintw(logrow++, 0, "You have been defeated. Game Over. Press key.");
        getch();
        return 0; // Lose
    } else {
        mvprintw(logrow++, 0, "Enemy defeated! Press key.");
        getch();
        return 1; // Win
    }
}

bool positionOccupied(int x, int y, const vector<Entity> &entities) {
    for (const auto &e : entities) {
        if (e.alive && e.x == x && e.y == y)
            return true;
    }
    return false;
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    srand(time(nullptr));

    Entity player{1, 1, 50, 10, true};
    vector<Entity> enemies;
    // Place enemies at random open locations
    for (int i = 0; i < NUM_ENEMIES; ++i) {
        int ex, ey;
        do {
            ex = 2 + rand() % (MAP_WIDTH - 4);
            ey = 2 + rand() % (MAP_HEIGHT - 4);
        } while ((ex == player.x && ey == player.y) || positionOccupied(ex, ey, enemies));
        int hp = 18 + rand() % 15;
        int atk = 5 + rand() % 7;
        enemies.push_back(Entity{ex, ey, hp, atk, true});
    }

    bool running = true;
    int message_y = MAP_HEIGHT + 1;
    while (running) {
        clear();
        drawMap(player, enemies);
        drawSidebar(player, enemies);
        mvprintw(MAP_HEIGHT, 0, "Use arrows/WASD to move. q = quit");
        refresh();

        if (player.hp <= 0) {
            mvprintw(message_y, 0, "You are DEAD!");
            refresh();
            getch();
            break;
        }

        bool allDead = true;
        for (const auto &e : enemies) if (e.alive) allDead = false;
        if (allDead) {
            mvprintw(message_y, 0, "All enemies defeated! You win!");
            refresh();
            getch();
            break;
        }

        int dx = 0, dy = 0;
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
        if (ch == KEY_UP || ch=='w' || ch=='W') dy = -1;
        else if (ch == KEY_DOWN || ch=='s' || ch=='S') dy = 1;
        else if (ch == KEY_LEFT || ch=='a' || ch=='A') dx = -1;
        else if (ch == KEY_RIGHT || ch=='d' || ch=='D') dx = 1;

        int nx = player.x + dx;
        int ny = player.y + dy;
        if (!isWall(nx, ny)) {
            // Check for enemy collision
            bool fought = false;
            for (auto &e : enemies) {
                if (e.alive && e.x == nx && e.y == ny) {
                    int result = battle_ncurses(player, e);
                    if (result) e.alive = false;
                    else player.alive = false;
                    fought = true;
                    break;
                }
            }
            if (!fought) {
                player.x = nx;
                player.y = ny;
            }
        }
    }
    endwin();
    return 0;
}

