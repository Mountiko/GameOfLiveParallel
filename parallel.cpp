#include <iostream>
#include <mpi.h>
#include <vector>
#include <cmath>
#include "parallel.h"

using namespace std;

vector<int> distribute(int id, int p, int x1, int x2)
/*
id:     processors id
p:      total amount of processors
x1, x2: dimensions of global domain

return:
    a vector of the:
        size = amount of columns of processors
    each element = amount of processors in each column
    
    The distribution is optimised depending on the rate of x1/x2
    with the pupose of achiving local domains as close to 
    squares as possible
*/
{
    // define rate of domain = length/width
    double rate = (double)(x1)/x2;

    // define variables for best devider
    // and minimum difference in rate
    int best_div = p;
    double min_diff = p;
    // try all possible deviders
    for (int i = 1; i < p; i++)
    {
        // get rate with current devider
        double cur_rate = 1/(((double)p) / i / i);
        // get difference of optimal rate and current rate
        double diff = sqrt(pow(cur_rate - rate, 2));

        // If current rate is closer to optimum rate,
        // then update variables of best devider and minimum
        // difference of best rate and optimum rate
        if (diff <= min_diff)
        {
            min_diff = diff;
            best_div = p / i;
        }
    }

    // compute the leftover of the devider with the best fit
    int leftover = p % best_div;

    // initiate a vector that stores all the
    // arrangement of the split domains
    vector<int> order;
    for (int i = 0; i < best_div; i++)
    {
        if (i < leftover)
            order.push_back(p/best_div + 1);
        else order.push_back(p/best_div);
    }

    return order;
}

void getLocalDomain(int id, int p, int x1, int x2, vector<int> order,\
int* x1_local, int* x2_local)
/*
id:         processors id
p:          total amount of processors
x1, x2:     dimensions of global domain
order:      vector that maps out the arrangement of the processors 
            within the global domain (distribute, see above)
x1_local, x2_local:     dimensions of local domain,
                        to be assigned in this function
*/
{
    // Assigning new x2_local values to each processor.
    // The processors on the left edge of the domain 
    // get the leftover cells 
    if (id % order.size() < x2 % order.size())
        *x2_local = x2 / order.size() + 1;
    else *x2_local = x2 / order.size();

    // Assigning new x1_local values to each processor.
    // The processors at the top edge of the domain
    // get the leftover cells
    if (id % order.size() < (p % order.size()))
    {
        if (id / order.size() < x1 % order.front())
            *x1_local = x1 / order.front() + 1;
        else *x1_local = x1 / order.front();
    }
    else
    {
        if (id / order.size() < x1 % order.back())
            *x1_local = x1 / order.back() + 1;
        else *x1_local = x1 / order.back();
    }
}

pair<int, int> getVerticalNeighbours(int id, int p, vector<int> order, bool periodic)
/*
id:     processors id
p:      total amount of processors
order:  vector that maps out the arrangement of the processors 
        within the global domain (distribute, see above)
periodic:   tells whether the global domain is periodic or not
*/
{
    // initiate pair if ints to return
    pair<int, int> v_neighbours;


    // assign rough idea of which ones the top and bottom neighbours are
    v_neighbours.first = id - order.size(); // top neighbour
    v_neighbours.second = id + order.size(); // bottom neighbour

    // make corrections if neighbour id is negative
    if (v_neighbours.first < 0)
    {
        // is the neighbour in a big column or a small column
        if (abs(v_neighbours.first) > order.size() - p % order.size())
            v_neighbours.first = order.front() * order.size() + v_neighbours.first;
            // if in big column use first value of order to get bottom row of procs
        else 
            v_neighbours.first = order.back() * order.size() + v_neighbours.first;
            // if in small column use last value of order to get bottom row of procs
    }
    // make corrections if neighbour id is larger than total amount if procs
    if (v_neighbours.second >= p)
    {
        // again check if neighbour in big or small column
        if (v_neighbours.second % order.size() < p % order.size())
            v_neighbours.second = v_neighbours.second - order.front() * order.size();
            // if in big column use first value of order to get bottom row of procs

        else
            v_neighbours.second = v_neighbours.second - order.back() * order.size();
            // if in small column use last value of order to get bottom row of procs
    }
    // correct for non periodic case
    if (!periodic && id / order.size() == 0)
        v_neighbours.first = -1;
    else if (!periodic && id / order.size() == order[id % order.size()] - 1)
        v_neighbours.second = -1;

    return v_neighbours;
}

// assuming there is only one neighbour above and below the local domain then
// there can be a max of 3 neighbours on the either side of a local domain.
// diagonal neighbours that connect with only one cell are also considered 
// horizontal neighbours
pair<vector<int>, vector<int>> getHorizontalNeighbours(int id, int p,\
vector<int> order, bool periodic)
/*
id:         processors id
p:          total amount of processors
order:      vector that maps out the arrangement of the processors 
            within the global domain (distribute, see above)
periodic:   tells whether the global domain is periodic or not

In this example with 10 procs, proc 4 would have 2 neighbours on the left
and 3 ni3ghbours on the right. 
_________________________________________________
|              |                |               |
|      0       |        1       |       2       |
|______________|                |               |
|              |________________|_______________|
|      3       |                |               |
|______________|        4       |        5      |
|              |                |               |
|      6       |________________|_______________|
|______________|                |               |
|              |       7        |         8     |
|      9       |                |               |
|______________|________________|_______________|
*/
{
    // get row id and col id
    int row_id = id / order.size();
    int col_id = id % order.size();
    
    // compute the remainder of processors
    int leftover = p % order.size();

    // initiate pair to return
    // first vector is left neighbours
    // second vector is right neighbours
    pair<vector<int>, vector<int>> h_neighbours;

    // get the direct horizontal neighbours
    int left = id / order.size() * order.size() + (id + order.size() - 1) % order.size();
    int right =\
    id / order.size() * order.size() + (id + order.size() + 1) % order.size();
    
    // make correction in case direct neighbour over shoots
    if (left >= p)
        left = left - order.size();
    if (right >= p)
        right = right - order.size();

    // get the vertical neighbours of the each horizontal neighbour
    pair<int, int> left_add_ons = getVerticalNeighbours(left, p, order, periodic);
    pair<int, int> right_add_ons = getVerticalNeighbours(right, p, order, periodic);
    
    // differentiating the different cases

    // first case: square case, all procs have three lateral neighbours
    // with no offsets
    if (leftover == 0)
    {
        h_neighbours.first.push_back(left_add_ons.first);
        h_neighbours.first.push_back(left);
        h_neighbours.first.push_back(left_add_ons.second);

        h_neighbours.second.push_back(right_add_ons.first);
        h_neighbours.second.push_back(right);
        h_neighbours.second.push_back(right_add_ons.second);
    }
    else // otherwise they will need to be classified 
    {

        // left hand neighbours

        // proc is in the first small column and not a boundary proc
        if (col_id == leftover && row_id != 0 && row_id != order.back() - 1)
        {
            h_neighbours.first.push_back(left);
            h_neighbours.first.push_back(left_add_ons.second);
        }
        // proc is in the first small column and at the top boundary
        else if (col_id == leftover && row_id == 0)
        {
            h_neighbours.first.push_back(left_add_ons.first);
            h_neighbours.first.push_back(left);
            h_neighbours.first.push_back(left_add_ons.second);
        }
        // proc is in the first small column and at the bottom boundary
        else if (col_id == leftover && row_id == order.back() - 1)
        {
            pair<int, int> left_add_on_2 =\
            getVerticalNeighbours(left_add_ons.second, p, order, periodic);

            h_neighbours.first.push_back(left);
            h_neighbours.first.push_back(left_add_ons.second);
            h_neighbours.first.push_back(left_add_on_2.second);
        }
        // proc is any other smaller column
        else if (col_id > leftover)
        {
            h_neighbours.first.push_back(left_add_ons.first);
            h_neighbours.first.push_back(left);
            h_neighbours.first.push_back(left_add_ons.second);
        }
        // proc is in big column that is not the first column
        else if (col_id < leftover && col_id != 0)
        {
            h_neighbours.first.push_back(left_add_ons.first);
            h_neighbours.first.push_back(left);
            h_neighbours.first.push_back(left_add_ons.second);
        }
        // proc is in first collumn and not at bottom boundary
        else if (col_id == 0 && row_id != order.front() - 1)
        {
            h_neighbours.first.push_back(left_add_ons.first);
            h_neighbours.first.push_back(left);
        }
        // proc is in first column and at the bottom boundary
        else if (col_id == 0 && row_id == order.front() - 1)
        {
            h_neighbours.first.push_back(left);
            h_neighbours.first.push_back(left_add_ons.second);
        }
        

        // right hand neighbours

        // proc is in last big column and not at bottom boundary
        if (col_id == leftover-1 && row_id != order.front() - 1)
        {
            h_neighbours.second.push_back(right_add_ons.first);
            h_neighbours.second.push_back(right);
        }
        // proc is in last big column and at bottom boundary
        if (col_id == leftover-1 && row_id == order.front() - 1)
        {
            h_neighbours.second.push_back(right);
            h_neighbours.second.push_back(right_add_ons.second);
        }
        // proc is any other big column
        else if (col_id < leftover-1)
        {
            h_neighbours.second.push_back(right_add_ons.first);
            h_neighbours.second.push_back(right);
            h_neighbours.second.push_back(right_add_ons.second);
        }
        // proc is in any small column but not last column
        else if (col_id >= leftover && col_id != order.size() - 1)
        {
            h_neighbours.second.push_back(right_add_ons.first);
            h_neighbours.second.push_back(right);
            h_neighbours.second.push_back(right_add_ons.second);
        }
        // proc is in last column and at top boundary
        else if (col_id == (order.size() - 1) && row_id == 0)
        {
            h_neighbours.second.push_back(right_add_ons.first);
            h_neighbours.second.push_back(right);
            h_neighbours.second.push_back(right_add_ons.second);
        }
        // proc is in last column and at bottom boundary
        else if (col_id == (order.size() - 1) && row_id == order.back() - 1)
        {
            pair<int, int> right_add_on_2 =\
            getVerticalNeighbours(right_add_ons.second, p, order, periodic);

            h_neighbours.second.push_back(right);
            h_neighbours.second.push_back(right_add_ons.second);
            h_neighbours.second.push_back(right_add_on_2.second);
        }
        // proc is in last column and at neither vertical boundary
        else if (col_id == (order.size() - 1) && row_id != 0 && row_id != order.back())
        {
            h_neighbours.second.push_back(right);
            h_neighbours.second.push_back(right_add_ons.second);
        }
    }

    if (!periodic && id % order.size() == 0)
    {    
        h_neighbours.first.clear();
        h_neighbours.first.push_back(-1);
    }
    else if (!periodic && id % order.size() == order.size())
    {    
        h_neighbours.second.clear();
        h_neighbours.second.push_back(-1);
    }
    return h_neighbours;
}


void getPosition(int p, vector<int> order, int* &all_x1_local, int* &x1_pos)
/*
p:              total amount of processors
order:          vector that maps out the arrangement of the processors 
                within the global domain (distribute, see above)
all_x1_local:   all local x1 values of all procs
x1_pos:         x1 coordinates of all procs
*/
{
    // looop over all procs
    for (int i = 0; i < p; i++)
    {
        // set initial value to zero
        x1_pos[i] = 0;
        // add all local x1 values of procs that are closer to top boundary
        for (int j = 0; j < i / order.size(); j++)
        {
            x1_pos[i] += all_x1_local[j * order.size() + i % order.size()];
        }
    }
}

pair<vector<int>, vector<int>> getOffsets(\
int id, pair<vector<int>, vector<int>> h_neighbours, int x1_local, int x1, int* &x1_pos)
/*
id:             processors id
h_neighbours:   all horizontal neigbours
x1_local:       local x1 dimension values
x1:             global x1 dimension values
x1_pos:         x1 coordinates of all procs
*/
{
    // intiate pair of vectors to return
    // first vector is left neighbour offsets
    // second vector is right neighbour offsets
    pair<vector<int>, vector<int>> offsets;

    // first offset of either side is zero
    offsets.first.push_back(0);
    offsets.second.push_back(0);

    // loop over all left neighbours
    for (int i = 0; i < h_neighbours.first.size() - 1; i++)
    {
        // create temporaray variable to push back on vector
        int tmp;
        // compute the the difference between ith + 1 neighbours
        // and position of this processor
        tmp = (x1_pos[h_neighbours.first[i + 1]] - x1_pos[id]);
        // when proc at bottom boundary an neighbour at top boundary
        // the value will be negative and needs to  be corrected
        if (tmp < 0)
            tmp += x1;
        // add the offset to vector
        offsets.first.push_back(tmp);
    }
    // last offset is the local x1 dimension
    offsets.first.push_back(x1_local);

    if (offsets.first.back() == x1)
        offsets.first.back() -= x1;

    // same thing for left neighbours
    for (int i = 0; i < h_neighbours.second.size() - 1; i++)
    {
        int tmp;
        tmp = x1_pos[h_neighbours.second[i + 1]] - x1_pos[id];

        if (tmp < 0)
            tmp += x1;

        offsets.second.push_back(tmp);
    }

    offsets.second.push_back(x1_local);

    if (offsets.second.back() == x1)
        offsets.second.back() -= x1;

    return offsets;
}

vector<commTyp> getComm(GameOfLife* &game, pair<int, int> v_neighbours,\
pair<vector<int>, vector<int>> h_neighbours, pair<vector<int>, vector<int>> offsets,\
bool v_periodic, bool h_periodic, vector<int> order)
/*
game:           GameOfLife object
v_neighbours:   vertical neighbours
h_neighbours:   horizontal neighbours
offsets:        offsets to each horizontal neighbour

This function sets up all the communications necessary
between this proc and all its neighbours
*/
{
    // vector to store all communications, will be returned by this function
    vector<commTyp> comms;

    // initiate to MPI communications types (see commTyp.h, commTyp.cpp)
    // for vertical neighbours

    if (!v_periodic)
    {
        commTyp* top_comm = new commTyp(game, v_neighbours.first, true, order);
        commTyp* bot_comm = new commTyp(game, v_neighbours.second, false, order);

        // add them to communication vector
        if (v_neighbours.first >= 0)
            comms.push_back(*top_comm);
        if (v_neighbours.second >= 0)
            comms.push_back(*bot_comm);

        // delete the commTyp pointers
        delete top_comm;
        delete bot_comm;
    }
    // do the horizntal neighbours 

    // bools telling the position of the left neighbour 
    // first, last or neither 
    bool last;
    bool first;
    // loop over all left neighbours
    for (int i = 0; i < h_neighbours.first.size(); i++)
    {
        // update position status
        first = max(0, 1 - i);
        last = max((int(-h_neighbours.first.size()) + 2 + i), 0);
        
        // get communication
        if ((!h_periodic || (h_periodic && i != 1)) && h_neighbours.first[i] >= 0)
        {
            commTyp* tmp = new commTyp(game, h_neighbours.first[i], offsets.first[i],\
                                    offsets.first[i+1], true, first, last, order, h_neighbours);

            // managing tag numbers for special cases
            if (first && order.front() == 2)
            {
                tmp->tag_send = 10;
                tmp->tag_recv = 40;
            }
            else if (last && order.front() == 2)
            {
                tmp->tag_send = 20;
                tmp->tag_recv = 30;
            }

            if (h_periodic)
            {
                tmp->tag_send = 0;
                tmp->tag_recv = 1;
            }
            if (v_periodic)
            {
                tmp->tag_send = (tmp->tag_send * 10) + i;
                tmp->tag_recv = (tmp->tag_recv * 10) + h_neighbours.first.size() - 1 - i;
            }

            // add communication to comm vector
            comms.push_back(*tmp);
            // delete memory allocated to temporary commTyp ptr
            delete tmp;
        }
    }     
    // do the same for right neighbours
    for (int i = 0; i < h_neighbours.second.size(); i++)
    {
        // update position status
        first = max(0, 1 - i);
        last = max((int(-h_neighbours.second.size()) + 2 + i), 0);

        // get communication
        if ((!h_periodic || (h_periodic && i != 1))  && h_neighbours.second[i] >= 0)
        {
            commTyp* tmp = new commTyp(game, h_neighbours.second[i], offsets.second[i],\
                                        offsets.second[i+1], false, first, last, order, h_neighbours);
            
            if (first && order.front() == 2)
            {
                tmp->tag_send = 30;
                tmp->tag_recv = 20;
            }
            else if (last && order.front() == 2)
            {
                tmp->tag_send = 40;
                tmp->tag_recv = 10;
            }

            if (h_periodic)
            {
                tmp->tag_send = 1;
                tmp->tag_recv = 0;
            }
            
            if (v_periodic)
            {
                tmp->tag_send = (tmp->tag_send * 10) + i;
                tmp->tag_recv = (tmp->tag_recv * 10) + h_neighbours.second.size() - 1 - i;
            }

            // add communication to comm vector
            comms.push_back(*tmp);
            // delete memory allocated to temporary commTyp ptr
            delete tmp;
        }
    }
    return comms;
}

void sendBounds(vector<commTyp> comms)
{
    MPI_Request requests[2 * comms.size()];

    int cnt = 0;

    for (int i = 0; i < comms.size(); i++)
    {
        MPI_Isend(MPI_BOTTOM, 1, comms[i].mpi_send, comms[i].proc,\
                    comms[i].tag_send, MPI_COMM_WORLD, &requests[cnt]);
        cnt++;
        
        MPI_Irecv(MPI_BOTTOM, 1, comms[i].mpi_recv, comms[i].proc,\
                    comms[i].tag_recv, MPI_COMM_WORLD, &requests[cnt]);
        cnt++;
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    // MPI_Waitall(cnt, requests, MPI_STATUSES_IGNORE);
}
