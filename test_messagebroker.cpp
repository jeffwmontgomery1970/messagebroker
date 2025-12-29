#include <gtest/gtest.h>
#include "messagebroker.h"
#include "gre/sbio_wrapper.h"
#include <string>

bool eventReceived = false;
sbioEvent_t receivedEvent;

void receiveEvent(
        const char *eventName, 
        char *format, 
        void *data, 
        int dataSize, 
        void *userData) 
        {
          receivedEvent.eventName = eventName;
          receivedEvent.format = format;
          receivedEvent.data = static_cast<const char *>(data);
          eventReceived = true;
        };

TEST(messagebroker_tests, BrokerCreatesSBIO) {
  Broker broker;

  sbio_channel_handle_t *handle;
  int result = sbio_create_send_channel(BROKER_NAME, 0, &handle);
  EXPECT_EQ(result, 0);
  sbio_destroy_channel(handle);
}

TEST(messagebroker_tests, BrokerRegistersCallback) {
  Broker broker;

  sbio_channel_handle_t *handle;
  int result = sbio_create_send_channel(BROKER_NAME, 0, &handle);
  EXPECT_EQ(result, 0);
  sbio_send_event(handle, "TestEvent", "s0", (void *)"TestData", 9);
  sleep(1); // Allow time for event to be received
  sbioEvent_t event;
  bool eventAvailable = broker.getNextEvent("TestEvent", &event);
  EXPECT_TRUE(eventAvailable);    
  EXPECT_EQ(strcmp(event.eventName.c_str(), "TestEvent"), 0);  
  EXPECT_EQ(strcmp(event.data.c_str(), "TestData"), 0);
  sbio_destroy_channel(handle);
}

TEST(messagebroker_test, BrokerRegistersSubscribers) {
  Broker broker;
  
  string channelName = "TestChannel";

  sbio_channel_handle_t *receiveHandle;

  int result = sbio_create_receive_channel(channelName.c_str(), 0, &receiveHandle);

  EXPECT_EQ(result, 0);

  sbio_channel_handle_t *sendHandle;

  result = sbio_create_send_channel(BROKER_NAME, 0, &sendHandle);

  EXPECT_EQ(result, 0);

  string channelNameFilter = channelName + "TestChannel.*";

  result = sbio_send_event(sendHandle, "sbio_mq.register", "s0:channel_name", (void *)channelNameFilter.c_str(), channelNameFilter.length() + 1);

  EXPECT_EQ(result, 0);

  sbio_destroy_channel(receiveHandle);
  sbio_destroy_channel(sendHandle);
};

TEST(messagebroker_test, BrokerReceivesBroadcasts) {
  Broker broker;
  
  string channelName = "BroadcastChannel";

  sbio_channel_handle_t *receiveHandle;

  int result = sbio_create_receive_channel(channelName.c_str(), 0, &receiveHandle);

  EXPECT_EQ(result, 0);

  sbio_channel_handle_t *sendHandle;

  result = sbio_create_send_channel(BROKER_NAME, 0, &sendHandle);

  EXPECT_EQ(result, 0);

  string channelNameFilter = channelName + ".*";

  result = sbio_send_event(sendHandle, "sbio_mq.register", "s0:channel_name", (void *)channelNameFilter.c_str(), channelNameFilter.length() + 1);

  EXPECT_EQ(result, 0);

  sleep(1); // Allow time for registration to process

  result = sbio_send_event(sendHandle, "BroadcastChannel.TestEvent", "s0", (void *)"BroadcastData", 14);

  EXPECT_EQ(result, 0);

  sleep(1); // Allow time for event to be received

  sbioEvent_t event;
  bool eventAvailable = broker.getNextEvent("BroadcastChannel.TestEvent", &event);
  EXPECT_TRUE(eventAvailable);    
  EXPECT_EQ(strcmp(event.eventName.c_str(), "BroadcastChannel.TestEvent"), 0);  
  EXPECT_EQ(strcmp(event.data.c_str(), "BroadcastData"), 0);

  sbio_destroy_channel(receiveHandle);
  sbio_destroy_channel(sendHandle);
};

TEST(messagebroker_test, BrokerBroadcastEvents) {
  Broker broker;

  string channelName = "TestChannel";
  
  sbio_channel_handle_t *receiveHandle;

  int result = sbio_create_receive_channel(channelName.c_str(), 0, &receiveHandle);

  EXPECT_EQ(result, 0);

  result = sbio_add_event_callback(receiveHandle, NULL, (sbio_event_callback_t)receiveEvent, nullptr);

  EXPECT_EQ(result, 0);

  sbio_channel_handle_t *sendHandle;

  result = sbio_create_send_channel(BROKER_NAME, 0, &sendHandle);

  EXPECT_EQ(result, 0);

  string channelNameFilter = channelName + ":TestChannel.*";

  result = sbio_send_event(sendHandle, "sbio_mq.register", "s0:channel_name", (void *)channelNameFilter.c_str(), channelNameFilter.length() + 1);

  EXPECT_EQ(result, 0);

  sleep(1); // Allow time for registration to process

  result = sbio_send_event(sendHandle, "TestChannel.testevent", "s0:data", (void *)"TestData",9);

  EXPECT_EQ(result, 0);
  
  sleep(10); // Allow time for event to be broadcast
  EXPECT_TRUE(eventReceived);
  EXPECT_EQ(strcmp(receivedEvent.eventName.c_str(),"TestChannel.testevent"),0);
  EXPECT_EQ(strcmp(receivedEvent.data.c_str(),"TestData"),0);
  sbio_rem_event_callback(receiveHandle, (sbio_event_callback_t)receiveEvent, nullptr);
  sbio_destroy_channel(receiveHandle);
  sbio_destroy_channel(sendHandle);
};