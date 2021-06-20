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
        return reinterpret_cast<v8_value_t>(*local);
    }

    inline v8_value_t ValueFromV8Template(v8::Local<v8::Template> local)
    {
        return reinterpret_cast<v8_value_t>(*local);
    }

    // PR - NJS
    inline v8::Local<v8::Value> V8FromValue(v8_value_t value)
    {
        return *reinterpret_cast<v8::Local<v8::Value>*>(&value);
    }

    inline v8::Local<v8::ObjectTemplate> V8ObjectTemplateFromValue(v8_value_t value)
    {
        return *reinterpret_cast<v8::Local<v8::ObjectTemplate>*>(&value);
    }
};


///////////////////////////////////
//  V8
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

    return reinterpret_cast<v8_instance_t*>(platform.release());
}

void v8_shutdown(v8_instance_t *instance)
{
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete reinterpret_cast<v8::Platform*>(instance);
}


///////////////////////////////////
//  VALUES
///////////////////////////////////


void v8_create_int32(v8_value_t *result, int32_t value)
{
    *result = v8impl::ValueFromV8(v8::Integer::New(v8::Isolate::GetCurrent(), value));
}

void v8_create_uint32(v8_value_t *result, uint32_t value)
{
    *result = v8impl::ValueFromV8(v8::Integer::NewFromUnsigned(v8::Isolate::GetCurrent(), value));
}

void v8_create_int64(v8_value_t *result, int64_t value)
{
    *result = v8impl::ValueFromV8(v8::Number::New(v8::Isolate::GetCurrent(), static_cast<double>(value)));
}

void v8_create_double(v8_value_t *result, double value)
{
    *result = v8impl::ValueFromV8(v8::Number::New(v8::Isolate::GetCurrent(), value));
}

void v8_create_object(v8_value_t *result)
{
    *result = v8impl::ValueFromV8(v8::Object::New(v8::Isolate::GetCurrent()));
}

void v8_create_array(v8_value_t *result)
{
    *result = v8impl::ValueFromV8(v8::Array::New(v8::Isolate::GetCurrent()));
}

void v8_create_array_with_length(v8_value_t *result, size_t length)
{
    *result = v8impl::ValueFromV8(v8::Array::New(v8::Isolate::GetCurrent(), length));
}

void v8_create_string_utf8(v8_value_t *result, const char *str, size_t length)
{
    *result = v8impl::ValueFromV8(
        v8::String::NewFromUtf8(
            v8::Isolate::GetCurrent(),
            str,
            v8::NewStringType::kNormal,
            static_cast<int>(length)
        ).ToLocalChecked()
    );
}

void v8_create_string_utf16(v8_value_t *result, const char16_t *str, size_t length)
{
    *result = v8impl::ValueFromV8(
        v8::String::NewFromTwoByte(
            v8::Isolate::GetCurrent(),
            reinterpret_cast<const uint16_t*>(str),
            v8::NewStringType::kNormal,
            static_cast<int>(length)
        ).ToLocalChecked()
    );
}

void v8_get_utf8(v8_value_t value, char *buf, size_t bufsize, size_t *result)
{
    v8::Local<v8::Value> val = v8impl::V8FromValue(value);

    *result = val.As<v8::String>()->WriteUtf8(
        v8::Isolate::GetCurrent(),
        buf,
        bufsize,
        nullptr,
        v8::String::REPLACE_INVALID_UTF8
    );
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

void v8_callback_get_args(v8_callback_info_t cb_info, v8_value_t* args, int argc)
{
    auto JsArgs = *reinterpret_cast<const v8::FunctionCallbackInfo<v8::Value>*>(cb_info);

    for (int i = 0; i < argc; i++)
        args[i] = v8impl::ValueFromV8(JsArgs[i]);
}

void v8_create_function(v8_value_t *result, v8_func_cb_t cb, void *data)
{
    // Get current isolate.
    auto isolate        = v8::Isolate::GetCurrent();

    // Create a bundle.
    auto bundle         = std::make_unique<v8_bundle_t>();
    bundle->func_cb     = cb;
    bundle->data        = data;

    // Create an external V8 value to wrap the bundle in.
    v8::Local<v8::Value> cb_data = v8::External::New(isolate, bundle.release());

    // Create a persistent wrapper for the external value.
    // v8::Persistent<v8::Value> persistent(v8::Isolate::GetCurrent(), cb_data);

    // Assign a finalizer callback to destroy the bundle.
    // persistent.SetWeak(bundle, finalize_callback, v8::WeakCallbackType::kFinalizer);

    *result = v8impl::ValueFromV8Template(
        v8::FunctionTemplate::New(
            isolate,
            v8_callback_function,
            cb_data
        )
    );
}


///////////////////////////////////
//  GLOBAL
///////////////////////////////////


void v8_create_global(v8_value_t *result)
{
    *result = v8impl::ValueFromV8Template(v8::ObjectTemplate::New(v8::Isolate::GetCurrent()));
}

void v8_set_global(v8_value_t global, const char *key, v8_value_t value)
{
    v8::Local<v8::ObjectTemplate> object = v8impl::V8ObjectTemplateFromValue(global);

    object->Set(
        v8::Isolate::GetCurrent(),
        key,
        v8impl::V8FromValue(value)
    );
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

        // Call init callback.
        start_cb(data);
    }

    // Dispose isolate and delete allocator.
    isolate->Dispose();
    delete allocator;
}

v8_value_t v8_script_run(v8_value_t global, const char *source_code)
{
    // Get current isolate.
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, v8impl::V8ObjectTemplateFromValue(global));

    // @TODO modules
    // v8::Module::InstantiateModule(context);

    // Enter the context for compiling and running the script.
    v8::Context::Scope context_scope(context);

    // Create a string containing the JavaScript source code.
    v8::MaybeLocal<v8::String> maybe_source = v8::String::NewFromUtf8(isolate, source_code);

    if (maybe_source.IsEmpty()) {
        return (NULL);
    }

    // Compile the source code and detect errors.
    v8::MaybeLocal<v8::Script> maybe_script = v8::Script::Compile(context, maybe_source.ToLocalChecked());

    if (maybe_script.IsEmpty()) {
        return (NULL);
    }

    // Run the script.
    v8::Local<v8::Value> result_def = static_cast<v8::Local<v8::Value>>(v8::Undefined(isolate));
    v8::Local<v8::Value> result     = maybe_script.ToLocalChecked()->Run(context).FromMaybe(result_def);

    // Return result.
    return (v8impl::ValueFromV8(result));
}
