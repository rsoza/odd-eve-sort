// mpic++ even_odd_mp.cpp -o even_odd_mp
// mpiexec -n # ./even_odd_mp 10000
using namespace std;
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <mpi.h>

void print_array(int arr[], int arr2[], int size, int my_rank)
{
    cout << "Size of array: " << size << " Rank: " << my_rank << " :: my_keys ";
    for (int i = 0; i < size; i++)
    {
        cout << arr[i] << " ";
    }

    cout << endl;
}

// Determine if the process is idle
int compute_partner(int p, int rank, int num_proc)
{
    int partner;
    if (p % 2 == 0)
    { // even phase
        if (rank % 2 != 0)
        {
            partner = rank - 1; // odd rank
        }
        else
        {
            partner = rank + 1; // even rank
        }
    }
    else
    { // odd phase
        if (rank % 2 != 0)
        {
            partner = rank + 1; // odd rank
        }
        else
        {
            partner = rank - 1; // even rank
        }
    }
    return partner;
}

void merge_low(int my_keys[], int recv_keys[], int local_n)
{
    int m_i = 0, r_i = 0, t_i = 0;
    int *temp_keys = new int[local_n];

    while (t_i < local_n)
    {
        if (my_keys[m_i] <= recv_keys[r_i])
        {
            temp_keys[t_i++] = my_keys[m_i++];
        }
        else
        {
            temp_keys[t_i++] = recv_keys[r_i++];
        }
    }
    for (m_i = 0; m_i < local_n; m_i++)
    {
        my_keys[m_i] = temp_keys[m_i];
    }
    delete temp_keys;
}

void merge_high(int my_keys[], int recv_keys[], int local_n)
{
    int m_i = local_n - 1, r_i = local_n - 1, t_i = local_n - 1;
    int *temp_keys = new int[local_n];

    while (t_i >= 0)
    {
        if (m_i >= 0 && (r_i < 0 || my_keys[m_i] >= recv_keys[r_i]))
        {
            temp_keys[t_i--] = my_keys[m_i--];
        }
        else
        {
            temp_keys[t_i--] = recv_keys[r_i--];
        }
    }
    for (int i = 0; i < local_n; i++)
    {
        my_keys[i] = temp_keys[i];
    }
    delete temp_keys;
}

void odd_even_sort(int my_keys[], int my_rank, int n_p, int comm_sz, int counts[], int displacements[])
{
    int phase, i, temp;
    int partner; // the process's partner rank
    MPI::Status status;

    sort(my_keys, my_keys + n_p);
    int max_size = *max_element(counts, counts + comm_sz);
    int *recv_keys = new int[max_size];

    for (phase = 0; phase < comm_sz; phase++)
    {
        partner = compute_partner(phase, my_rank, comm_sz);
        if (partner == -1 || partner == comm_sz)
            continue; // process is idle
        if (partner >= 0 && partner < comm_sz)
        { // not idle
            int p_size = counts[partner];
            // send/receive keys to partner
            MPI::COMM_WORLD.Sendrecv(
                my_keys, n_p, MPI::INT, partner, 0,
                recv_keys, p_size, MPI::INT, partner, 0, status);

            if (my_rank < partner)
            {
                // keep smaller keys
                merge_low(my_keys, recv_keys, n_p);
            }
            else
            {
                // keep larger keys
                merge_high(my_keys, recv_keys, n_p);
            }
        }
    }
    MPI::COMM_WORLD.Barrier();

    delete recv_keys;
}

void fetch_vals(int keys[], int n, string file_name)
{
    ifstream file(file_name);
    int number;
    int index = 0;

    if (file.is_open())
    {
        while (file >> number)
        {
            if (index < n)
            {
                keys[index] = number;
                index++;
            }
            else
            {
                cout << "Error: More numbers than expected in the file!" << endl;
                break;
            }
        }
        file.close();
    }
    else
    {
        cout << "Error opening file!" << endl;
    }
}

bool compare_arrays(int arr[], int size)
{
    int sorted_array[size];
    fetch_vals(sorted_array, size, "check_vals.txt");
    for (int i = 0; i < size; i++)
    {
        if (arr[i] != sorted_array[i])
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    MPI::Init(argc, argv);                    // Initalize MPI environment
    int my_rank = MPI::COMM_WORLD.Get_rank(); // get process identity number
    int comm_sz = MPI::COMM_WORLD.Get_size(); // get total number of processes
    int n = atoi(argv[1]);                    // n number of total keys

    int *keys = nullptr; // Root's data array

    if (my_rank == 0)
    {
        keys = new int[n];
        fetch_vals(keys, n, "random_nums.txt");
    }

    int n_p = n / comm_sz;
    int remainder = n % comm_sz;

    int *counts = new int[comm_sz];
    int *displacements = new int[comm_sz];

    for (int i = 0; i < comm_sz; i++)
    {
        counts[i] = n_p + (i < remainder ? 1 : 0);
        displacements[i] = (i == 0) ? 0 : displacements[i - 1] + counts[i - 1];
    }

    n_p = counts[my_rank];
    int *my_keys = new int[n_p];

    MPI::COMM_WORLD.Scatterv(keys, counts, displacements, MPI::INT, my_keys, n_p, MPI::INT, 0);

    double time = MPI_Wtime();

    odd_even_sort(my_keys, my_rank, n_p, comm_sz, counts, displacements);
    if (my_rank == 0)
    {
        delete keys;
        keys = new int[n];
    }
    MPI::COMM_WORLD.Gatherv(my_keys, n_p, MPI::INT, keys, counts, displacements, MPI::INT, 0);

    time = MPI_Wtime() - time;
    double parallel_time;
    MPI::COMM_WORLD.Reduce(&time, &parallel_time, 1, MPI::DOUBLE, MPI_MAX, 0);

    if (argc == 2)
    {
        if (my_rank == 0)
        {

            cout << "Execution Time = " << parallel_time << " Seconds"
                 << endl;
            string check = (compare_arrays(keys, n) == 0) ? "False" : "True";
            cout << "Is array sorted = " << check << endl;
        }
    }
    else
    {
        if (*argv[1] == 'G')
        {
            cout << "Execution Time = " << " " << parallel_time << endl;
        }
        // print time in CSV format
        else if (*argv[1] == 'C')
        {
            cout << n << "," << parallel_time << endl;
        }
        else if (*argv[1] == 'H')
        {
            // print time and Checksum in normal human readable format
            cout << "Number of active data points =" << n << endl;
            cout << "Execution Time = " << parallel_time << " Seconds"
                 << endl;
        }
    }
    delete counts;
    delete displacements;
    delete keys;

    MPI::Finalize();
    return 0;
}
