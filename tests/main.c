#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/v8c.h"


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
    size_t bufsize;
    v8_value_t value = NULL;

    // Get first argument.
    v8_callback_get_args(cb_info, &value, 1);

    // Transform value to C string.
    v8_get_utf8(value, buf, 1024, &bufsize);

    // Put string in console.
    puts(buf);
}

void func_ak(v8_callback_info_t cb_info, int argc, void *data)
{
    printf("TU TAI FAI AK!\n");
}


///////////////////////////////////
//  V8 CALLBACKS
///////////////////////////////////


void on_context_start(void *data)
{
    char *script = read_file("script.js");

    v8_value_t global;
    v8_value_t val_println;
    v8_value_t val_ak;
    v8_value_t val_version;

    // Create builtins functions.
    v8_create_function(&val_println, func_println, NULL);
    v8_create_function(&val_ak, func_ak, NULL);

    // Create builtins getter.
    v8_create_string_utf8(&val_version, "AKING V1.0", 11);

    // Create a global template and define builtins.
    v8_create_global(&global);
    v8_set_global(global, "println", val_println);
    v8_set_global(global, "ak", val_ak);
    v8_set_global(global, "version", val_version);

    // Run script.
    v8_script_run(global, script);

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
