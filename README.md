# acse-6-individual-assignment-acse-ncv19
acse-6-individual-assignment-acse-ncv19 created by GitHub Classroom

# Conway's Game of Life

This software will run Conaway's Game of Life with random initial values. 
This software is heavily parallelised. Any arbitrary number of processors can be used to run 
the Game of Live in periodic and non periodic mode, appart one special case that will be explained below. 
The special case of running with two processors has not be implemented yet.

### Installation guide

1. Clone this git repository to your local drive:

```
git clone https://github.com/acse-2019/acse-5-assignment-bad_bois
```

2. In your terminal navigate to the folder containing the git repository.

3. Compile the repository with this command:

Before compiling open main.cpp. At the very top you can specify the dimensions of your global domain (x1, x2),
the periodicity and the number of iterations that you would like the Game of Life to run for.

```
mpicxx -o par.out GameOfLife.cpp commTyp.cpp parallel.cpp main.cpp -std=c++11
```

4. To run the game of live use this command:

```
mpirun -n 4 par.out 
```
Where the integer after the -n flag specifies the number of processors you would like to use.


### Postprocessing

The post processing script will run automatically when running the game of life.
You will find a folder "figures" that contains the images of all timesteps computed and the raw data in 
files called "out*.dat".
The "data" folder contains all the raw data of each individual processor. If you wish to print the buffer layer
of each processor you can by switching the bool value of "write_buffer" to true in main.cpp in line 121.
If you do switch this bool no post processing will happen.

### Testing

Purpose of testing parallalised version of the game:
When you run a game with small dimensions and many processors, in a way that a single processors domain may have only
one line or one row (or even none) to compute for, the result is going to be wrong. Please choose a sufficient size 
for the domain to accomodate all processors. As a rule of thumb: your max. amount of processors shouldn't
exceed your smaller global spatial dimension. If you are uncertain you can run this test.

To test whether the parallelised version is computing the same thing as the serial version of this software,
you can run a semi-automated test (test.cpp). This test is semi-automated because you have to specify the 
dimensions of your global domain and the periodicity status. You can define for how many iterations you would
like to test for. The test will test all the iterations specified.
The specifications needed for the boundary conditions can be made in test.cpp in lines:

line 79: int rows (specifies the x1 dimension of the global domain)
line 80: int cols (specifies the x2 dimension of the global domain)
line 81: iterations (for how many iterations the test should check the results)
line 82: bool periodic (whether the tested game is periodic or not)

The test will read all data automatically.

Run the test after making the necessary changes in test.cpp:

1. Compile:
```
g++ test.cpp -std=c++11
```
2. Run:
```
./a.out
```

### Special case

In the special case for which the domain will be split between the processors in one row of multiple processors
the computations will be wrong. That is the case if the ratio x1/x2 becomes very small. I this happens the an assert will pop up asking you to transpose your global domain. This will make it work now. The global domain can also just be shifted by 90°.

### Advantages of this software

+ it can use any arbitrary number if processors
+ the domain distribution is optimised depending on the rate x1/x2
+ the local domains for the individual processors are opzimised to be as squared and of equal sized as possible
+ a processor will never send to itself but rather use a local periodicity (vertically or horizontally) if necessary
+ the domain distribution will ensure a maximum of 8 neighbours per processors, or less in uneven domains (6 or 7 neighbours)
+ the previous two advantages minimise the number of communications between processors

### disatvantages

- the test is only semi-automated
- need for update of special case described above
- the post processing script is in python and can take quite a while when processing large amounts of iterations


### Domain decomposition

The domain is decomposed according to the rate of the global dimensions x1/x2. The algorithm divides all numbers between 1 and 
p-1 (p = amount of procs) and devides it. Then it divides its result by p and takes the rate of both results. The rate that is 
closest to x1/x2 will be used to for the rough mapping of the decomposition. The processors that don't fit in this map (for 
example when using a prime number of processors) will be added to the first few columns of processors. When deviding the total 
length of x2 to all columns of processors the once at the beginning will gain slidly more to compensate that they have less 
cells in x1 direction.


### Communication 

Peer to peer communication:
All processors compute most of the information needed for the communication themselves (neighbours, local domain size, 
vertical and horizontal periodicity, etc.). However, the position-coordinates of each processors is computed only by processor 
0. For this processor 0 uses MPI_Gather to gather all individual local domain sizes and bordcasts (MPI_Bcast) an array that 
contains the position coordinates of all processors. After every time iteration all processors exchange boundaries with there 
neighbours.


### Complexity

This analysis is done with a squared domain and may vary with different domain shapes.

[complexity chart](https://github.com/acse-2019/acse-6-individual-assignment-acse-ncv19/blob/master/charts/complexity%20chart.png)

In this chart you will be able to see how the time taken to solve 100 iterations for a 1000x1000 grid varies with increasing
amount of processors. As expected the time taken to compute the Game of Live decreases with increasing number of 
processors and aproximates towards the a horizontal asymptote localted at zero seconds. 
Although, the graph is not monotonically decreasing. For example when using a squared domain (as in the graph above) the 
computation time for 9, 10 and 11 processors barly varies. The computation time for 12, 13 and 14 processors is also pretty
much the same. The computation time only increases when a new row of processors is filled completly.
In other cases such as when comparing the computation times of 20 processors and 24 processors the graph line increases in
value. In other words the computation time for 24 processors is longer than the computation time for 20 processors.
For just a few number of prcessors, such as 24 and 72 the efficiency drops in a punctial fashion.
There are optimal amounts of processors that can be used for certain domain sizes. As analysed in this graph:
for squared domains it is optimal to use an amount of processors that is well devided according to the rate x1/x2 (= 1 for 
square domains). Such amounts would be (4, 6, 8, 9, 12, 16, 20, etc.).
Other, non-squared domains, have different optima that can be computed as follows:

$$x1\/x2 = proc_rows\/proc_cols$$

The abvove condition you will want to meet best and then compute the total amount of processors:

$$total_procs = proc_rows\*proc_cols$$

[split times chart](https://github.com/acse-2019/acse-6-individual-assignment-acse-ncv19/blob/master/charts/split%20times.png)

This chart is basically the same as the previous one with an additional function representing the time that is invested in 
communication. Here it becomes clear that when the where the less efficient uneven domain decomposition originates:
the communication time is much greater in unevenly decomposed domains.
It is very interesting to see that the communication is less efficient in such cases where the domain decomposition is uneven.
It is interesting because in an uneven distribution there are some processors that have less than the usual 8 neighbours. Some 
processors may have only 6 or 7 neighbours. So the relative amount of communications is a actually less. 
However, evertime that the communication time increases, the total computation time won't drop as expected with increasing 
number if procs, but rather stay constant or in rare ocasions even increase. This is because even though the communication is 
less efficient the additional processors speed up the calculation part of the computation

[speed-up chart](https://github.com/acse-2019/acse-6-individual-assignment-acse-ncv19/blob/master/charts/Speed-up.png)

The speed-up chart shows the amount of times that the parallel code is faster than the serial code i.e. when using 120
processors the computation time is 81 times faster. The overall speed-up appears to be increasing linearly, though there are
fluctuations visible due to uneven domain decomposition (i.e. a prime number of processes). However, it seems that the slope 
is slidly decreasing when increasing the number of processes. This is due to the decreasing efficiency with more processors.

[efficiency chart](https://github.com/acse-2019/acse-6-individual-assignment-acse-ncv19/blob/master/charts/efficiency.png)

The efficiency represents the actual speed-up (explained above) devided an ideal speed-up. An ideal speed-up would be a speed-
up directly proportional to the number processors. In other words: when using double the amount of processors, the total 
computation time is halved. Therefore, the efficiency is usually less than one, there are special cases though in which they 
can be greater than one).
This graph also decreases with increasing number of processors, just as expected. This explaines why the slope of the speed-up 
above is slidly decreasing with increasing number of processors. Here, again, are fluctuations visible due to uneven domain 
decomposition. Although, this chart makes it very visible that the fluctuations due to the uneven domain distribution decrease 
with increasing number of processors.

[initiation time chart](https://github.com/acse-2019/acse-6-individual-assignment-acse-ncv19/blob/master/charts/Initiation%20time.png)

The initiation time is the time that this software needs initiate the computations for the Game of Live. This includes 
computations such as the domain decomposition, getting each neighbours, setting up the communications, etc.
Even though, this is a singular time consuming action, it is very quick and the only action that gets more efficient the more 
processors are involved. The rising initiation time efficiency is very small. Here, too, fluctuations are observable.

[increasing cells chart](https://github.com/acse-2019/acse-6-individual-assignment-acse-ncv19/blob/master/Performance%20over%20increasing%20cells.png)

This chart analyses computation time over increasing amount of cells. It is obvious that there is an 
exponential increase of computation time with exponentially increasing the amount of cells. You could also think like:
one cell takes x seconds to be computed. 10 Cells take x\*10 seconds to be computed and 100 x 100 cells take x\*100\^2 seconds
to be computed. Which is actually a linear increase of computation time over cells.

### Author

* **Nikolas Vornehm** - [Github](https://github.com/acse-ncv19)

### Acknowledgments

Thanks to the teaching staff at Imperial College London for their enthusiastic classes and all the Teaching Assistants for their assistance.

### License

This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/acse-2019/acse-6-individual-assignment-acse-ncv19/blob/master/LICENSE) file for details.




