#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../include/v8c.h"



///////////////////////////////////
//  TYPES
///////////////////////////////////


typedef struct
{
    v8_value_t      callback;
} queue_t;


///////////////////////////////////
//  UTILS
///////////////////////////////////


char *read_file(const char *path)
{
    char *buf;
    int fd = open(path, O_RDONLY);
    size_t len;

    // Check file descriptor.
    if (fd == -1)
        return (NULL);

    // Get file size.
    len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Try to allocate memory.
    if ((buf = malloc(sizeof(char) * (len + 1))) == NULL)
        return (NULL);

    // Read full file.
    len = read(fd, buf, len);
    close(fd);

    // Assign null terminated string.
    buf[len] = 0;
    return (buf);
}


///////////////////////////////////
//  V8 NATIVE
///////////////////////////////////


void func_println(v8_callback_info_t cb_info, int argc, void *data)
{
    char buf[1024];
    v8_value_t value;

    // Get first argument.
    v8_get_callback_args(cb_info, &value, 1);

    // Transform value to C string.
    v8_get_utf8(value, buf, 1024);

    // Put string in console.
    // puts(buf);
}

void func_schedule(v8_callback_info_t cb_info, int argc, queue_t *data)
{
    v8_value_t callback;

    // Get callback value.
    v8_get_callback_args(cb_info, &callback, argc);

    // Create a persistent reference to the callback.
    data->callback = v8_create_ref(callback);
}


///////////////////////////////////
//  V8 CALLBACKS
///////////////////////////////////


void on_context_start(v8_value_t global, void *data)
{
    queue_t queue   = { 0 };
    char *script    = read_file("script.js");

    // Set global object properties.
    v8_set_obj_var(global, "println"         , v8_create_function((v8_callback_info_t)func_println, NULL));
    v8_set_obj_var(global, "scheduleTicks"   , v8_create_function((v8_callback_info_t)func_schedule, &queue));

    // Run script.
    v8_script_run(script);

    // Get local value i don't know why but if the element is not referenced it is been freed by the garbage collector, OOF.
    v8_value_t callback = v8_get_ref_value(queue.callback);

    while (1) {
        // Call callback.
        v8_call_function(callback, NULL, 0);
    }

    // Free script.
    free(script);
}


///////////////////////////////////
//  MAIN
///////////////////////////////////


int main(int argc, char **argv)
{
    v8_instance_t *instance = v8_initialize(argv[0]);

    // Create a V8 isolate and JS stack.
    v8_isolate_start(on_context_start, NULL);

    v8_shutdown(instance);
    return (0);
}
