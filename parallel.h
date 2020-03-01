#ifndef PARALLEL
#define PARALLEL

#include <iostream>
#include <vector>
#include "commTyp.h"
#include "GameOfLife.h"

using namespace std;

vector<int> distribute(int id, int p, int x1, int x2);

void getLocalDomain(int id, int p, int x1, int x2, vector<int> order,\
                    int* x1_local, int* x2_local);

pair<int, int> getVerticalNeighbours(int id, int p, vector<int> order, bool periodic);

// assuming there is only one neighbour above and below the local domain
// the can be a max of 3 neighbours on the either side of a local domain.
// diagonal neighbours that connect with only one cell are also considered 
// horizontal neighbours
pair<vector<int>, vector<int>> getHorizontalNeighbours(int id, int p,\
vector<int> order, bool periodic);

void getPosition(int p, vector<int> order, int* &all_x1_local, int* &x1_pos);

pair<vector<int>, vector<int>> getOffsets(\
int id, pair<vector<int>, vector<int>> h_neighbours, int x1_local, int x1, int* &x1_pos);

vector<commTyp> getComm(GameOfLife* &game, pair<int, int> v_neighbours,\
pair<vector<int>, vector<int>> h_neighbours,\
pair<vector<int>, vector<int>> offsets, bool v_periodic, bool h_periodic,\
vector<int> order);

void sendBounds(vector<commTyp> comms);


#endif