#include <gtest/gtest.h>
#include "messagebroker.h"
#include "gre/sbio_wrapper.h"

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
  sbioEvent_t event;
  bool eventAvailable = broker.getNextEvent(&event);
  EXPECT_TRUE(eventAvailable);
  EXPECT_EQ(event.name, "TestEvent");
  EXPECT_EQ(event.data, "TestData");
  sbio_destroy_channel(handle);
}