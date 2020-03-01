#ifndef Game_of_Life
#define Game_of_Life

#include <iostream>

using namespace std;

class GameOfLife
{
public:
    // constructor
    GameOfLife(int id, int x1, int x2, bool v_periodic, bool h_periodic);
    // destructor
    ~GameOfLife();

    void printGame(bool show_buffer);

    void writeToFile(bool write_buffer, int time_stp);

    void solveOnce();
    
    void getBuffer();

    /*
    void getTop(int* &top_bound);
    void getBot(int* &bot_bound);
    void getLeft(int* &left_bound);
    void getRight(int* &right_bound);
    */

    int x1 = -1; // rows of domain
    int x2 = -1; // cols of domain
    int x1_buf = -1;
    int x2_buf = -1;
    int cells = -1;
    // values pointer is 1D-array that row-major order.
    bool* values;
    bool v_periodic;
    bool h_periodic;
    int id;

private:
    
};

#endif
