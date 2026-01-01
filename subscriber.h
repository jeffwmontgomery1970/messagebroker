#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <fstream>
#include "gre/sbio_wrapper.h"

extern std::mutex global_lock;

class Subscriber {
public:
    Subscriber(const std::string& channel_name, const std::string& controller_name, const std::string& filter);
    ~Subscriber();
    void receive(const std::string& event_type, const std::string& message);
    bool send_message(const std::string& event_type, const std::string& message);
    bool start();
    bool change_filter(const std::string& filter);
    bool unsubscribe(const std::string& filter);
    void stop();
    void run(const std::string& log_file_path);
    std::string get_channel_name();
    std::string get_filter();
    std::string get_next_message();
private:
    volatile bool is_running = true;
    std::thread sub_thread;
    std::mutex m_lock;
    std::queue<std::string> m_queue;
    std::string channel_name;
    std::string controller_channel;
    std::string filter;
    sbio_channel_handle_t *handle;
    sbio_channel_handle_t *controller;
    std::ofstream log_file;
};

#endif