#pragma once
#include <queue>
#include <string>
#include <mutex>
#include <atomic>

extern std::queue<std::string> msg_q;
extern std::mutex q_mutex;
extern std::atomic<bool> running;