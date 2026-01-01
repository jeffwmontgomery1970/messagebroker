#include "publisher.h"
#include "subscriber.h"
#include <iostream>
#include <chrono>
#include <random>
 
std::string random_string(std::string::size_type length)
{
 /**
 * Create random alpha-numeric string of length requested
 * 
 * @param length Integer defining the length of the random string to be generated
 * @return String containing the random string value
 */
    static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;

    s.reserve(length);

    while(length--)
        s += chrs[pick(rg)];

    return s;
}

enum actions { publisher, subscriber, test, broadcast };

int main(int argc, char* argv[])
{
/**
 * Main entry point for the MessageBroker application
 * Parses command line arguments and runs in one of four modes:
 * - Publisher: Creates a publisher and waits for messages
 * - Subscriber: Creates a subscriber and listens for messages
 * - Broadcast: Interactive mode to send messages to the broker
 * - Test: Runs automated tests of publisher/subscriber functionality
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return int Exit code (0 for success, non-zero for error)
 */
    actions action = publisher;
    std::string log_file = "/tmp/broker.log";
    std::string control_channel_name = "sbio_mq";
    std::string subscriber_channel_name = random_string(24);
    std::string filter = "";
    if (argc > 1)
    {
        std::string next_arg = "";
        for (int index = 1; index < argc; index++)
        {
            next_arg = std::string(argv[index]);
            if (next_arg.find("Publisher") != std::string::npos)
            {
                action = publisher;
            }
            else if (next_arg.find("Subscriber") != std::string::npos)
            {
                action = subscriber;
            }
            else if (next_arg.find("Test") != std::string::npos)
            {
                action = test;
            }
            else if (next_arg.find("Broadcast") != std::string::npos)
            {
                action = broadcast;
            }
            else if (next_arg.find("-log_file") != std::string::npos)
            {
                if (index < argc - 1)
                {
                    log_file = argv[index + 1];
                    index += 1;
                }
            }
            else if (next_arg.find("-channel_name") != std::string::npos)
            {
                if (index < argc - 1)
                {
                    subscriber_channel_name = argv[index + 1];
                    index += 1;
                }
            }
            else if (next_arg.find("-filter") != std::string::npos)
            {
                if (index < argc - 1)
                {
                    filter = argv[index + 1];
                    index += 1;
                }
            }
        }
    }

    if (action == publisher)
    {
        std::cout << "Running in Publisher mode with Control Channel Name: " << control_channel_name << " and log file name:" << log_file << std::endl;
        Publisher *publisher1 = new Publisher(control_channel_name, log_file);
        while (publisher1 != NULL)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        exit(0);
    }
    else if (action == broadcast)
    {
        std::cout << "Running in Broadcast mode with Control Channel Name: " << control_channel_name << " and log file name:" << log_file << std::endl;
        const char* control_channel_str = control_channel_name.c_str();
        sbio_channel_handle_t *handle;
        int result = sbio_create_send_channel(control_channel_str, GRE_IO_TYPE_WRONLY, &handle);
        if (result != 0)
        {
            std::cout << "Failed to create write channel for: " << control_channel_name << std::endl;
            exit(3);
        }
        std::string event_name = "";
        std::string event_data = "";
        std::cout << "Enter event types and data to broadcast, enter shutdown as the command type to exit" << std::endl;

        while (event_name != "shutdown")
        {
            std::cout << "Please enter the event name:" << std::endl;
            std::cin >> event_name;
            if (event_name != "shutdown")
            {
                std::cout << "Please enter event data:" << std::endl;
                std::cin >> event_data;
            }
            const char* event_name_str = event_name.c_str();
            const char* event_data_str = event_data.c_str();
            int result = sbio_send_event(handle, event_name_str, "1s0 client_channel", (void*) event_data_str, event_data.length() + 1);
            if (result != 0)
            {
                std::cout << "Failed to send message to channel: " << control_channel_name << std::endl;
            }
            else
            {
                std::cout << "Message sent" << std::endl;
            }
        }
        exit(0);
    }
    else if (action == subscriber)
    {
        std::cout << "Running in Subscriber mode with Channel Name: " << subscriber_channel_name << " and log file name:" << log_file << std::endl;
        Subscriber *subscriber1 = new Subscriber(subscriber_channel_name, control_channel_name, filter);
        if (subscriber1 == NULL)
        {
            std::cout << "Failed to create Subscriber with channel name:" << subscriber_channel_name << std::endl;
            exit(4);
        }
        else
        {
            subscriber1->run(log_file);
            std::string message = subscriber1->get_next_message();
            while (message != "shutdown")
            {
                if (message.length() > 0) 
                {
                    std::cout << "Received message from broker: " << message << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                message = subscriber1->get_next_message();
            }
            std::cout << "Message broker has shutdown" << std::endl;
            delete subscriber1;
            exit(0);
        }
    }
    else if (action == test)
    {
        std::cout << "Running in Test mode with Control Channel Name: " << control_channel_name << " and log file name:" << log_file << std::endl;
        
        Publisher *test_publisher = new Publisher(control_channel_name, log_file);

        if (test_publisher != NULL)
        {
            std::cout << "Publisher constructor test passed" << std::endl;
        }
        else
        {
            std::cout << "Publisher constructor test failed" << std::endl;
        }

        Subscriber *test_subscriber = new Subscriber(subscriber_channel_name, control_channel_name, filter);

        if (test_subscriber != NULL)
        {
            std::cout << "Subscriber constructor test passed" << std::endl;
        }
        else
        {
            std::cout << "Subscriber constructor test failed" << std::endl;
            exit(1);
        }

        test_subscriber->run(log_file);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        sbio_channel_handle_t *test_handle;
        const char *channel_name_str = control_channel_name.c_str();
        int result = sbio_create_send_channel(channel_name_str, GRE_IO_TYPE_WRONLY, &test_handle);
        if (result != 0) 
        {
            std::cout << "Failed to create write channel to controller" << std::endl;
            exit(2);
        }

        std::string event_name = "info";
        std::string event_data ="test message";
        const char* event_name_str = event_name.c_str();
        const char* event_data_str = event_data.c_str();

        result = sbio_send_event(test_handle, event_name_str, "1s0 client_channel", (void*)event_data_str, event_data.length() + 1);
        if (result != 0)
        {
            std::cout << "Failed to write message to control channel" << std::endl;
            exit(3);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::string new_message = test_subscriber->get_next_message();
        std::string expected_message = event_name + ":" + event_data;
        if (new_message == expected_message)
        {
            std::cout << "Subscriber read test passed" << std::endl;
        }
        else
        {
            std::cout << "Subscriber read test failed" << std::endl;
        }
        
        if (!test_subscriber->change_filter("bogus filter"))
        {
            std::cout << "Failed to set new filter on test subscriber" << std::endl;
            exit(4);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        result = sbio_send_event(test_handle, event_name_str, "1s0 client_channel", (void*)event_data_str, event_data.length() + 1);
        if (result != 0)
        {
            std::cout << "Failed to write message to control channel" << std::endl;
            exit(5);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        new_message = test_subscriber->get_next_message();
        
        if (new_message == "")
        {
            std::cout << "Subscriber filter test passed" << std::endl;
        }
        else
        {
            std::cout << "Subscriber filter test failed" << std::endl;
        }

        if (!test_subscriber->change_filter("in*"))
        {
            std::cout << "Failed to set new filter on test subscriber" << std::endl;
            exit(6);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        result = sbio_send_event(test_handle, event_name_str, "1s0 client_channel", (void*)event_data_str, event_data.length() + 1);
        if (result != 0)
        {
            std::cout << "Failed to write message to control channel" << std::endl;
            exit(7);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        new_message = test_subscriber->get_next_message();

        if (new_message == expected_message)
        {
            std::cout << "Subscriber wildcard filter test passed" << std::endl;
        }
        else
        {
            std::cout << "Subscriber wildcard filter failed" << std::endl;
        }
        exit(0);

        event_name = "media_info";
        event_name_str = event_name.c_str();

        result = sbio_send_event(test_handle, event_name_str, "1s0 client_channel", (void*)event_data_str, event_data.length() + 1);
        if (result != 0)
        {
            std::cout << "Failed to write message to control channel" << std::endl;
            exit(8);
        }        
        result = sbio_send_event(test_handle, event_name_str, "1s0 client_channel", (void*)event_data_str, event_data.length() + 1);
        if (result != 0)
        {
            std::cout << "Failed to write message to control channel" << std::endl;
            exit(8);
        }        
        result = sbio_send_event(test_handle, event_name_str, "1s0 client_channel", (void*)event_data_str, event_data.length() + 1);
        if (result != 0)
        {
            std::cout << "Failed to write message to control channel" << std::endl;
            exit(8);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        test_subscriber->change_filter("media*");

        std::this_thread::sleep_for(std::chrono::seconds(1));

        int i=0;
        for (i; i < 3; i++)
        {
            new_message = test_subscriber->get_next_message();
            if (new_message.length() == 0) 
            {
                std::cout << "Broker cached messages test failed" << std::endl;
                break;
            }
        }
        if (i == 3)
        {
            std::cout << "Broker cached messages test passed" << std::endl;
        }
        exit(0);
    }   
    return 0;
}
