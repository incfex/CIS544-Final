#include <iostream>
#include <thread>
using namespace std;

int main() {
    const int m = 20, n = 20;
    thread    partW[m];
    thread    prodW[n];
    for (int i = 0; i < n; i++) {
        partW[i] = thread(PartWorker, i);
        prodW[i] = thread(ProductWorder, i);
    }
    for (int i = 0; i < m; i++) { partW[i] = thread(PartWorker, i); }

    for (int i = 0; i < n; i++) {
        partW[i].join();
        prodW[i].join();
    }

    cout << "Finish!" << endl;

    return 0;
}