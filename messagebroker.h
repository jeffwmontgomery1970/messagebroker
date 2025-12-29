#include "gre/sbio_wrapper.h"
#include <map>
#include <vector>
#include <string>

using namespace std;

#define BROKER_NAME "sbio_mq"

struct sbioEvent_t {
    string eventName;
    string format;
    string data;
};

class Broker
{
    public:
    Broker();
    ~Broker();
    bool getNextEvent(string, sbioEvent_t*);
    void addReceivedEvent(sbioEvent_t);
    private:
        sbio_channel_handle_t *handle;
        map<string,string> *registeredChannels;
        map<string, vector<sbioEvent_t>> *last10Events;
        void registerChannel(const char *, const char *);
        void unregisterChannel(const char *);
        void broadCastEvent(sbioEvent_t);
};
