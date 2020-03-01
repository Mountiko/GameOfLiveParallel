#include <iostream>
#include <ctime>
#include <vector>
#include <mpi.h>
#include "commTyp.h"


using namespace std;

// constructor for vertical communication
commTyp::commTyp(GameOfLife* &game, int proc, bool top, vector<int> order): proc(proc)
/*
game:   GameOfLife object that communicates
proc:   processor to communicate with
top:    bool to define whether top or bottom neighbour is communicating
*/
{
    // initiate vectors to store
    // length, address and type of comm data
    vector<int> block_length;
    vector<MPI_Aint> addresses;
    vector<MPI_Datatype> typelist;

    
    if (top)
    {
        // define tag
        this->tag_send = (game->id / order.size()) % order[game->id % order.size()];
        this->tag_recv = ((game->id / order.size()) % order[game->id % order.size()]);

        // fill mpi_send
        
        // length is local x2 dimansion game in this proc
        block_length.push_back(game->x2);
        MPI_Aint temp;
        // get the address from first non padded element and push it onto addresses vector
        MPI_Get_address(&game->values[game->x2_buf + 1], &temp);
        addresses.push_back(temp);
        typelist.push_back(MPI_C_BOOL); // data type is bool

        // create the MPI type and call it this->mpi_send
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_send);
        // commit the type
        MPI_Type_commit(&this->mpi_send);

        // clear the vectors to reuse them for receive
        block_length.clear();
        addresses.clear();
        typelist.clear();

        // fill mpi_recv

        // receives array of same length since corners are done in horizontal neighbours
        block_length.push_back(game->x2);
        MPI_Aint temp2;
        // values address starts at second element of values array
        // this is where received data will be stored
        MPI_Get_address(&game->values[1], &temp2); // corner is excluded
        addresses.push_back(temp2);
        typelist.push_back(MPI_C_BOOL); // datatype bool
        
        // create the MPI type and call it this->mpi_recv
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_recv);
        // commit the type
        MPI_Type_commit(&this->mpi_recv);
    }
    // do the same thing for bottom neighbour
    else
    {
        // define tag
        this->tag_send = ((game->id / order.size() + 1) % order[game->id % order.size()]);
        this->tag_recv = (game->id / order.size() + 1) % order[game->id % order.size()];

        // fill mpi_send

        // length is local x2 dimansion game in this proc
        block_length.push_back(game->x2);
        MPI_Aint temp;
        // get the address from first non padded element at bottom edge and push it onto addresses vector
        MPI_Get_address(&game->values[game->x2_buf * game->x1 + 1], &temp);
        addresses.push_back(temp);
        typelist.push_back(MPI_C_BOOL); // data type is bool

        // create the MPI type and call it this->mpi_send
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_send);

        // commit the type
        MPI_Type_commit(&this->mpi_send);

        // clear the vectors to reuse them for receive
        block_length.clear();
        addresses.clear();
        typelist.clear();

        // fill mpi_recv

        // length is local x2 dimansion game in this proc
        block_length.push_back(game->x2);
        MPI_Aint temp2;
        // get the address from first padded element at bottom edge and push it onto addresses vector
        // this is where received data will be stored
        MPI_Get_address(&game->values[game->x2_buf * (game->x1+1) + 1], &temp2); // corners excluded
        addresses.push_back(temp2);
        typelist.push_back(MPI_C_BOOL); // data type is bool
        

        // create the MPI type and call it this->mpi_recv
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_recv);

        // commit the type
        MPI_Type_commit(&this->mpi_recv);
    }
}

// constructor for vertical communication
commTyp::commTyp(GameOfLife* &game, int proc, int beg, int end,\
bool left, bool first, bool last, vector<int> order, pair<vector<int>, vector<int>> h_neighbours): proc(proc)
/*
game:           GameOfLife object that communicates
proc:           processor to communicate with
left:           bool to define whether left or right neighbour is communicating
first, last:    bools that define position of neighbour 
*/
{    
    // bool that defines if sending array had to be 
    // truncated because processor domain ended
    bool one_smaller_end = false;
    bool one_smaller_beg = false;

    // truncate end of array by one if necessary
    if (end == game->x1)
    {
        end -= 1;
        one_smaller_end = true;
    }
    if (beg - 1 >= 0)
    {
        beg -= 1;
        one_smaller_beg = true;
    }
    
    // initiate vectors to store
    // length, address and type of comm data
    vector<int> block_length;
    vector<MPI_Aint> addresses;
    vector<MPI_Datatype> typelist;

    // do left neighbours
    if (left)
    {
        // define tag
        this->tag_send = (game->id % order.size()) % order.size();
        this->tag_recv = (game->id % order.size()) % order.size();

        for (int i = beg; i <= end; i++)
        {
            // add a one to the blocklength for every element added
            block_length.push_back(1);
            MPI_Aint temp;
            // get the address of the left boundary of local domain
            MPI_Get_address(&game->values[game->x2_buf * (i + 1) + 1], &temp);
            addresses.push_back(temp);
            typelist.push_back(MPI_C_BOOL); // data type is bool
        }
        
        // create the MPI type and call it this->mpi_send
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_send);
        // commit the type
        MPI_Type_commit(&this->mpi_send);

        // clear the vectors to reuse them for receive
        block_length.clear();
        addresses.clear();
        typelist.clear();
        
        // changing the ranges form inner coord-system without buffer
        // to outer coord system including buffer layer
        if ((beg != 0 && !first) || (beg == 0 && !first && one_smaller_beg))
            beg += 2;
        else if (beg == 0 && !first && !one_smaller_beg)
            beg += 1;
        if (end == (game->x1 - 1) && last)
            end += 2;
        else if (end == (game->x1 - 1) && !last && one_smaller_end)
            end += 1;

        for (int i = beg; i <= end; i++)
        {
            // add a one to the blocklength for every element added
            block_length.push_back(1);
            MPI_Aint temp;
            // get the address of the left buffer of local domain
            // this is where received data will be stored
            MPI_Get_address(&game->values[game->x2_buf * i], &temp);
            addresses.push_back(temp);
            typelist.push_back(MPI_C_BOOL); // data type is bool
        }

        // create the MPI type and call it this->mpi_recv
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_recv);
        // commit the type
        MPI_Type_commit(&this->mpi_recv);
    }
    // do right neighbours
    else
    {
        // define tag
        this->tag_send = (game->id % order.size() + 1) % order.size();
        this->tag_recv = (game->id % order.size() + 1) % order.size();

        for (int i = beg; i <= end; i++)
        {
            // add a one to the blocklength for every element added
            block_length.push_back(1);
            MPI_Aint temp;
            // get the address of the right boundary of local domain
            MPI_Get_address(&game->values[game->x2_buf * (i + 1) + game->x2], &temp);
            addresses.push_back(temp);
            typelist.push_back(MPI_C_BOOL); // data type is bool
        }
        
        // create the MPI type and call it this->mpi_send
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_send);
        // commit the type
        MPI_Type_commit(&this->mpi_send);

        // clear the vectors to reuse them for receive
        block_length.clear();
        addresses.clear();
        typelist.clear();

        // changing the ranges form inner coord-system without buffer
        // to outer coord system including buffer layer
        if ((beg != 0 && !first) || (beg == 0 && !first && one_smaller_beg))
            beg += 2;
        else if (beg == 0 && !first && !one_smaller_beg)
            beg += 1;
        if (end == (game->x1 - 1) && last)
            end += 2;
        else if (end == (game->x1 - 1) && !last && one_smaller_end)
            end += 1;

        for (int i = beg; i <= end; i++)
        {
            // add a one to the blocklength for every element added
            block_length.push_back(1);
            MPI_Aint temp;
            // get the address of the right buffer of local domain
            // this is where received data will be stored
            MPI_Get_address(&game->values[game->x2_buf * i + (game->x2 + 1)], &temp);
            addresses.push_back(temp);
            typelist.push_back(MPI_C_BOOL); // data type is bool
        }
            
        // create the MPI type and call it this->mpi_recv
        MPI_Type_create_struct(block_length.size(), &block_length[0],\
                                &addresses[0], &typelist[0], &this->mpi_recv);
        // commit the type
        MPI_Type_commit(&this->mpi_recv);
    }
    
}

// destrcutor
commTyp::~commTyp()
{}
