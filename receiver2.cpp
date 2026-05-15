#include <stdio.h>
#include <iostream>
#include "receiver2.h"
#include "shard.h"
using namespace std;

extern "C"{
    #include "fake_receiver.h"
}

void receiver_thread_func() {
    int rst1 = open_can(can_file.c_str());
    if (rst1 == 0) {
        cout << "CAN interface opened successfully." << endl;
    } else if (rst1 == -1) {
        cerr << "Failed to open CAN interface." << endl;
        running = false;
    }

    while (running) {
        char msg[21]; // MAX_CAN_MESSAGE_SIZE + 1 for null terminator
        int rst = can_receive(msg);
        
        if (rst < 0) {
            running = false;
            break;
        }

        lock_guard<mutex> lock(q_mutex);
        msg_q.push(string(msg));
    }
        
    close_can();
}
