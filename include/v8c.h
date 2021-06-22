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
    typedef void*                   v8_value_t;
    typedef void*                   v8_template_t;
    typedef const void*             v8_callback_info_t;

    typedef void (*v8_start_cb_t)(v8_value_t global, void *data);
    typedef void (*v8_func_cb_t)(v8_callback_info_t cb_info, int argc, void *data);


    ///////////////////////////////////
    //  DEFINITIONS
    ///////////////////////////////////


    // Init
    v8_instance_t *v8_initialize(const char *path);
    void v8_shutdown(v8_instance_t *instance);

    // Values
    v8_value_t v8_create_int32(int32_t value);
    v8_value_t v8_create_uint32(uint32_t value);
    v8_value_t v8_create_int64(int64_t value);
    v8_value_t v8_create_double(double value);
    v8_value_t v8_create_array(size_t length);
    v8_value_t v8_create_uint8_array(size_t size);
    v8_value_t v8_create_uint16_array(size_t size);
    v8_value_t v8_create_uint32_array(size_t size);
    v8_value_t v8_create_string_utf8(const char* str);
    v8_value_t v8_create_string_utf16(const char16_t* str);
    v8_value_t v8_create_object(void);
    v8_template_t v8_create_function_template(void);
    v8_value_t v8_create_ref(v8_value_t value);

    // Getters
    v8_value_t v8_get_ref_value(v8_value_t ref);
    size_t v8_get_utf8(v8_value_t value, char* buf, size_t len);
    void v8_get_callback_args(v8_callback_info_t cb_info, v8_value_t* args, int argc);

    // Setters
    void v8_set_obj_var(v8_value_t object, const char *key, v8_value_t value);
    void v8_set_template_var(v8_template_t object, const char key, v8_value_t value);

    // Functions
    v8_value_t v8_create_function(v8_func_cb_t cb, void *data);
    v8_value_t v8_call_function(v8_value_t func, v8_value_t *args, size_t count);

    // Context
    void v8_isolate_start(v8_start_cb_t start_cb, void *data);
    v8_value_t v8_script_run(const char *source_code);


#ifdef __cplusplus
    }
#endif
