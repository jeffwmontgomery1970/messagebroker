#include "publisher.h"
#include "subscriber.h"
#include <algorithm>
#include <chrono>
#include <exception>

int MAX_MESSAGE_QUEUE_LENGTH = 10;

static void callback(const char *event_name, char *event_format, void *event_data, int event_data_size, void *user_data)
{
/**
 * Function used to receive callbacks on the read channel created
 * Converts incoming data to stings and calls the receiveMessage call within the registering class instance
 *
 * @param event_name String containing the name of the event sent
 * @param event_format String containing the format of the event sent
 * @param event_data Pointer to the event data sent
 * @param event_size Integer containing the size of the event data sent
 * @param user_data Pointer to any user_data that was registered in the read channel callback
 */
    std::string event_name_string, data_string;
    event_name_string = std::string(event_name);
    data_string = std::string((char*)event_data);
    Publisher * self = static_cast<Publisher*>(user_data);
    self->receiveMessage(event_name_string, data_string);
}

static bool is_match(const std::string &line_str, const std::string& pattern_str)
{
/**
 * Check for a match between a string and a string pattern with wildcards
 *
 * @param line_str String to find match within
 * @param pattern_str Pattern potentially with wildcards to find a match with
 * @return bool true if match is found, false if not
 */
    int wildcard = 0;

    const char* pattern = pattern_str.c_str();
    const char* line = line_str.c_str();

    const char* last_pattern_start = 0;
    const char* last_line_start = 0;
    do
    {
        if (*pattern == *line)
        {
            if(wildcard == 1)
                last_line_start = line + 1;

            line++;
            pattern++;
            wildcard = 0;
        }
        else if (*pattern == '?')
        {
            if(*(line) == '\0') // the line is ended but char was expected
                return false;
            if(wildcard == 1)
                last_line_start = line + 1;
            line++;
            pattern++;
            wildcard = 0;
        }
        else if (*pattern == '*')
        {
            if (*(pattern+1) == '\0')
            {
                return true;
            }

            last_pattern_start = pattern;
            //last_line_start = line + 1;
            wildcard = 1;

            pattern++;
        }
        else if (wildcard)
        {
            if (*line == *pattern)
            {
                wildcard = 0;
                line++;
                pattern++;
                last_line_start = line + 1 ;
            }
            else
            {
                line++;
            }
        }
        else
        {
            if ((*pattern) == '\0' && (*line) == '\0')
                return true;
            else
            {
                if (last_pattern_start != 0)
                {
                    pattern = last_pattern_start;
                    line = last_line_start;
                    last_line_start = 0;
                }
                else
                {
                    return false;
                }
            }
        }

    } while (*line);

    if (*pattern == '\0')
    {
        return true;
    }
    else
    {
        return false;
    }
}

BrokerMessage::BrokerMessage(const std::string& event, const std::string& data)
{
/**
 * BrokerMessage constructor
 * 
 * @param event String containing the event type for this message
 * @param data String containing the message data
 */
    event_type = event;
    message = data;
}

const std::string& BrokerMessage::getEventType()
{
/**
 * Get the event type for this broker message
 * 
 * @return const std::string& Reference to the event type string
 */
    return event_type;
}

const std::string& BrokerMessage::getMessage()
{
/**
 * Get the message data for this broker message
 * 
 * @return const std::string& Reference to the message data string
 */
    return message;
}

void MessageBuffer::addMessage(const BrokerMessage& new_message)
{
/**
 * Add a new message to the buffer, removing oldest if at capacity
 * 
 * @param new_message BrokerMessage to add to the buffer
 */
    if(messages.size() >= MAX_MESSAGE_QUEUE_LENGTH)
    {
        messages.erase(messages.begin());
    }
    messages.push_back(new_message);
}

std::vector<BrokerMessage>& MessageBuffer::getMessages()
{
/**
 * Get reference to the vector of messages in this buffer
 * 
 * @return std::vector<BrokerMessage>& Reference to the messages vector
 */
    return messages;
}

Publisher::Publisher(const std::string& control_channel, const std::string& log_path)
{
/**
 * Publisher constructor
 *
 * @param control_channel String containing the name that will be used to receive messages
 * @param log_path String containing the path to the log_file that will be used to log messages to
 * @return Publisher instance or exits program on error connecting class or registering for read callback
 */
    try
    {
        log_file.open(log_path, std::ios_base::app);
        const char *ctl = control_channel.c_str();
        int result = sbio_create_receive_channel(ctl, 0, &handle);
        int result2 = sbio_add_event_callback(handle, NULL, (sbio_event_callback_t )(callback), this);
        if (result != 0 || result2 != 0)
        {
            log_file << "Failed to register control channel or callback" << std::endl;
            log_file.flush();
            log_file.close();
            exit(2);
        }
    }
    catch (int e)
    {
        log_file << "Publisher constructor failed with exception: " << e << std::endl;
        log_file.flush();
        log_file.close();
        exit(1);
    }
}

Publisher::~Publisher()
{
/**
 * Publisher destructor
 *
 * Removes any remaining subscribers, flushes and closes log_file
 */
    while (subscribers.size() > 0)
    {
        Subscriber *next = subscribers.back();
        subscribers.pop_back();
        removeSubscriber(next);
    }
    log_file.flush();
    log_file.close();
}

void Publisher::publish(const std::string& event_type, const std::string &message)
{
/**
 * Publish incoming messages to the list of registered subscribers
 * Uses the filter in each subscriber register to determine if they should receive the message
 *
 * @param event_type String containing the event_type to be broadcast
 * @param message String containing the message to be broadcast
 */
    std::unique_lock<std::mutex> lock(mutex);
    BrokerMessage new_message(event_type, message);
    if (messageBuffers.find("no_filter") == messageBuffers.end())
    {
        MessageBuffer newBuffer;
        newBuffer.addMessage(new_message);
        messageBuffers.insert({"no_filter",newBuffer});
    }
    else
    {
        MessageBuffer& buffer = messageBuffers.find("no_filter")->second;
        buffer.addMessage(new_message);
    }
    if (event_type.find("media") == 0)
    {
        if (messageBuffers.find("media*") == messageBuffers.end())
        {
            MessageBuffer newBuffer;
            newBuffer.addMessage(new_message);
            messageBuffers.insert({"media*", newBuffer});
        }
        else
        {
            MessageBuffer& buffer = messageBuffers.find("media*")->second;
            buffer.addMessage(new_message);
        }
    }
    if (messageBuffers.find(event_type) == messageBuffers.end())
    {
        MessageBuffer newBuffer;
        newBuffer.addMessage(new_message);
        messageBuffers.insert({event_type, newBuffer});
    }
    else
    {
        MessageBuffer& buffer = messageBuffers.find(event_type)->second;
        buffer.addMessage(new_message);
    }

    for (auto subscriber : subscribers)
    {
        if (this->checkFilter(subscriber, event_type))
        {
            subscriber->send_message(event_type, message);
            log_file << "Sent event: " << event_type << " with message: " << message << " to channel: " << subscriber->get_channel_name() << std::endl;
        }
        else
        {
            log_file << "Skipped subscriber as filter: " << subscriber->get_filter() << " did not match event_type: " << event_type << std::endl;
        }
    }
}

void Publisher::addSubscriber(Subscriber *subscriber)
{/**
 * Add subscriber to subscriber list
 *
 * @param subscriber Pointer to Subscriber instance to be added to the list
 */
    std::unique_lock<std::mutex> lock(mutex);
    subscribers.push_back(subscriber);
}

void Publisher::removeSubscriber(Subscriber *subscriber)
{
/**
 * Remove subscriber from subscriber list
 *
 * @param subscriber Pointer to subscriber instance to remove from list
 */
    std::unique_lock<std::mutex> lock(mutex);
    subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriber), subscribers.end());
}

void Publisher::receiveMessage(const std::string& event_type, const std::string& message)
{
/**
 * Called from callback when SBIO channel receives a new message
 * Creates or deletes subscribers when event_type is subscribe or unsubscribe
 * Otherwise broadcasts the message to all subscribers in list with matching event filter
 * When shutdown message received function waits for all subscribers to unsubscribe and exits
 *
 * @param event_type String containing incoming event_type
 * @param message String containing incoming message
 */
    if (event_type == "subscribe" || event_type == "unsubscribe")
    {
        std::string channel_name = "";
        std::string filter = "";
        int split_pos = message.find(":");
        if (split_pos == std::string::npos)
        {
            channel_name = message;
        }
        else
        {
            channel_name = message.substr(0,split_pos);
            if (split_pos < message.length())
            {
                filter = message.substr(split_pos+1,message.length());
            }
        }
        Subscriber *existing_sub = NULL;
        for (Subscriber *next : subscribers)
        {
            if (next->get_channel_name() == channel_name && next->get_filter() == filter)
            {
                existing_sub = next;
                break;
            }
        } 
        if (event_type == "subscribe")
        {
            if (existing_sub == NULL)
            {
                try
                {
                    Subscriber* new_sub = new Subscriber(channel_name,"", filter);
                    addSubscriber(new_sub);
                    log_file << "Added subscriber: " << channel_name << " with Filter: " << filter << std::endl;
                    MessageBuffer* messages = NULL;
                    if (filter.length() == 0)
                    {
                        if (messageBuffers.find("no_filter") != messageBuffers.end())
                        {
                            messages = &(messageBuffers.find("no_filter")->second);
                        }
                    }
                    else if (messageBuffers.find(filter) != messageBuffers.end())
                    {
                        messages = &(messageBuffers.find(filter)->second);
                    }
                    if (messages != NULL)
                    {
                        for (auto message : messages->getMessages())
                        {
                            new_sub->send_message(message.getEventType(), message.getMessage());
                        }
                    }
                }
                catch (int e)
                {
                    log_file << "Exception creating Subscriber: " << e << std::endl;
                }
            }
            else
            {
                log_file << "Subscription already exists for channel: " << channel_name << " and filter: " << filter << std::endl;
            }
        }
        else
        {
            if (existing_sub == NULL)
            {
                log_file << "No subscription found for channel: " << channel_name << " and filter:" << filter << std::endl; 
            }
            else
            {
                removeSubscriber(existing_sub);
                log_file << "Removed subscriber: " << channel_name << " with Filter: " << filter << std::endl;
            }
        }
    }
    else
    {
        publish(event_type, message);
        log_file << "Broadcast event: " << event_type << " with Message: " << message << std::endl;
        if (event_type == "shutdown")
        {
            while (subscribers.size() > 0)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            log_file << "Shutting down message broker" << std::endl;
            delete this;
            exit(0);
        }
    }
}

bool Publisher::checkFilter(class Subscriber *subscriber, const std::string& event_type)
{
/**
 * Check if an event type matches a subscriber's filter
 * 
 * @param subscriber Pointer to the subscriber to check filter for
 * @param event_type String containing the event type to match against filter
 * @return bool Returns true if event matches filter or no filter set, false otherwise
 */
    std::string filter = subscriber->get_filter();
    if (filter.length() == 0)
    {
        return true;
    }
    else 
    {
        return(is_match(event_type, filter));
    }
}