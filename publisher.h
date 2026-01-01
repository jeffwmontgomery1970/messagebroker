#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <fstream>
#include <map>
#include "gre/sbio_wrapper.h"

extern int MAX_MESSAGE_QUEUE_LENGTH;

class BrokerMessage {
public:
    BrokerMessage(const std::string& event_type, const std::string& message);
    const std::string& getEventType();
    const std::string& getMessage();
private:
    std::string event_type;
    std::string message;
};

class MessageBuffer {
public:
    void addMessage(const BrokerMessage& message);
    std::vector<BrokerMessage>& getMessages();
private:
    std::vector<BrokerMessage> messages;
};

class Publisher {
public:
    Publisher(const std::string& control_channel, const std::string& log_file);
    ~Publisher();
    void publish(const std::string& event_type, const std::string& message);
    void addSubscriber(class Subscriber* subscriber);
    void removeSubscriber(class Subscriber* subscriber);
    void receiveMessage(const std::string& event_type, const std::string& message);
    bool checkFilter(class Subscriber *subscriber, const std::string& event_type);
private:
    std::vector<class Subscriber*> subscribers;
    std::mutex mutex;
    sbio_channel_handle_t *handle;
    std::ofstream log_file;
    std::map<std::string,MessageBuffer> messageBuffers;
};

#endif