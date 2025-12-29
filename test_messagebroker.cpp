#include <gtest/gtest.h>
#include "messagebroker.h"
#include "gre/sbio_wrapper.h"
#include <string>

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

TEST(messagebroker_test, BrokerMaintainsLast10Events) {
  Broker broker;
  
  string channelName = "HistoryChannel";

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

  for (int i = 1; i <= 12; ++i) {
    string eventName = "HistoryChannel.Event" + to_string(i);
    string eventData = "Data" + to_string(i);
    result = sbio_send_event(sendHandle, eventName.c_str(), "s0", (void *)eventData.c_str(), eventData.length() + 1);
    EXPECT_EQ(result, 0);
    sleep(1); // Allow time for event to be received
  }

  for (int i = 3; i <= 12; ++i) {
    string eventName = "HistoryChannel.Event" + to_string(i);
    sbioEvent_t event;
    bool eventAvailable = broker.getNextEvent(eventName, &event);
    EXPECT_TRUE(eventAvailable);    
    EXPECT_EQ(strcmp(event.eventName.c_str(), eventName.c_str()), 0);  
    string expectedData = "Data" + to_string(i);
    EXPECT_EQ(strcmp(event.data.c_str(), expectedData.c_str()), 0);
  }

  sbio_destroy_channel(receiveHandle);
  sbio_destroy_channel(sendHandle);
};

TEST(messagebroker_test, BrokerBroadcastEvents) {
  bool messageReceived = false;
  void receiveEvent(
        const char *eventName, 
        char *format, 
        void *data, 
        int dataSize, 
        void *userData) 
        {
          EXPECT_EQ(strcmp(eventName, "TestChannel.testEvent"), 0);
          EXPECT_EQ(strcmp((char *)data, "TestData"), 0);
          messageReceived = true;
        };

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

  string channelNameFilter = channelName + "TestChannel.*";

  result = sbio_send_event(sendHandle, "sbio_mq.register", "s0:channel_name", (void *)channelNameFilter.c_str(), channelNameFilter.length() + 1);

  EXPECT_EQ(result, 0);

  sleep(1); // Allow time for registration to process

  result = sbio_send_event(sendHandle, "TestChannel.testevent", "s0:data", (void *)"TestData",9);

  EXPECT_EQ(result, 0);
  
  sleep(10); // Allow time for event to be broadcast
  EXPECT_TRUE(messageReceived);
  sbio_rem_event_callback(receiveHandle, (sbio_event_callback_t)receiveEvent, nullptr);
  sbio_destroy_channel(receiveHandle);
  sbio_destroy_channel(sendHandle);
};