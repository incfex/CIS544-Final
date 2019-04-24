#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

mutex              mtx;
condition_variable cv1, cv2;
bool               ready = false;

int partO = 0;
int partI = 0;
int prodO = 0;
int prodI = 0;

int buf[4] = {0, 0, 0, 0};

struct parts {
    int  type[4] = {0, 0, 0, 0};
    void put() {
        while (type[0] > 0 && buf[0] < 5) {
            type[0]--;
            buf[0]++;
        }
        while (type[1] > 0 && buf[1] < 5) {
            type[1]--;
            buf[1]++;
        }
        while (type[2] > 0 && buf[2] < 4) {
            type[2]--;
            buf[2]++;
        }
        while (type[3] > 0 && buf[3] < 3) {
            type[3]--;
            buf[3]++;
        }
    }
    void take() {
        while (type[0] > 0 && buf[0] > 0) {
            type[0]--;
            buf[0]--;
        }
        while (type[1] > 0 && buf[1] > 0) {
            type[1]--;
            buf[1]--;
        }
        while (type[2] > 0 && buf[2] > 0) {
            type[2]--;
            buf[2]--;
        }
        while (type[3] > 0 && buf[3] > 0) {
            type[3]--;
            buf[3]--;
        }
    }
};

bool inorout(bool aorr) {
    if (aorr) {
        if (partO + partI == 0) return false;
        if (rand() % (partO + partI) < partI)
            return true;
        else
            return false;
    }
    if (prodO + prodI == 0) return false;
    if (rand() % (prodO + prodI) < prodI)
        return true;
    else
        return false;
}

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
    int it = 0;
    partO++;
    unique_lock<mutex> ulck(mtx);
    partO--;

    while (it <= 6) {
        this_thread::sleep_for(chrono::milliseconds(rand() % 5 + 2));
        parts mp   = genParts();
        int   bump = 0;
        parts sta  = mp;

        while (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] != 0
               && it <= 6) {
            printf("Part Worker ID: %d\nIteration: %d\n", a, it);
            printf("Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1], buf[2],
                   buf[3]);
            printf("Place Request: (%d,%d,%d,%d)\n", mp.type[0], mp.type[1],
                   mp.type[2], mp.type[3]);

            mp.put();

            printf("Updated Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1],
                   buf[2], buf[3]);
            printf("Updated Place Request: (%d,%d,%d,%d)\n\n", mp.type[0],
                   mp.type[1], mp.type[2], mp.type[3]);

            bool same = true;
            if (bump > 3) {
                bump = 0;
                printf("DeadLock!\n\n");
                break;
            }
            for (int i = 0; i < 4; i++) {
                if (mp.type[i] != sta.type[i]) same = false;
            }
            if (same)
                bump++;
            else {
                sta  = mp;
                bump = 0;
            }

            if (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] == 0) break;

            if (partO + partI == 0) continue;
            if (inorout(true)) {
                cv1.notify_one();
            } else {
                partI++;
                cv1.wait(ulck);
                partI--;
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
        it++;
    }
}

void ProdWorker(int a) {
    srand((unsigned)time(0) - a);
    int it = 0;

    prodO++;
    unique_lock<mutex> ulck(mtx);
    prodO--;

    while (it <= 6) {
        this_thread::sleep_for(chrono::milliseconds(rand() % 5 + 2));
        parts mp   = takParts();
        int   bump = 0;
        parts sta  = mp;

        while (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] != 0
               && it <= 6) {
            printf("Product Worker ID: %d\nIteration: %d\n", a, it);
            printf("Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1], buf[2],
                   buf[3]);
            printf("Pickup Request: (%d,%d,%d,%d)\n", mp.type[0], mp.type[1],
                   mp.type[2], mp.type[3]);

            mp.take();

            printf("Updated Buffer State: (%d,%d,%d,%d)\n", buf[0], buf[1],
                   buf[2], buf[3]);
            printf("Updated Pickup Request: (%d,%d,%d,%d)\n\n", mp.type[0],
                   mp.type[1], mp.type[2], mp.type[3]);

            if (bump > 3) {
                bump = 0;
                printf("DeadLock!\n\n");
                break;
            }
            bool same = true;
            for (int i = 0; i < 4; i++) {
                if (mp.type[i] != sta.type[i]) same = false;
            }
            if (same)
                bump++;
            else {
                sta  = mp;
                bump = 0;
            }

            if (mp.type[0] + mp.type[1] + mp.type[2] + mp.type[3] == 0) break;

            if (prodO + prodI == 0) continue;
            if (inorout(false)) {
                cv2.notify_one();
            } else {
                prodI++;
                cv2.wait(ulck);
                prodI--;
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
        it++;
    }
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