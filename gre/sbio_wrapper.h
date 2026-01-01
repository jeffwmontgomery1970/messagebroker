#ifndef SBIO_WRAPPER_H
#define SBIO_WRAPPER_H

// SBIO wrapper definitions
typedef void* sbio_channel_handle_t;
typedef void (*sbio_event_callback_t)(const char*, char*, void*, int, void*);

#define GRE_IO_TYPE_WRONLY 1

#ifdef __cplusplus
extern "C" {
#endif

int sbio_create_receive_channel(const char* name, int flags, sbio_channel_handle_t** handle);
int sbio_add_event_callback(sbio_channel_handle_t* handle, const char* filter, sbio_event_callback_t callback, void* user_data);
int sbio_create_send_channel(const char* name, int type, sbio_channel_handle_t** handle);
int sbio_send_event(sbio_channel_handle_t* handle, const char* event_name, const char* format, void* data, int size);
void sbio_destroy_channel(sbio_channel_handle_t* handle);
int sbio_rem_event_callback(sbio_channel_handle_t* handle, sbio_event_callback_t callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif