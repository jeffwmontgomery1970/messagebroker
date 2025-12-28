#include <iostream>
#include <vector>
#include <string>
#include "messagebroker.h"
#include "gre/sbio_wrapper.h"

using namespace std;

void receivedEvent(char *eventName, char *format, void *data, size_t dataSize, void *userData)
{
   Broker *broker = static_cast<Broker *>(userData);
   sbioEvent_t newEvent = {eventName,format,(string)(static_cast<const char*>(data))};
   broker->addReceivedEvent(newEvent);
   
   void Broker::receivedEvent(char *eventName, char *format, void *data, size_t dataSize) {
   if (last10Events->find(eventName) != last10Events->end()) {
      

      last10Events->at(eventName).push_back(newEvent);
      if (last10Events->at(eventName).size() > 10) {
         last10Events->at(eventName).erase(last10Events->at(eventName).begin());
      } else {
         last10Events->at(eventName) = vector<sbioEvent_t>{newEvent};
      }
   }
}
       int result = sbio_create_receive_channel(BROKER_NAME, 0, &handle);
   if (result != 0) 
   {
      cerr << "Failed to create " << BROKER_NAME << " channel" << endl;
      exit(1);
   }
   result = sbio_add_event_callback(handle, ".*", (sbio_event_callback_t)receivedEvent, this);
};

Broker::~Broker()
{
   sbio_destroy_channel(handle);
}

int main()
{
   vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};

   for (const string& word : msg)
   {
      cout << word << " ";
   }
   cout << endl;
};

bool Broker::getNextEvent(string name, sbioEvent_t event)
{
   if (last10Events->find(name) != last10Events->end() && !last10Events->at(name).empty()) 
   {
      event = last10Events->at(name).back();
      return true;
   }
   return false;
}


