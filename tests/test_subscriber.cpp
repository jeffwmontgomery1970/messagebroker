#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../subscriber.h"
#include <thread>
#include <chrono>

class SubscriberTest : public ::testing::Test {
protected:
    void SetUp() override {
        channel_name = "test_subscriber_channel";
        controller_name = "test_controller";
        filter = "test_filter";
        log_file_path = "/tmp/test_subscriber.log";
    }
    
    void TearDown() override {
        std::remove(log_file_path.c_str());
    }
    
    std::string channel_name;
    std::string controller_name;
    std::string filter;
    std::string log_file_path;
};

TEST_F(SubscriberTest, ConstructorWithoutController) {
    EXPECT_NO_THROW({
        Subscriber subscriber(channel_name, "", filter);
    });
}

TEST_F(SubscriberTest, ConstructorWithController) {
    EXPECT_NO_THROW({
        Subscriber subscriber(channel_name, controller_name, filter);
    });
}

TEST_F(SubscriberTest, GetChannelName) {
    Subscriber subscriber(channel_name, controller_name, filter);
    EXPECT_EQ(subscriber.get_channel_name(), channel_name);
}

TEST_F(SubscriberTest, GetFilter) {
    Subscriber subscriber(channel_name, controller_name, filter);
    EXPECT_EQ(subscriber.get_filter(), filter);
}

TEST_F(SubscriberTest, ReceiveMessage) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    // Test receiving a regular message
    subscriber.receive("test_event", "test_data");
    
    std::string message = subscriber.get_next_message();
    EXPECT_EQ(message, "test_event:test_data");
}

TEST_F(SubscriberTest, ReceiveShutdownMessage) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    // Test receiving a shutdown message (without calling stop which requires thread)
    subscriber.receive("shutdown", "");
    
    std::string message = subscriber.get_next_message();
    EXPECT_EQ(message, "Broker has shutdown");
}

TEST_F(SubscriberTest, SendMessage) {
    Subscriber subscriber(channel_name, "", filter);
    
    bool result = subscriber.send_message("test_event", "test_data");
    EXPECT_TRUE(result);
}

TEST_F(SubscriberTest, GetNextMessageEmpty) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    // Test getting message from empty queue
    std::string message = subscriber.get_next_message();
    EXPECT_EQ(message, "");
}

TEST_F(SubscriberTest, GetNextMessageMultiple) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    // Add multiple messages
    subscriber.receive("event1", "data1");
    subscriber.receive("event2", "data2");
    subscriber.receive("event3", "data3");
    
    // Get messages in order
    EXPECT_EQ(subscriber.get_next_message(), "event1:data1");
    EXPECT_EQ(subscriber.get_next_message(), "event2:data2");
    EXPECT_EQ(subscriber.get_next_message(), "event3:data3");
    EXPECT_EQ(subscriber.get_next_message(), ""); // Empty queue
}

TEST_F(SubscriberTest, ChangeFilter) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    bool result = subscriber.change_filter("new_filter");
    EXPECT_TRUE(result);
    EXPECT_EQ(subscriber.get_filter(), "new_filter");
}

TEST_F(SubscriberTest, Unsubscribe) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    bool result = subscriber.unsubscribe(filter);
    EXPECT_TRUE(result);
}

// Test thread safety and concurrent operations
TEST_F(SubscriberTest, ConcurrentReceive) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    // Simulate concurrent message receiving
    std::thread t1([&subscriber]() {
        for (int i = 0; i < 10; ++i) {
            subscriber.receive("event" + std::to_string(i), "data" + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    std::thread t2([&subscriber]() {
        for (int i = 10; i < 20; ++i) {
            subscriber.receive("event" + std::to_string(i), "data" + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    t1.join();
    t2.join();
    
    // Count received messages
    int message_count = 0;
    while (!subscriber.get_next_message().empty()) {
        message_count++;
    }
    
    EXPECT_EQ(message_count, 20);
}

// Test message queue behavior
TEST_F(SubscriberTest, MessageQueueFIFO) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    // Add messages in specific order
    subscriber.receive("first", "1");
    subscriber.receive("second", "2");
    subscriber.receive("third", "3");
    
    // Verify FIFO order
    EXPECT_EQ(subscriber.get_next_message(), "first:1");
    EXPECT_EQ(subscriber.get_next_message(), "second:2");
    EXPECT_EQ(subscriber.get_next_message(), "third:3");
}

// Test edge cases
TEST_F(SubscriberTest, EmptyEventName) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    subscriber.receive("", "data");
    EXPECT_EQ(subscriber.get_next_message(), ":data");
}

TEST_F(SubscriberTest, EmptyEventData) {
    Subscriber subscriber(channel_name, controller_name, filter);
    
    subscriber.receive("event", "");
    EXPECT_EQ(subscriber.get_next_message(), "event:");
}

TEST_F(SubscriberTest, EmptyFilter) {
    Subscriber subscriber(channel_name, controller_name, "");
    EXPECT_EQ(subscriber.get_filter(), "");
}