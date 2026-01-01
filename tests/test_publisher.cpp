#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../publisher.h"
#include "../subscriber.h"

class PublisherTest : public ::testing::Test {
protected:
    void SetUp() override {
        log_file_path = "/tmp/test_broker.log";
        control_channel = "test_channel";
    }
    
    void TearDown() override {
        std::remove(log_file_path.c_str());
    }
    
    std::string log_file_path;
    std::string control_channel;
};

// Test BrokerMessage class
TEST(BrokerMessageTest, Constructor) {
    BrokerMessage msg("test_event", "test_message");
    EXPECT_EQ(msg.getEventType(), "test_event");
    EXPECT_EQ(msg.getMessage(), "test_message");
}

TEST(BrokerMessageTest, GetEventType) {
    BrokerMessage msg("info", "data");
    EXPECT_EQ(msg.getEventType(), "info");
}

TEST(BrokerMessageTest, GetMessage) {
    BrokerMessage msg("warning", "alert_data");
    EXPECT_EQ(msg.getMessage(), "alert_data");
}

// Test MessageBuffer class
TEST(MessageBufferTest, AddMessage) {
    MessageBuffer buffer;
    BrokerMessage msg("test", "data");
    
    buffer.addMessage(msg);
    EXPECT_EQ(buffer.getMessages().size(), 1);
    EXPECT_EQ(buffer.getMessages()[0].getEventType(), "test");
}

TEST(MessageBufferTest, MaxQueueLength) {
    MessageBuffer buffer;
    
    // Add more than MAX_MESSAGE_QUEUE_LENGTH messages
    for (int i = 0; i < MAX_MESSAGE_QUEUE_LENGTH + 5; i++) {
        BrokerMessage msg("test" + std::to_string(i), "data" + std::to_string(i));
        buffer.addMessage(msg);
    }
    
    EXPECT_EQ(buffer.getMessages().size(), MAX_MESSAGE_QUEUE_LENGTH);
    // First message should be removed, so first in queue should be message 5
    EXPECT_EQ(buffer.getMessages()[0].getEventType(), "test5");
}

// Test Publisher class
TEST_F(PublisherTest, Constructor) {
    EXPECT_NO_THROW({
        Publisher publisher(control_channel, log_file_path);
    });
}

TEST_F(PublisherTest, PublishMessage) {
    Publisher publisher(control_channel, log_file_path);
    
    // Test publishing without subscribers
    EXPECT_NO_THROW({
        publisher.publish("test_event", "test_message");
    });
}

TEST_F(PublisherTest, AddSubscriber) {
    Publisher publisher(control_channel, log_file_path);
    
    // Create a mock subscriber (we'll use nullptr for simplicity in this test)
    // In a real test, you'd create a proper mock
    Subscriber* mock_subscriber = nullptr;
    
    EXPECT_NO_THROW({
        publisher.addSubscriber(mock_subscriber);
    });
}

TEST_F(PublisherTest, RemoveSubscriber) {
    Publisher publisher(control_channel, log_file_path);
    
    Subscriber* mock_subscriber = nullptr;
    publisher.addSubscriber(mock_subscriber);
    
    EXPECT_NO_THROW({
        publisher.removeSubscriber(mock_subscriber);
    });
}

TEST_F(PublisherTest, ReceiveSubscribeMessage) {
    Publisher publisher(control_channel, log_file_path);
    
    EXPECT_NO_THROW({
        publisher.receiveMessage("subscribe", "test_channel:test_filter");
    });
}

TEST_F(PublisherTest, ReceiveUnsubscribeMessage) {
    Publisher publisher(control_channel, log_file_path);
    
    EXPECT_NO_THROW({
        publisher.receiveMessage("unsubscribe", "test_channel:test_filter");
    });
}

TEST_F(PublisherTest, ReceiveRegularMessage) {
    Publisher publisher(control_channel, log_file_path);
    
    EXPECT_NO_THROW({
        publisher.receiveMessage("info", "test_data");
    });
}

TEST_F(PublisherTest, CheckFilterNoFilter) {
    Publisher publisher(control_channel, log_file_path);
    
    class MockSubscriber : public Subscriber {
    public:
        MockSubscriber() : Subscriber("test", "", "") {}
        std::string get_filter() { return ""; }
    };
    
    MockSubscriber mock_sub;
    EXPECT_TRUE(publisher.checkFilter(&mock_sub, "any_event"));
}

// Test pattern matching function (static function testing through public interface)
class PatternMatchTest : public ::testing::Test {
protected:
    Publisher* publisher;
    
    void SetUp() override {
        publisher = new Publisher("test_channel", "/tmp/test.log");
    }
    
    void TearDown() override {
        delete publisher;
        std::remove("/tmp/test.log");
    }
};

TEST_F(PatternMatchTest, ExactMatch) {
    class TestSubscriber : public Subscriber {
    public:
        TestSubscriber(const std::string& filter) : Subscriber("test", "", filter), test_filter(filter) {}
        std::string get_filter() { return test_filter; }
    private:
        std::string test_filter;
    };
    
    TestSubscriber sub("info");
    EXPECT_TRUE(publisher->checkFilter(&sub, "info"));
    EXPECT_FALSE(publisher->checkFilter(&sub, "warning"));
}

TEST_F(PatternMatchTest, WildcardMatch) {
    class TestSubscriber : public Subscriber {
    public:
        TestSubscriber(const std::string& filter) : Subscriber("test", "", filter), test_filter(filter) {}
        std::string get_filter() { return test_filter; }
    private:
        std::string test_filter;
    };
    
    TestSubscriber sub("in*");
    EXPECT_TRUE(publisher->checkFilter(&sub, "info"));
    EXPECT_TRUE(publisher->checkFilter(&sub, "input"));
    EXPECT_FALSE(publisher->checkFilter(&sub, "warning"));
}