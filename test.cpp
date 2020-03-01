#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "GameOfLife.h"
#include "GameOfLife.cpp"

using namespace std;


// void getBoundaryConditions(int* &rows, int* &cols, bool periodic)
// {
//     string file_name = "data/arrangement.dat";
//     cout << "Boundary conditions from: ";
//     cout << file_name << endl;
//     ifstream infile(file_name);   
// }

void readFile(bool* &values, int iteration, int rows, int cols)
{
    string file_name;

    file_name = "figures/out" + to_string(iteration) + ".dat";

    cout << "iter: " << iteration << ": " << file_name << endl;

    ifstream infile(file_name);

    string line;

    int cnt = 0;

    if (infile.is_open())
    {
        while (getline(infile, line))
        {
            istringstream iss(line);

            vector<string> tokens;
            
            copy(istream_iterator<string>(iss),
                istream_iterator<string>(),
                back_inserter(tokens));

            for (int i = 0; i < cols; i++)
            {
                istringstream(tokens[i]) >> values[cnt + i];
            }
            cnt += cols;
        }
    }
    else
    {
        cout << "FILE DOESN'T EXIST" << endl;
        assert(infile.is_open());
    }
    
}

void allclose(bool* &seriel_vals, bool* &parallel_vals, int rows, int cols)
{
    bool same = true;
    for (int i = 1; i < rows+1; i++)
        for (int j = 1; j < cols+1; j++)
            {
                //cout << seriel_vals[i * (cols+2) + j] << " " << parallel_vals[(i-1) * cols + (j-1)] << endl;
                if (seriel_vals[i * (cols+2) + j] != parallel_vals[(i-1) * cols + (j-1)])
                    same = false;
            }
    
    cout << boolalpha;
    cout << same << endl;
}


int main()
{
    int rows = 100;
    int cols = 100;
    int iterations = 10;
    bool periodic = true;
    
    // getBoundaryConditions(rows, cols, periodic);

    bool* parallel_vals = new bool[cols * rows];

    GameOfLife* test_game = new GameOfLife(0, rows, cols, periodic, periodic);
    
    cout << "initial condition from: " << endl;
    readFile(parallel_vals, 0, rows, cols);
    cout << endl;

    for (int i = 1; i < (test_game->x1 + 1); i++)
        for (int j = 1; j < (test_game->x2 + 1); j++)
            test_game->values[i * test_game->x2_buf + j] = parallel_vals[(i-1) * rows + (j-1)];

    test_game->getBuffer();
        //test_game->printGame(false);



    for (int i = 1; i < iterations; i++)
    {
        readFile(parallel_vals, i, rows, cols);
        
        test_game->solveOnce();

        allclose(test_game->values, parallel_vals, rows, cols);
        //test_game->printGame(false);
        //cout << endl;
    }



    delete test_game;
    delete[] parallel_vals;
}
