#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

mutex              mtx;
condition_variable cv;
bool               ready = false;

int buf[4] = {0, 0, 0, 0};

struct parts {
    int  type[4] = {0, 0, 0, 0};
    void put() {
        if (type[0] > 0) {
            if (buf[0] < 5) {
                type[0]--;
                buf[0]++;
            }
        }
        if (type[1] > 0) {
            if (buf[1] < 5) {
                type[1]--;
                buf[1]++;
            }
        }
        if (type[2] > 0) {
            if (buf[2] < 4) {
                type[2]--;
                buf[2]++;
            }
        }
        if (type[3] > 0) {
            if (buf[3] < 3) {
                type[3]--;
                buf[3]++;
            }
        }
    }
    void take() {
        if (type[0] > 0) {
            if (buf[0] > 0) {
                type[0]--;
                buf[0]--;
            }
        }
        if (type[1] > 0) {
            if (buf[1] > 0) {
                type[1]--;
                buf[1]--;
            }
        }
        if (type[2] > 0) {
            if (buf[2] > 0) {
                type[2]--;
                buf[2]--;
            }
        }
        if (type[3] > 0) {
            if (buf[3] > 0) {
                type[3]--;
                buf[3]--;
            }
        }
    }
};

bool checkTypes(parts p) {
    int types = 0;
    for (int i = 0; i < 4; i++) {
        if (p.type[i] > 0) types++;
    }
    if (types < 2)
        return true;
    else
        return false;
}

parts genParts() {
    parts myParts;
    for (int i = 0; i < 4; i++) {
        if (checkTypes(myParts)) {
            switch (rand() % 4) {
                case 0: myParts.type[0]++; break;
                case 1: myParts.type[1]++; break;
                case 2: myParts.type[2]++; break;
                case 3: myParts.type[3]++; break;
                default: break;
            }
        } else {
            if (rand() % 2) {
                if (myParts.type[0] > 0)
                    myParts.type[0]++;
                else if (myParts.type[1] > 0)
                    myParts.type[1]++;
                else
                    myParts.type[2]++;
            } else {
                if (myParts.type[3] > 0)
                    myParts.type[3]++;
                else if (myParts.type[2] > 0)
                    myParts.type[2]++;
                else
                    myParts.type[1]++;
            }
        }
    }
    return myParts;
}

parts takParts() {
    parts mp;
    int   ran = rand() % 4;
    mp.type[(ran + 1) % 4]++;
    mp.type[(ran + 2) % 4]++;
    mp.type[(ran + 3) % 4]++;

    ran = rand() % 4;
    if (mp.type[ran])
        mp.type[ran]++;
    else
        mp.type[(ran + 1) % 4]++;

    return mp;
}

void PartWorker(int a) {
    srand((unsigned)time(0) - a);
    int                it = 0;
    parts              mp = genParts();
    unique_lock<mutex> ulck(mtx);

    printf("Part Worker ID: %d\nIteration: %d\n", a, it);
    printf("Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1], buf[2], buf[3]);
    printf("Place Request: (%d,%d,%d,%d)\n", mp.type[0], mp.type[1], mp.type[2],
           mp.type[3]);

    while (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] != 0) {
        mp.take();
        if (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] != 0)
            cv.wait(ulck);
    }

    cv.notify_one();

    printf("Updated Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1], buf[2],
           buf[3]);
    printf("Updated Place Request: (%d,%d,%d,%d)\n", mp.type[0], mp.type[1],
           mp.type[2], mp.type[3]);
}

void ProdWorker(int a) {
    srand((unsigned)time(0) - a);
    int                it = 0;
    parts              mp = takParts();
    unique_lock<mutex> ulck(mtx);

    printf("Product Worker ID: %d\nIteration: %d\n", a, it);
    printf("Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1], buf[2], buf[3]);
    printf("Pickup Request: (%d,%d,%d,%d)\n", mp.type[0], mp.type[1],
           mp.type[2], mp.type[3]);

    while (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] != 0) {
        mp.take();
        if (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] != 0)
            cv.wait(ulck);
    }

    printf("Updated Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1], buf[2],
           buf[3]);
    printf("Updated Pickup Request: (%d,%d,%d,%d)\n", mp.type[0], mp.type[1],
           mp.type[2], mp.type[3]);
}

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