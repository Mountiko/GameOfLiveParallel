#include <iostream>
#include <mpi.h>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <ctime>
#include "GameOfLife.h"
#include "commTyp.h"
#include "parallel.h"

using namespace std;


// set dimensions of global domain
int x1 = 100;
int x2 = 100;
// is it a periodic domain or not?
bool periodic = true;
// choose number of timesteps
int time_stps = 100;



// initiate variables 
int id, p; // processors id and total amount of processors p
int tag_num; // tag number



int main(int argc, char *argv[])
{   
    clock_t start = clock();

    // initiate MPI environment
    MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 1000);

    // split global domain
    // and distribute to processors
    vector<int> order = distribute(id, p, x1, x2);

    assert(order[0] != 1 && "Please transpose your domain or shift it by 90° !!!");

    // initiate variable for local domain of each processor
    int x1_local;
    int x2_local;
    // assign dimensions of local domain of each 
    // individual processors 
    getLocalDomain(id, p, x1, x2, order, &x1_local, &x2_local);
    
    // vertical and horizontal periodicity for local domain
    bool v_periodic = false;
    bool h_periodic = false;

    // compute vertical neighbours
    pair<int, int> v_neighbours = getVerticalNeighbours(id, p, order, periodic);
    
    // set v_periodic if only one row of procs
    if (order[id % order.size()] == 1/*id == v_neighbours.first*/)
        v_periodic = true;

    // compute horizontal neighbours
    pair<vector<int>, vector<int>> h_neighbours =\
    getHorizontalNeighbours(id, p, order, periodic);

    // set h_periodic if only one column of procs
    if (/*id == h_neighbours.first[1]*/order.size() == 1)
        h_periodic = true;

    // pointer to store values of all local x1 dimensions
    int* all_x1_local = new int[p];
    // gather all local x1 dimension values from each proc
    MPI_Gather(&x1_local, 1, MPI_INT, all_x1_local, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // allocate memory to store global x1 positions of all procs
    int* x1_pos = new int[p];

    // get global x1 positions of all procs
    if (id == 0)
        getPosition(p, order, all_x1_local, x1_pos);
    // delete unnecessary memory
    delete[] all_x1_local;

    // broadcast x1 positions to all procs
    MPI_Bcast(&x1_pos[0], p, MPI_INT, 0, MPI_COMM_WORLD);
    
    
    // GET THINGY WORKING WHERE ITSELF IS NEIGHBOUR

    
    
    
    //cout << id << v_periodic << h_periodic << endl;
    
    
    
    // intiate GameOfLife instance
    auto* firstGame = new GameOfLife(id, x1_local, x2_local, v_periodic, h_periodic);

    // get the offsets to all horizontal neighbours
    pair<vector<int>, vector<int>> offsets;
    offsets = getOffsets(id, h_neighbours, x1_local, x1, x1_pos);

    // store all necessary communications in one vector
    vector<commTyp> comms = getComm(firstGame, v_neighbours, h_neighbours, offsets, v_periodic, h_periodic, order);
    
    // clock_t init_time = clock();
    // double init_t = ((double) (init_time - start)) / CLOCKS_PER_SEC;
    // cout << "Processor " << id << " initiation time: " << init_t << " sec" << endl;
    

    // print global data necessary for postprocessing to a file
    if (id == 0)
    {   
        // remove all previous files
        system("exec rm -r ./data/*");
        system("exec rm -r ./figures/*");
        
        stringstream filename;
        filename << "./data/arrangement.dat";
        fstream f;
        f.open(filename.str().c_str(), ios_base::out);
        f << x1 << " ";
        f << x2 << endl;
        f << periodic << endl;
        for (int i = 0; i < order.size(); i++)
            f << order[i] << " ";
    }

    
    bool write_buffer = false;
    // make initial communications 
    // and write initial conditions to a file
    sendBounds(comms);
    firstGame->writeToFile(write_buffer, 0);


    // clock_t comm_time = clock();
    // double comm_t = ((double) (comm_time - init_time)) / CLOCKS_PER_SEC;
    // clock_t comm_time2;

    // run gameof life
    for (int i = 1; i < time_stps; i++)
    {
        // solve for every timestep
        firstGame->solveOnce();
        // exchange new boundaries

        //comm_time = clock();
        sendBounds(comms);
        // comm_time2 = clock();        
        // comm_t += ((double) (comm_time2 - comm_time)) / CLOCKS_PER_SEC;

        // write current stat of game to a file
        firstGame->writeToFile(write_buffer, i);
    }

    // delete all previously allcated memory
    delete firstGame;
    delete[] x1_pos;
        
    // cout << "Processor " << id << " communication time: " << comm_t << " sec" << endl;
    
    clock_t finish = clock();
    double t = ((double) (finish - start)) / CLOCKS_PER_SEC;
    cout << "Processor " << id << ": " << t << " sec" << endl;
    


    MPI_Finalize();

    // run postprocessor
    if (id == 0)
    {
        cerr << "Post processing..." << endl;
        system("exec python post_proc.py ");
        cerr << "finalized!" << endl;
    }


}
    
    
