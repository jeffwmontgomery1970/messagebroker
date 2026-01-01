#include "subscriber.h"
#include <iostream>

std::mutex global_lock;

static void callback(const char *event_name, char *event_format, void *event_data, int event_data_size, void *user_data)
{
 /**
 * Callback function used to register against the SBIO read channel
 * Messages to the read channel will be sent to this callback function
 * This function expects the user_data to be the pointer to the instance that created the read channel
 * The receive function of the Subscriber instance being pointed to will be called with the incoming message
 * 
 * @param event_name String containing the event_name for the message being received
 * @param event_format String containing the event format of the message being received
 * @param event_data Pointer to the event_data for the message being received
 * @param event_data_size Integer containing the size of the data object being received
 * @param user_data Pointer to any user_data that was register when creating the read channel
 */
    std::string event_name_string, data_string;
    event_name_string = std::string(event_name);
    data_string = std::string((char*)event_data);
    Subscriber * self = static_cast<Subscriber*>(user_data);
    self->receive(event_name_string, data_string);
}

Subscriber::Subscriber(const std::string& channel_name_str, const std::string& controller_name_str, const std::string& filter_str)
{
/**
 * Subscriber constructor
 * If the controller_name_str is empty the Subscriber will be used to connect to the write channel for the Subscriber
 * The channel_name_str will be used to set the name of the channel the subscriber will communicate on
 *
 * @param control_channel String containing the channel name of the subscriber
 * @param controller_name_str Optional string used when Subscriber will be used to write subscribe messages to the controller
 * @param filter String containing the filter that will be used to determine which event_names will be processed
 * @return Sibscriber instance or throws an std::runtime_error exception
 */
    filter = filter_str;
    channel_name = channel_name_str;
    controller_channel = controller_name_str;
    if (controller_channel.length() == 0) 
    {
        const char * channel_name_str = channel_name.c_str();
        
        int result = sbio_create_send_channel(channel_name_str, GRE_IO_TYPE_WRONLY, &handle);
        if (result != 0) {
            std::string errMsg = "Could not create the write channel for name: " + channel_name;
            throw std::runtime_error(errMsg);
        }
    }
}

Subscriber::~Subscriber()
{
/**
 * Subscriber destructor
 * Send message to controller to remove subscriber from message broker if applicable
 * Destroy the channel used for subscriber communications
 */
    if (controller_channel.length() > 0)
    {
        unsubscribe(filter);  
    }
    sbio_destroy_channel(handle);
}

void Subscriber::receive(const std::string& event_name, const std::string& event_data)
{
/**
 * Receive incoming messages send from the read channel callback
 * When shutdown message received call the stop function for this Subscriber instance
 * 
 * @param event_name String containing the event_name send in the received message
 * @param event_data String containing the message sent
 */
    std::string message = event_name + ":" + event_data;
    this->m_queue.push(message);
    if (message.find("shutdown") != std::string::npos && sub_thread.joinable())
    {
        this->stop();
    }
}

bool Subscriber::send_message(const std::string& event_type, const std::string& event_data)
{
/**
 * Send messages on the write channel for a subscriber
 * 
 * @param event_type String containing the event_type for the message to be sent
 * @param event_data String containing the message data to be sent
 * @return bool Returns true if message sent successfully, false otherwise
 */
    const char* event_type_str = event_type.c_str();
    const char* event_data_str = event_data.c_str();
    int event_data_size = event_data.length() + 1;
    int result = sbio_send_event(handle, event_type_str, "1s0 client_channel", (void*) event_data_str, event_data_size);
    if (result != 0)
    {
        return false;
    }  
    else 
    {
        return true;
    }
}


bool Subscriber::start()
{
/**
 * Connect subscriber to read and write channels and register with controller
 * Creates receive channel, adds event callback, creates controller send channel,
 * and sends subscribe message to controller. Runs main event loop.
 * 
 * @return bool Returns true if all connections successful and loop completes, false on error
 */
    is_running = false;
    const char* channel_name_str = channel_name.c_str();
    const char* controller_channel_str = controller_channel.c_str();
    std::string event_data = channel_name + ":" + filter;
    const char* event_data_str = event_data.c_str();
    int event_data_size = event_data.length() + 1;
    int result = sbio_create_receive_channel(channel_name_str, 0, &handle);
    if (result != 0) 
    {
        log_file << "sbio_create_receive_channel failed for channel name: " << channel_name << std::endl;
        return false;
    }
    result = sbio_add_event_callback(handle, NULL, (sbio_event_callback_t)(callback), this);
    if (result != 0) 
    {
        log_file << "sbio_add_event_callback failed for channel name: " << channel_name << std::endl;
        return false;
    }
    result = sbio_create_send_channel(controller_channel_str, GRE_IO_TYPE_WRONLY, &controller);
    if (result != 0) 
    {
        log_file << "sbio_create_send_channel failed for channel name: " << controller_channel << std::endl;
        return false;
    }
    result = sbio_send_event(controller, "subscribe", "1s0 client_channel", (void*)event_data_str, event_data_size);
    if (result != 0) 
    {
        log_file << "sbio_send_event failed for channel name: " << controller_channel << std::endl;
        return false;
    }
    is_running = true;
    while (is_running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    sbio_rem_event_callback(handle, callback, this);
    return true;
}

bool Subscriber::change_filter(const std::string& new_filter)
{
/**
 * Change the filter used to determine which event_types are received by the subscriber
 * Makes unsubscribe call with old filter to remove subscriber from message broker
 * Makes subscribe call with new filter value to add new subscriber to message broker
 * 
 * @param new_filter String containing the filter value to set for the subscriber channel
 * @return bool Return true if new filter is applied and false otherwise
 */
    if (!unsubscribe(filter))
    {
        return false;
    }
    std::string subscribe_data = channel_name + ":" + new_filter;
    const char *subscribe_data_str = subscribe_data.c_str();
    filter = new_filter;
    int result = sbio_send_event(controller, "subscribe", "1s0 client_channel", (void*)subscribe_data_str, subscribe_data.length() + 1);
    if (result == 0)
    {
        return true;
    }
    else
    {
        log_file << "Failed to change filter to new value: " << new_filter << std::endl;
        return false;
    }
}

bool Subscriber::unsubscribe(const std::string& old_filter)
{
/**
 * Send unsubscribe message to the message broker
 * 
 * @param old_filter String containing the filter value to set for the subscriber channel
 * @return bool Return true if new filter is applied and false otherwise
 */
    std::string unsubscribe_data = channel_name + ":" + old_filter;
    const char *unsubscribe_data_str = unsubscribe_data.c_str();
    int result = sbio_send_event(controller, "unsubscribe", "1s0 client_channel", (void*)unsubscribe_data_str, unsubscribe_data.length() + 1);
    if (result == 0)
    {
        return true;
    }
    else
    {
        log_file << "Failed to unsubscribe filter: " << unsubscribe_data << std::endl;
        return false;
    }
}

void Subscriber::stop()
{
/**
 * Stop monitoring read channel for subscriber
 * Sends unsubscribe message to message broken until succesdful
 * Sets is_running flag and joins thread subscriber was runnign on
 */
    while (!unsubscribe(filter))
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    is_running = false;
    sub_thread.join();
}

void Subscriber::run(const std::string& log_file_path)
{
/**
 * Creates and starts a thread for the subscriber to run on
 * Opens the log file and launches the start method in a separate thread
 * 
 * @param log_file_path String containing the path to the log file for this subscriber
 */ 
    log_file.open(log_file_path, std::ios_base::app);
    sub_thread = std::thread(&Subscriber::start, this);
}

std::string Subscriber::get_channel_name()
{
/**
 * Get subscriber read channel name
 * @return String with the read channel name
 */
    return channel_name;
}

std::string Subscriber::get_filter()
{
/**
 * Get current subscriber filter
 * @return String with filter value in it
 */
    return filter;
}

std::string Subscriber::get_next_message()
{
/**
 * Return next string store in the subscriber's read queue
 * Return empty string if nothing in the queue
 * Message is popped from the queue if available
 * 
 * @return String next message in the read queue or empty string
 */
    if (!m_queue.empty())
    {
        const std::string message = m_queue.front();
        m_queue.pop();
        if (message.find("shutdown") == std::string::npos)
        {
            return message;
        }
        else
        {
            return "Broker has shutdown";
        }
    }
    else
    {
        return "";
    }
}