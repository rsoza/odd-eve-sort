// g++ odd_even_serial.cpp -o odd_even_serial
// ./odd_even_serial 10000 1 10000
using namespace std;
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/time.h>
#define TIMER_CLEAR  (tv1.tv_sec = tv1.tv_usec = tv2.tv_sec = tv2.tv_usec = 0)
#define TIMER_START     gettimeofday(&tv1, (struct timezone*)0)
#define TIMER_ELAPSED   (double) (tv2.tv_usec- tv1.tv_usec)/1000000.0+(tv2.tv_sec-tv1.tv_sec)
#define TIMER_STOP      gettimeofday(&tv2, (struct timezone*)0)
struct timeval tv1,tv2;


void odd_even_sort(int a[], int n){
	int phase, i, temp;

	for (phase =0; phase<n; phase++){
		if(phase % 2 == 0) { // even phase
			for(i=1;i<n;i+=2) {
				if(a[i-1] > a[i]){
					temp = a[i];
					a[i] = a[i-1];
					a[i-1] = temp;
				} 
            }
        } else {
			for(i=1;i<n-1;i+=2) {
				if(a[i] > a[i+1]){
					temp = a[i];
					a[i] = a[i+1];
					a[i+1] = temp;
				} 
            }
       }
    }
}

void store_values(int* arr, int n, string file) {
    fstream outputFile;
    outputFile.open(file, fstream::out);

    for (int i = 0; i < n; ++i) {
        if(outputFile.is_open()){
            outputFile << arr[i] << " ";
        } else {
            cout << "Error opening text file" << endl;
        }
    }
}

void generateRandomValues(int* arr, int count, int minValue, int maxValue) {

    for (int i = 0; i < count; ++i) {
        arr[i] = minValue + rand() % (maxValue - minValue + 1);
    }
    store_values(arr, count, "random_nums.txt");
}

void print_array(int a[], int n) {
    cout << "Size of array: " << n << " Array Values: ";
    for(int i =0; i<n; i++){
        cout << a[i] << " ";
    }
     cout << endl;
}

void print_usage_instructions(char *command) {
    cout << "Usage: mpiexec " << command << endl <<
        " [n = number of random values] [min range] [max range]" <<
        " <x>" << endl <<
        "   where optional argument x = " << endl <<
        "       H suppress output -- print n,runtime and" << endl <<
        "                            checksum in human readable format" << endl <<
        "       S suppress output -- print checkSum" << endl <<
        "       C suppress output -- print n,runtime" << endl <<
        "                            in CSV format" << endl <<
        "       G suppress output -- print n,runtime" << endl <<
        "                            in gnuplot format" << endl << flush;
}

int main(int argc, char *argv[]){
    if (argc!=4 && argc!=5) {
        print_usage_instructions(argv[0]);
        exit(0);
    }

    int n = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);
    int* a = new int[n];
    generateRandomValues(a, n, min, max);
    
    if (argc == 4){
      cout << "Random values before sorting: " << endl;
      print_array(a, n);
    }
    // begin timer
    TIMER_CLEAR;
    TIMER_START;
    odd_even_sort(a,n);
    // stop timer
    TIMER_STOP;

    if (argc==4) {
        cout << "Random values after sorting: " << endl;
        print_array(a, n);
        cout << "Execution Time = " << TIMER_ELAPSED << " Seconds"
             << endl;
        store_values(a, n, "check_vals.txt");
    }
    else {
        if (*argv[4]=='G') {
            cout << "Execution Time = " << n << " " << TIMER_ELAPSED << endl;
        }
        // print time in CSV format 
        else if (*argv[4]=='C') {
            cout << n << "," << TIMER_ELAPSED << endl;
        }
        else if (*argv[4]=='H') {
           // print time and Checksum in normal human readable format
           cout << "Number of active data points =" << n << endl;
           cout << "Execution Time = " << TIMER_ELAPSED << " Seconds"
                << endl;
        }
    }
}


