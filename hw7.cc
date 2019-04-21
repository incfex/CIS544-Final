#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex              mtx;
condition_variable cv;
bool               ready = false;

void PartWorker(int a) {}

void ProdWorker(int a) {}

int main() {
    const int m = 20, n = 20;
    thread    partW[m];
    thread    prodW[n];
    for (int i = 0; i < n; i++) {
        partW[i] = thread(PartWorker, i);
        prodW[i] = thread(ProdWorker, i);
    }
    for (int i = n; i < m; i++) { partW[i] = thread(PartWorker, i); }

    for (int i = 0; i < n; i++) {
        partW[i].join();
        prodW[i].join();
    }
    for (int i = n; i < m; i++) { partW[i].join(); }

    cout << "Finish!" << endl;

    return 0;
}