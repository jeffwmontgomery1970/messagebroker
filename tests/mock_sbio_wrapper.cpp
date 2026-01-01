#include "mock_sbio_wrapper.h"
#include <cstdlib>

// Mock implementations of SBIO functions
extern "C" {
    int sbio_create_receive_channel(const char* name, int flags, sbio_channel_handle_t** handle) {
        static int dummy_handle = 1;
        *handle = reinterpret_cast<sbio_channel_handle_t*>(&dummy_handle);
        return 0; // Success
    }
    
    int sbio_add_event_callback(sbio_channel_handle_t* handle, const char* filter, sbio_event_callback_t callback, void* user_data) {
        return 0; // Success
    }
    
    int sbio_create_send_channel(const char* name, int type, sbio_channel_handle_t** handle) {
        static int dummy_handle = 2;
        *handle = reinterpret_cast<sbio_channel_handle_t*>(&dummy_handle);
        return 0; // Success
    }
    
    int sbio_send_event(sbio_channel_handle_t* handle, const char* event_name, const char* format, void* data, int size) {
        return 0; // Success
    }
    
    void sbio_destroy_channel(sbio_channel_handle_t* handle) {
        // Mock implementation - do nothing
    }
    
    int sbio_rem_event_callback(sbio_channel_handle_t* handle, sbio_event_callback_t callback, void* user_data) {
        return 0; // Success
    }
}