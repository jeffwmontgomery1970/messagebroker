#include "gre/sbio_wrapper.h"
#include <map>
#include <vector>
#include <string>

using namespace std;

#define BROKER_NAME "sbio_mq"

struct sbioEvent_t {
    char eventName[30];
    char format[128];
    char data[1024];
};

class Broker
{
    public:
        Broker();
        ~Broker();

        void receivedEvent();
    private:
        sbio_channel_handle_t *handle;
        map<string,string> *registeredChannels;
        map<string, vector<sbioEvent_t>> *last10Events;
        void registerChannel(char *, char *);
        void unregisterChannel(char *);
        void broadCastEvent(sbioEvent_t);
};