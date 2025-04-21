#include <iostream>
#include <chrono>    
#include <thread> 

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 2) {
        throw runtime_error("number of elements");
    }

    int64_t duration = (*argv[1] - '0');

    this_thread::sleep_for(chrono::seconds(duration));
    cerr << "error message" << endl;
    cout << "sleeping for " << duration << " seconds" << endl;

    this_thread::sleep_for(chrono::seconds(10));
    throw runtime_error("runtime error");
}