#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "GameOfLife.h"

using namespace std;



// constructor
GameOfLife::GameOfLife(int id, int x1, int x2, bool v_periodic, bool h_periodic):\
id(id), x1(x1), x2(x2),cells(x1 * x2), x1_buf(x1 + 2), x2_buf(x2 + 2),\
v_periodic(v_periodic), h_periodic(h_periodic)
{
    // allocate memory for the values pointer
    // enough memory to include the buffer around the domain
    this->values = nullptr; // set all value sot zero
    this->values = new bool[x1_buf * x2_buf];
    
    
    srand(time(NULL) + id * 1000);
    
    // Fill values array with random numbers of 0 and 1.
    // Fill only the actual domain in continuous 1D-array
    for (int i = 1; i <= this->x1; i++)
        for (int j = 1; j <= this->x2; j++)
            {
                this->values[i * this->x2_buf + j] = rand() % 2;
            }

    // fill padding with zeros
    for (int i = 0; i < x2_buf; i++)
    {
        this->values[i] = 0;
        this->values[this->x2_buf * (this->x1 + 1) + i] = 0;
    }

    for (int i = 1; i <= this->x1; i++)
        {
            this->values[this->x2_buf * i] = 0;
            this->values[this->x2_buf * i + this->x2 + 1] = 0;
        }


    // Call function to fill buffer cells if necessary
    this->getBuffer();
}

GameOfLife::~GameOfLife()
{
    // delete allocated memory of values pointer
    delete[] this->values;
}

void GameOfLife::printGame(bool show_buffer)
{
    cout << "ID: " << this->id << endl;
    // print with buffer layer
    if (show_buffer)
    {
        for (int i = 0; i < this->x1_buf; i++)
        {
            for (int j = 0; j < this->x2_buf; j++)
                cout << this->values[i * this->x2_buf + j] << " ";
            cout << endl;
        }
    }
    // print without buffer layer
    else
    {
        for (int i = 1; i <= this->x1; i++)
        {
            for (int j = 1; j <= this->x2; j++)
                cout << this->values[i * this->x2_buf + j] << " ";
            cout << endl;
        }
    }
}

void GameOfLife::writeToFile(bool write_buffer, int time_stp)
{
    stringstream filename;
    filename << "./data/proc_" << this->id << "_" << time_stp << ".dat";
	fstream f;
	f.open(filename.str().c_str(), ios_base::out);

    if (write_buffer)
    {
        for (int i = 0; i < this->x1_buf; i++)
        {
            for (int j = 0; j < this->x2_buf; j++)
                f << this->values[i * this->x2_buf + j] << " ";
            f << endl;
        }
    }
    // print without buffer layer
    else
    {
        for (int i = 1; i <= this->x1; i++)
        {
            for (int j = 1; j <= this->x2; j++)
                f << this->values[i * this->x2_buf + j] << " ";
            f << endl;
        }   
    }
}


void GameOfLife::getBuffer()
{
    // Fill top and bottom buffer if vertically periodic.
    if (this->v_periodic)
    {
        for (int j = 1; j <= this->x2; j++)
        {
            // fill top buffer cells with bottom domain cell values
            this->values[j] = this->values[this->x1 * this->x2_buf + j];
            // fill bottom buffer cells with top domain cell values
            this->values[(this->x1 + 1) * this->x2_buf + j] = this->values[x2_buf + j];
        }
    }

    // Fill right and left buffer if horozontally periodic.
    if (this->h_periodic)
    {
        for (int i = 1; i <= this->x1; i++)
        {
            // fill left buffer cells with right domain cell values
            this->values[i * this->x2_buf] = this->values[i * this->x2_buf + this->x2];
            // fill right buffer cells with left domain cell values
            this->values[(i + 1) * this->x2_buf - 1] = this->values[i * this->x2_buf + 1];
        }
    }

    // Fill corners if both dimensions are periodic
    if (this->v_periodic && this->h_periodic)
    {
        // copy the bottom right cell value into top left buffer cell
        this->values[0] = this->values[this->x1 * this->x2_buf + this->x2];
        // copy the bottom left cell value into top right buffer cell
        this->values[this->x2_buf - 1] =\
        this->values[this->x1 * this->x2_buf + 1];

        // copy the top right cell value into bottom left buffer cell
        this->values[(this->x1_buf - 1) * this->x2_buf] =\
        this->values[this->x2_buf + this->x2];
        // copy the top left cell value into bottom right buffer cell
        this->values[this->x2_buf * this->x1_buf - 1] = this->values[this->x2_buf + 1];   
    }
}

void GameOfLife::solveOnce()
{
    // temporary vector to store the what needs to be changed
    vector<int> temp; 

    // loop only over domain cells (not buffer cells)
    for (int i = 1; i <= this->x1; i++)
        for (int j = 1; j <= this->x2; j++)
        {
            int neighbours = 0; // neighbours count

            // count all neighbours of a cell
            neighbours += this->values[(i - 1) * x2_buf + (j - 1)];
            neighbours += this->values[(i - 1) * x2_buf + j];
            neighbours += this->values[(i - 1) * x2_buf + (j + 1)];
            neighbours += this->values[i * x2_buf + (j - 1)];
            neighbours += this->values[i * x2_buf + (j + 1)];
            neighbours += this->values[(i + 1) * x2_buf + (j - 1)];
            neighbours += this->values[(i + 1) * x2_buf + j];
            neighbours += this->values[(i + 1) * x2_buf + (j + 1)];

            // choose which case is happening (only record changes)
            // and safe it to the temporary vector

            // dead cells comes to life with 3 neighbours
            if (this->values[i * this->x2_buf + j] == 0 && neighbours == 3)
            {   
                temp.push_back(i);
                temp.push_back(j);
                temp.push_back(1);
            }
            // living cell dies of under population
            else if (this->values[i * this->x2_buf + j] == 1 && neighbours < 2)
            {
                temp.push_back(i);
                temp.push_back(j);
                temp.push_back(0);
            }
            // living cell dies of over population
            else if (this->values[i * this->x2_buf + j] == 1 && neighbours >= 4)
            {
                temp.push_back(i);
                temp.push_back(j);
                temp.push_back(0);
            }       
        }
    // add the recorded changes to the values ptr
    for (int n = 0; n < temp.size()/3; n++)
        this->values[temp[n * 3] * x2_buf + temp[n * 3 + 1]] = temp[n * 3 + 2];

    // get the new buffer layer around the domain
    this->getBuffer();
}

/*
void GameOfLife::getTop(int* &top_bound)
{
    for (int i = 0; i < this->x2; i++)
        top_bound[i] = this->values[this->x2_buf + i];
}
void GameOfLife::getBot(int* &bot_bound)
{
    for (int i = 0; i < this->x2; i++)
        bot_bound[i] = this->values[this->x2_buf * this->x1 + i];
}
void GameOfLife::getLeft(int* &left_bound)
{
    for (int i = 0; i < this->x1; i++)
        left_bound[i] = this->values[this->x2_buf * (i + 1)];
}
void GameOfLife::getRight(int* &right_bound)
{
    for (int i = 0; i < this->x1; i++)
        right_bound[i] = this->values[this->x2_buf * (i + 1) + this->x1];
}
*/
