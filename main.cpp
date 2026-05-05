#include <stdio.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <ctime>
#include <chrono>
#include <map>
#include "receiver2.h"
#include "shard.h"
using namespace std;

enum state {IDLE, RUN};
static int current_session = 0;
static FILE* current_file = NULL;
static state current_state = IDLE;
static map<string, vector<long long>> ID_map;
static bool DEBUG = false;
static bool OVERWRITE = false;

extern "C"{
    #include "fake_receiver.h"
}

void init_session_counter() {
    while (true) {
        string f = "session" + to_string(current_session) + ".txt";
        FILE* test = fopen(f.c_str(), "r");
        if (test == NULL) break;  // file nonexistent, session marker found
        fclose(test);
        current_session++;
    }
}

void open_new(const char* session_name){
    string f = string(session_name) + to_string(current_session) + ".txt";
    current_file = fopen(f.c_str(), "w");
    if (current_file == NULL) {
        cerr << "Failed to open file: " << f << endl;
        return;
    }
}

void gen_csv();

void print_id_map() {
    cout << "Current ID Map:" << endl;
    for (const auto& pair : ID_map) {
        cout << "ID: " << pair.first << ", Times: ";
        for (long long time : pair.second) {
            cout << time << " ";
        }
        cout << endl;
    }
}

void close_current(){
    if (current_file != NULL) {
        fclose(current_file);
        current_file = NULL;
    }
    if (!ID_map.empty()) {
        gen_csv();
        if (DEBUG) print_id_map();
        ID_map.clear();
    }
}

void msg_to_file(const char* message, const char* id); // apparently I need to declare stuff before I use it, who knew

void parse_message(const char* message){
    cout << "Parsing message: " << message << endl;
    // <ID>#<DATA>

    // buffer because strtok doesn't like string literals
    char buffer[21]; // MAX_CAN_MESSAGE_SIZE + 1 for null terminator
    strncpy(buffer, message, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    char* token = strtok(buffer, "#");
    char* id = token;
    token = strtok(NULL, "#");
    char* data = token;
    cout << "ID: " << id << ", Data: " << data << endl;
    if (strcmp(id, "0A0") == 0) { // godawful nested if statement
        if ((strcmp(data, "6601") == 0 || strcmp(data, "FF01") == 0) && current_state == IDLE) {
            open_new("session");
            current_session++;
            current_state = RUN;
        } else if (strcmp(data, "66FF") == 0 && current_state == RUN) {
            close_current();
            current_state = IDLE;
        }
    } 
    if (current_state == RUN) {
        msg_to_file(message, id); // actually write to file
    }
}

void msg_to_file(const char* message, const char* id){
    cout << "Saving message to file: " << message << endl;
    if (current_file == NULL) {
        cerr << "Current file is NULL." << endl;
        return;
    }
    auto now = chrono::system_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

    time_t seconds = ms / 1000;
    int milliseconds = ms % 1000;
    struct tm* timeinfo = localtime(&seconds);
    fprintf(current_file, "(%04d-%02d-%02d %02d:%02d:%02d.%03d) %s\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, milliseconds, message);
    ID_map[string(id)].push_back((long long)ms);
}

// wanted to multithread this one, but the docs only mention 2 threads
void gen_csv() {
    string f = "output" + to_string(current_session) + ".csv";
    FILE* csvfile = fopen(f.c_str(), "w");
    if (csvfile == NULL) {
        cerr << "Failed to open CSV file." << endl;
        return;
    }
    fprintf(csvfile, "ID,number_of_messages,mean_time\n");
    for (const auto& pair : ID_map) {
        const string& id = pair.first;
        const vector<long long>& times = pair.second;
        fprintf(csvfile, "%s,%d,", id.c_str(), times.size());

        if (times.size() < 2) {
            fprintf(csvfile, "%s\n", "N/A");
            continue;
        }

        long long t_delta = 0;
        for (size_t i = 1; i < times.size(); i++) {
            t_delta += times[i] - times[i - 1];
        }
        double mean_delta = static_cast<double>(t_delta) / (times.size() - 1);
        fprintf(csvfile, "%.2f\n", mean_delta);
    }
    fclose(csvfile);
}

void arg_check(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        const char* args = argv[i];
        if (strcmp(args, "--debug") == 0) {
            DEBUG = true;
        }
        if (strcmp(args, "--overwrite") == 0) {
            OVERWRITE = true;
        }
        if (strcmp(args, "-f") == 0 && i + 1 < argc) {
            can_file = argv[i + 1];
            i++;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) arg_check(argc, argv);
    if (!OVERWRITE) init_session_counter();
    
    cout << "Welcome to Project 2" << endl;

    thread receiver_thread(receiver_thread_func);

    while (running) {
        string msg;
        
        {
            lock_guard<mutex> lock(q_mutex);
            if (!msg_q.empty()) {
                msg = msg_q.front();
                msg_q.pop();
            }
        }

        if (!msg.empty()) {
            parse_message(msg.c_str());   // process it
        }
    }

    receiver_thread.join();
    return 0;
}



