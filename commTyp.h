#ifndef COMM_TYPE
#define COMM_TYPE

#include <iostream>
#include <mpi.h>
#include "GameOfLife.h"

class commTyp
{
public:
    // constructor for top and bottom boundaries
    commTyp(GameOfLife* &game, int proc, bool top, vector<int> order);
    // constructor for latheral boundaries
    commTyp(GameOfLife* &game, int proc, int beg, int end, bool left, bool first,\
    bool last, vector<int> order, pair<vector<int>, vector<int>> h_neighbours);
    
    // destructor
    ~commTyp();

    // proc to communicate with
    int proc;
    // send and receive data
    MPI_Datatype mpi_send, mpi_recv;
    int tag_send;
    int tag_recv;
};


#endif