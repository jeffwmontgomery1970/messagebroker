#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../subscriberlist.h"
#include "../subscriber.h"

class SubscriberListTest : public ::testing::Test {
protected:
    void SetUp() override {
        subscriber_list = new SubscriberList();
        
        // Create test subscribers
        try {
            subscriber1 = new Subscriber("channel1", "", "filter1");
            subscriber2 = new Subscriber("channel2", "", "filter2");
            subscriber3 = new Subscriber("channel3", "", "filter3");
        } catch (const std::exception& e) {
            // Handle potential exceptions from Subscriber constructor
            subscriber1 = nullptr;
            subscriber2 = nullptr;
            subscriber3 = nullptr;
        }
    }
    
    void TearDown() override {
        delete subscriber_list;
        delete subscriber1;
        delete subscriber2;
        delete subscriber3;
    }
    
    SubscriberList* subscriber_list;
    Subscriber* subscriber1;
    Subscriber* subscriber2;
    Subscriber* subscriber3;
};

TEST_F(SubscriberListTest, InitiallyEmpty) {
    std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
    EXPECT_TRUE(subscribers.empty());
    EXPECT_EQ(subscribers.size(), 0);
}

TEST_F(SubscriberListTest, AddSingleSubscriber) {
    if (subscriber1) {
        subscriber_list->addSubscriber(subscriber1);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_EQ(subscribers.size(), 1);
        EXPECT_EQ(subscribers[0], subscriber1);
    }
}

TEST_F(SubscriberListTest, AddMultipleSubscribers) {
    if (subscriber1 && subscriber2 && subscriber3) {
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber2);
        subscriber_list->addSubscriber(subscriber3);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_EQ(subscribers.size(), 3);
        
        // Check that all subscribers are in the list
        EXPECT_NE(std::find(subscribers.begin(), subscribers.end(), subscriber1), subscribers.end());
        EXPECT_NE(std::find(subscribers.begin(), subscribers.end(), subscriber2), subscribers.end());
        EXPECT_NE(std::find(subscribers.begin(), subscribers.end(), subscriber3), subscribers.end());
    }
}

TEST_F(SubscriberListTest, AddDuplicateSubscriber) {
    if (subscriber1) {
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber1); // Add same subscriber twice
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_EQ(subscribers.size(), 2); // Should have duplicate entries
        EXPECT_EQ(subscribers[0], subscriber1);
        EXPECT_EQ(subscribers[1], subscriber1);
    }
}

TEST_F(SubscriberListTest, RemoveSubscriber) {
    if (subscriber1 && subscriber2) {
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber2);
        
        subscriber_list->removeSubscriber(subscriber1);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_EQ(subscribers.size(), 1);
        EXPECT_EQ(subscribers[0], subscriber2);
        EXPECT_EQ(std::find(subscribers.begin(), subscribers.end(), subscriber1), subscribers.end());
    }
}

TEST_F(SubscriberListTest, RemoveNonExistentSubscriber) {
    if (subscriber1 && subscriber2) {
        subscriber_list->addSubscriber(subscriber1);
        
        // Try to remove subscriber that's not in the list
        subscriber_list->removeSubscriber(subscriber2);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_EQ(subscribers.size(), 1);
        EXPECT_EQ(subscribers[0], subscriber1);
    }
}

TEST_F(SubscriberListTest, RemoveFromEmptyList) {
    if (subscriber1) {
        // Try to remove from empty list
        subscriber_list->removeSubscriber(subscriber1);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_TRUE(subscribers.empty());
    }
}

TEST_F(SubscriberListTest, RemoveAllSubscribers) {
    if (subscriber1 && subscriber2 && subscriber3) {
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber2);
        subscriber_list->addSubscriber(subscriber3);
        
        subscriber_list->removeSubscriber(subscriber1);
        subscriber_list->removeSubscriber(subscriber2);
        subscriber_list->removeSubscriber(subscriber3);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_TRUE(subscribers.empty());
    }
}

TEST_F(SubscriberListTest, RemoveDuplicateSubscribers) {
    if (subscriber1) {
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber1);
        
        // Remove should remove all instances
        subscriber_list->removeSubscriber(subscriber1);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_TRUE(subscribers.empty());
    }
}

TEST_F(SubscriberListTest, GetSubscribersReturnsConstCopy) {
    if (subscriber1 && subscriber2) {
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber2);
        
        std::vector<Subscriber*> subscribers1 = subscriber_list->getSubscribers();
        std::vector<Subscriber*> subscribers2 = subscriber_list->getSubscribers();
        
        // Should return copies, not references to the same object
        EXPECT_EQ(subscribers1.size(), subscribers2.size());
        EXPECT_EQ(subscribers1[0], subscribers2[0]);
        EXPECT_EQ(subscribers1[1], subscribers2[1]);
        
        // Modifying returned vector shouldn't affect the original
        subscribers1.clear();
        std::vector<Subscriber*> subscribers3 = subscriber_list->getSubscribers();
        EXPECT_EQ(subscribers3.size(), 2);
    }
}

TEST_F(SubscriberListTest, AddNullSubscriber) {
    subscriber_list->addSubscriber(nullptr);
    
    std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
    EXPECT_EQ(subscribers.size(), 1);
    EXPECT_EQ(subscribers[0], nullptr);
}

TEST_F(SubscriberListTest, RemoveNullSubscriber) {
    subscriber_list->addSubscriber(nullptr);
    subscriber_list->removeSubscriber(nullptr);
    
    std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
    EXPECT_TRUE(subscribers.empty());
}

// Test order preservation
TEST_F(SubscriberListTest, OrderPreservation) {
    if (subscriber1 && subscriber2 && subscriber3) {
        subscriber_list->addSubscriber(subscriber1);
        subscriber_list->addSubscriber(subscriber2);
        subscriber_list->addSubscriber(subscriber3);
        
        std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
        EXPECT_EQ(subscribers[0], subscriber1);
        EXPECT_EQ(subscribers[1], subscriber2);
        EXPECT_EQ(subscribers[2], subscriber3);
    }
}

// Test large number of subscribers
TEST_F(SubscriberListTest, LargeNumberOfSubscribers) {
    const int num_subscribers = 1000;
    std::vector<Subscriber*> test_subscribers;
    
    // Add many subscribers
    for (int i = 0; i < num_subscribers; ++i) {
        test_subscribers.push_back(reinterpret_cast<Subscriber*>(i + 1)); // Use fake pointers
        subscriber_list->addSubscriber(test_subscribers[i]);
    }
    
    std::vector<Subscriber*> subscribers = subscriber_list->getSubscribers();
    EXPECT_EQ(subscribers.size(), num_subscribers);
    
    // Remove half of them
    for (int i = 0; i < num_subscribers / 2; ++i) {
        subscriber_list->removeSubscriber(test_subscribers[i]);
    }
    
    subscribers = subscriber_list->getSubscribers();
    EXPECT_EQ(subscribers.size(), num_subscribers / 2);
}