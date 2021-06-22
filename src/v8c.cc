#include "v8.h"
#include "v8c.h"
#include "libplatform/libplatform.h"


///////////////////////////////////
//  UTILS
///////////////////////////////////


namespace v8impl
{
    static_assert(
        sizeof(v8::Local<v8::Value>) == sizeof(v8_value_t),
        "Cannot convert between v8::Local<v8::Value> and napi_value"
    );

    inline v8_value_t ValueFromV8(v8::Local<v8::Value> local)
    {
        return (reinterpret_cast<v8_value_t>(*local));
    }

    inline v8_template_t TemplateFromV8(v8::Local<v8::Template> local)
    {
        return (reinterpret_cast<v8_template_t>(*local));
    }

    // PR - NJS
    inline v8::Local<v8::Value> V8FromValue(v8_value_t value)
    {
        return (*reinterpret_cast<v8::Local<v8::Value>*>(&value));
    }

    inline v8::Local<v8::Object> V8ObjectFromValue(v8_value_t value)
    {
        return (*reinterpret_cast<v8::Local<v8::Object>*>(&value));
    }
};


///////////////////////////////////
//  INIT
///////////////////////////////////


v8_instance_t *v8_initialize(const char *path)
{
    // Initialize external data.
    v8::V8::InitializeICUDefaultLocation(path);
    v8::V8::InitializeExternalStartupData(path);

    // Create new instance and assign platform.
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform(1);

    // Initialize V8 engine.
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    return (reinterpret_cast<v8_instance_t*>(platform.release()));
}

void v8_shutdown(v8_instance_t *instance)
{
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete (reinterpret_cast<v8::Platform*>(instance));
}


///////////////////////////////////
//  VALUES
///////////////////////////////////


v8_value_t v8_create_int32(int32_t value)
{
    return (v8impl::ValueFromV8(v8::Integer::New(v8::Isolate::GetCurrent(), value)));
}

v8_value_t v8_create_uint32(uint32_t value)
{
    return (v8impl::ValueFromV8(v8::Integer::NewFromUnsigned(v8::Isolate::GetCurrent(), value)));
}

v8_value_t v8_create_int64(int64_t value)
{
    return (v8impl::ValueFromV8(v8::Number::New(v8::Isolate::GetCurrent(), static_cast<double>(value))));
}

v8_value_t v8_create_double(double value)
{
    return (v8impl::ValueFromV8(v8::Number::New(v8::Isolate::GetCurrent(), value)));
}

v8_value_t v8_create_array(size_t size)
{
    return (v8impl::ValueFromV8(v8::Array::New(v8::Isolate::GetCurrent(), size)));
}

v8_value_t v8_create_uint8_array(size_t size)
{
    v8::Isolate *isolate                = v8::Isolate::GetCurrent();
    v8::Local<v8::ArrayBuffer> buffer   = v8::ArrayBuffer::New(isolate, size);
    v8::Local<v8::Uint8Array> array     = v8::Uint8Array::New(buffer, 0, size);

    return (v8impl::ValueFromV8(array));
}

v8_value_t v8_create_uint16_array(size_t size)
{
    v8::Isolate *isolate                = v8::Isolate::GetCurrent();
    v8::Local<v8::ArrayBuffer> buffer   = v8::ArrayBuffer::New(isolate, size);
    v8::Local<v8::Uint16Array> array    = v8::Uint16Array::New(buffer, 0, size);

    return (v8impl::ValueFromV8(array));
}

v8_value_t v8_create_uint32_array(size_t size)
{
    v8::Isolate *isolate                = v8::Isolate::GetCurrent();
    v8::Local<v8::ArrayBuffer> buffer   = v8::ArrayBuffer::New(isolate, size);
    v8::Local<v8::Uint32Array> array    = v8::Uint32Array::New(buffer, 0, size);

    return (v8impl::ValueFromV8(array));
}

v8_value_t v8_create_string_utf8(const char *str)
{
    return (
        v8impl::ValueFromV8(
            v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), str).ToLocalChecked()
        )
    );
}

v8_value_t v8_create_string_utf16(const char16_t *str)
{
    return (
        v8impl::ValueFromV8(
            v8::String::NewFromTwoByte(
                v8::Isolate::GetCurrent(),
                reinterpret_cast<const uint16_t*>(str)
            ).ToLocalChecked()
        )
    );
}

v8_value_t v8_create_object()
{
    return (v8impl::ValueFromV8(v8::Object::New(v8::Isolate::GetCurrent())));
}

v8_template_t v8_create_function_template()
{
    return (v8impl::TemplateFromV8(v8::FunctionTemplate::New(v8::Isolate::GetCurrent())));
}

v8_value_t v8_create_ref(v8_value_t _value)
{
    v8::Isolate *isolate                    = v8::Isolate::GetCurrent();
    v8::Persistent<v8::Value> *persistent   = new v8::Persistent<v8::Value>(isolate, v8impl::V8FromValue(_value));

    return (reinterpret_cast<v8_value_t>(persistent));
}


///////////////////////////////////
//  GETTERS
///////////////////////////////////


v8_value_t v8_get_ref_value(v8_value_t ref)
{
    v8::Persistent<v8::Value> &persistent   = *reinterpret_cast<v8::Persistent<v8::Value>*>(ref);
    v8::Local<v8::Value> value              = v8::Local<v8::Value>::New(v8::Isolate::GetCurrent(), persistent);

    return (v8impl::ValueFromV8(value));
}

size_t v8_get_utf8(v8_value_t value, char *buf, size_t len)
{
    v8::Local<v8::Value> val = v8impl::V8FromValue(value);

    return (
        val.As<v8::String>()->WriteUtf8(
            v8::Isolate::GetCurrent(),
            buf,
            len,
            nullptr,
            v8::String::REPLACE_INVALID_UTF8
        )
    );
}

void v8_get_callback_args(v8_callback_info_t cb_info, v8_value_t* args, int argc)
{
    auto JsArgs = *reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(cb_info);

    for (int i = 0; i < argc; i++)
        args[i] = v8impl::ValueFromV8(JsArgs[i]);
}


///////////////////////////////////
//  SETTERS
///////////////////////////////////


void v8_set_obj_var(v8_value_t obj, const char *key, v8_value_t value)
{
    v8::Isolate *isolate            = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context  = isolate->GetCurrentContext();
    v8::Local<v8::Object> object    = context->Global();

    object->Set(
        context,
        v8::String::NewFromUtf8(isolate, key).ToLocalChecked(),
        v8impl::V8FromValue(value)
    ).FromJust();
}


///////////////////////////////////
//  FUNCTIONS
///////////////////////////////////


struct v8_bundle_t
{
    v8_func_cb_t    func_cb;
    void            *data;
};

void v8_callback_function(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    // Get external data pointer.
    v8_bundle_t bundle          =  *reinterpret_cast<v8_bundle_t*>(v8::Local<v8::External>::Cast(args.Data())->Value());
    v8_callback_info_t cb_info  =   reinterpret_cast<v8_callback_info_t>(&args);

    // Call callback.
    bundle.func_cb(cb_info, args.Length(), bundle.data);
}

v8_value_t v8_create_function(v8_func_cb_t cb, void *data)
{
    // Get current isolate and current context.
    v8::Isolate *isolate            = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context  = isolate->GetCurrentContext();

    // Create a bundle.
    auto bundle         = std::make_unique<v8_bundle_t>();
    bundle->func_cb     = cb;
    bundle->data        = data;

    // Create an external V8 value to wrap the bundle in.
    v8::Local<v8::Value> cb_data = v8::External::New(isolate, bundle.release());

    return (
        v8impl::ValueFromV8(
            v8::Function::New(
                context,
                v8_callback_function,
                cb_data
            ).ToLocalChecked()
        )
    );
}

v8_value_t v8_call_function(v8_value_t func, v8_value_t *args, size_t count)
{
    // Get current isolate and current context.
    v8::Isolate *isolate                = v8::Isolate::GetCurrent();

    // Enter a new scope before calling the function to prevent memory grow.
    v8::HandleScope scope(isolate);

    v8::Local<v8::Context> context      = isolate->GetCurrentContext();
    v8::Local<v8::Function> function    = v8::Local<v8::Function>::Cast(v8impl::V8FromValue(func));

    // Call Javascript function.
    return (v8impl::ValueFromV8(
        function->Call(
            context,
            v8::Undefined(isolate),
            count,
            reinterpret_cast<v8::Local<v8::Value>*>(args)
        ).ToLocalChecked()
    ));
}


///////////////////////////////////
//  CONTEXT
///////////////////////////////////


void v8_isolate_start(v8_start_cb_t start_cb, void *data)
{
    // Create allocator.
    v8::ArrayBuffer::Allocator *allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    // Create params.
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator        = allocator;
    create_params.only_terminate_in_safe_scope  = true;

    // Create isolate and enter.
    v8::Isolate *isolate = v8::Isolate::New(create_params);

    // Capture uncaught exceptions.
    isolate->SetCaptureStackTraceForUncaughtExceptions(true);

    // Create a sub stack to enter isolate.
    {
        // Create a stack-allocated isolate scope.
        v8::Isolate::Scope isolate_scope(isolate);

        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(isolate);

        // Create a new context.
        v8::Local<v8::Context> context = v8::Context::New(isolate);

        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);

        // Call init callback.
        start_cb(v8impl::ValueFromV8(context->Global()), data);
    }

    // Dispose isolate and delete allocator.
    isolate->Dispose();
    delete (allocator);
}

v8_value_t v8_script_run(const char *source_code)
{
    // Get current isolate and context.
    v8::Isolate *isolate            = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context  = isolate->GetCurrentContext();

    // @TODO modules
    // v8::Module::InstantiateModule(context);

    // Create a string containing the JavaScript source code.
    v8::MaybeLocal<v8::String> maybe_source = v8::String::NewFromUtf8(isolate, source_code);

    if (maybe_source.IsEmpty()) {
        return (nullptr);
    }

    // Compile the source code and detect errors.
    v8::MaybeLocal<v8::Script> maybe_script = v8::Script::Compile(context, maybe_source.ToLocalChecked());

    if (maybe_script.IsEmpty()) {
        return (nullptr);
    }

    // Run the script.
    v8::Local<v8::Value> result_def = static_cast<v8::Local<v8::Value>>(v8::Undefined(isolate));
    v8::Local<v8::Value> result     = maybe_script.ToLocalChecked()->Run(context).FromMaybe(result_def);

    // Return result.
    return (v8impl::ValueFromV8(result));
}
