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