#pragma once

#include <stdint.h>
#include <stddef.h>
#include <uchar.h>

#ifdef __cplusplus
    // Only 64 bits is supported
    static_assert(sizeof(void*) == 8);

    extern "C" {
#endif


    ///////////////////////////////////
    //  STRUCTURES
    ///////////////////////////////////


    typedef struct v8_instance_t    v8_instance_t;
    typedef struct v8_isolate_t     v8_isolate_t;
    typedef struct v8_global_t      v8_global_t;

    typedef void*                   v8_value_t;
    typedef void const*             v8_callback_info_t;

    typedef void (*v8_start_cb_t)(void *data);
    typedef void (*v8_func_cb_t)(v8_callback_info_t cb_info, int argc, void *data);


    ///////////////////////////////////
    //  DEFINITIONS
    ///////////////////////////////////


    // Initializer
    v8_instance_t *v8_initialize(const char *path);
    void v8_shutdown(v8_instance_t *instance);

    // Javacript values
    void v8_create_int32(v8_value_t *result, int32_t value);
    void v8_create_uint32(v8_value_t *result, uint32_t value);
    void v8_create_int64(v8_value_t* result, int64_t value);
    void v8_create_double(v8_value_t* result, double value);
    void v8_create_object(v8_value_t *result);
    void v8_create_array(v8_value_t *result);
    void v8_create_array_with_length(v8_value_t* result, size_t length);
    void v8_create_string_utf8(v8_value_t* result, const char* str, size_t length);
    void v8_create_string_utf16(v8_value_t* result, const char16_t* str, size_t length);
    void v8_get_utf8(v8_value_t value, char* buf, size_t bufsize, size_t* result);

    // Global scope
    void v8_callback_get_args(v8_callback_info_t cb_info, v8_value_t* args, int argc);
    void v8_create_function(v8_value_t *result, v8_func_cb_t cb, void *data);
    void v8_create_global(v8_value_t *result);
    void v8_set_global(v8_value_t global, const char *key, v8_value_t value);

    // Context
    void v8_isolate_start(v8_start_cb_t start_cb, void *data);
    v8_value_t v8_script_run(v8_value_t global, const char *source_code);


#ifdef __cplusplus
    }
#endif
