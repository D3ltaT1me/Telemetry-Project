#include "shard.h"

std::queue<std::string> msg_q;
std::mutex q_mutex;
std::atomic<bool> running = true;
std::string can_file;