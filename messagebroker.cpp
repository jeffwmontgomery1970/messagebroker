#include <iostream>
#include <vector>
#include <string>
#include "messagebroker.h"
#include "gre/sbio_wrapper.h"

using namespace std;

// Callback used by the sbio layer.  Matches the C callback signature and
// translates the incoming data into an sbioEvent_t which we hand to the
// Broker instance passed through userData.
void receivedEvent(char *eventName, char *format, void *data, size_t dataSize, void *userData)
{
   Broker *broker = static_cast<Broker *>(userData);
   std::string eventStr(eventName ? eventName : "");
   std::string fmtStr(format ? format : "");
   std::string dataStr;
   if (data && dataSize > 0) {
      dataStr.assign(static_cast<const char*>(data), dataSize);
   }
   sbioEvent_t newEvent{eventStr, fmtStr, dataStr};

   broker->addReceivedEvent(newEvent);
}

Broker::Broker()
   : handle(nullptr), registeredChannels(new map<string,string>()), last10Events(new map<string, vector<sbioEvent_t>>())
{
   // Create a receive channel and register the callback to receive events.
   int result = sbio_create_receive_channel(BROKER_NAME, 0, &handle);
   if (result != 0) {
      cerr << "Failed to create sbio receive channel" << endl;
      exit(1);
   }
   // Register callback; ignore the return for now.
   result = sbio_add_event_callback(handle,NULL,(sbio_event_callback_t)receivedEvent, this);
   if (result != 0) {
      cerr << "Failed to register sbio event callback" << endl;
      exit(1);
   }
}

Broker::~Broker()
{
   if (handle) sbio_destroy_channel(handle);
   delete registeredChannels;
   delete last10Events;
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

bool Broker::getNextEvent(string name, sbioEvent_t *event)
{
   if (last10Events->find(name) != last10Events->end() && !last10Events->at(name).empty()) 
   {
      *event = last10Events->at(name).back();
      return true;
   }
   return false;
};

void Broker::addReceivedEvent(sbioEvent_t event)
{
   if (event.eventName == "sbio_mq.register") {
      string name, filter;
      if (event.data.find(":") != string::npos) {
         name = event.data.substr(0, event.data.find(":"));
         filter = event.data.substr(event.data.find(":",2)+1);
      }
      else 
      {
         name = event.data;
         filter = "";
      }
      registerChannel(name.c_str(),filter.c_str());
   } else if (event.eventName == "sbio_mq.unregister")
   {
      unregisterChannel(event.data.c_str());
   }
   else
   {
      broadCastEvent(event);
      if (last10Events->find(event.eventName) == last10Events->end()) 
      {
         last10Events->emplace(event.eventName, vector<sbioEvent_t>{});
      }
      last10Events->at(event.eventName).push_back(event);
      if (last10Events->at(event.eventName).size() > 10) 
      {
         last10Events->at(event.eventName).erase(last10Events->at(event.eventName).begin());
      }
   }
};

void Broker::broadCastEvent(sbioEvent_t event)
{
   for (const auto &entry : *registeredChannels) 
   {
      const string &channelName = entry.first;
      const string &filter = entry.second;
      // Simple filter matching: check if event name starts with filter
      if (filter.empty() || event.eventName.find(filter.substr(0, filter.length()-1)) == 0) 
      {
         sbio_channel_handle_t *sendHandle;
         int result = sbio_create_send_channel(channelName.c_str(), 0, &sendHandle);
         if (result != 0) {
            cerr << "Failed to create sbio send channel for broadcasting" << endl;
            return;
         }
         result = sbio_send_event(sendHandle, event.eventName.c_str(), event.format.c_str(), (void *)event.data.c_str(), event.data.length());
         if (result != 0) {
            cerr << "Failed to send broadcast event" << endl;
         }
         sbio_destroy_channel(sendHandle);
      }
   }
};

void Broker::registerChannel(char const *name,char const* filter) 
{
   registeredChannels->insert_or_assign(name,filter);
}

void Broker::unregisterChannel(char const *name)
{
   registeredChannels->erase(name);
}


