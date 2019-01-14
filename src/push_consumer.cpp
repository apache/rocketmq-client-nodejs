/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <map>
#include "push_consumer.h"
#include "consumer_ack.h"
#include "workers/push_consumer/start_or_shutdown.h"

using namespace std;

namespace __node_rocketmq__ {

struct MessageHandlerParam
{
    RocketMQPushConsumer* consumer;
    ConsumerAckInner* ack;
    CMessageExt* msg;
};
char message_handler_param_keys[5][8] = { "topic", "tags", "keys", "body", "msgId" };

uv_mutex_t _get_msg_ext_column_lock;

map<CPushConsumer*, RocketMQPushConsumer*> _push_consumer_map;

#define NAN_GET_CPUSHCONSUMER() \
    RocketMQPushConsumer* _v8_consumer = ObjectWrap::Unwrap<RocketMQPushConsumer>(info.Holder()); \
    CPushConsumer* consumer_ptr = _v8_consumer->GetConsumer();

Nan::Persistent<Function> RocketMQPushConsumer::constructor;

RocketMQPushConsumer::RocketMQPushConsumer(const char* group_id, const char* instance_name) :
    consumer_ptr(NULL)
{
    consumer_ptr = CreatePushConsumer(group_id);

    if(instance_name)
    {
        SetPushConsumerInstanceName(consumer_ptr, instance_name);
    }

    _push_consumer_map[consumer_ptr] = this;

    RegisterMessageCallback(consumer_ptr, RocketMQPushConsumer::OnMessage);
}

RocketMQPushConsumer::~RocketMQPushConsumer()
{
    try
    {
        ShutdownPushConsumer(consumer_ptr);
        auto it = _push_consumer_map.find(consumer_ptr);
        if(it != _push_consumer_map.end())
        {
            _push_consumer_map.erase(consumer_ptr);
        }
    }
    catch(...)
    {
        //
    }

    DestroyPushConsumer(consumer_ptr);
    consumer_ptr = NULL;
}

void RocketMQPushConsumer::SetOptions(Local<Object> options)
{
    // set name server
    Local<Value> _name_server_v = Nan::Get(options, Nan::New<String>("nameServer").ToLocalChecked()).ToLocalChecked();
    if(_name_server_v->IsString())
    {
        Nan::Utf8String namesrv(_name_server_v);
        SetPushConsumerNameServerAddress(consumer_ptr, *namesrv);
    }

    // set thread count
    Local<Value> _thread_count_v = Nan::Get(options, Nan::New<String>("threadCount").ToLocalChecked()).ToLocalChecked();
    if(_thread_count_v->IsNumber())
    {
        int thread_count = Nan::To<int32_t>(_thread_count_v).FromJust();
        if(thread_count > 0)
        {
            SetPushConsumerThreadCount(consumer_ptr, thread_count);
        }
    }

    // set message batch max size
    Local<Value> _max_batch_size_v = Nan::Get(options, Nan::New<String>("maxBatchSize").ToLocalChecked()).ToLocalChecked();
    if(_max_batch_size_v->IsNumber())
    {
        int max_batch_size = Nan::To<int32_t>(_max_batch_size_v).FromJust();
        if(max_batch_size > 0)
        {
            SetPushConsumerMessageBatchMaxSize(consumer_ptr, max_batch_size);
        }
    }

    // set log num & single log size
    int file_num = 3;
    int64 file_size = 104857600;
    Local<Value> _log_file_num_v = Nan::Get(options, Nan::New<String>("logFileNum").ToLocalChecked()).ToLocalChecked();
    Local<Value> _log_file_size_v = Nan::Get(options, Nan::New<String>("logFileSize").ToLocalChecked()).ToLocalChecked();
    if(_log_file_num_v->IsNumber())
    {
        file_num = _log_file_num_v->Int32Value();
    }
    if(_log_file_size_v->IsNumber())
    {
        file_size = _log_file_size_v->Int32Value();
    }
    SetPushConsumerLogFileNumAndSize(consumer_ptr, file_num, file_size);

    // set log level
    Local<Value> _log_level_v = Nan::Get(options, Nan::New<String>("logLevel").ToLocalChecked()).ToLocalChecked();
    if(_log_level_v->IsNumber())
    {
        int level = _log_level_v->Int32Value();
        SetPushConsumerLogLevel(consumer_ptr, (CLogLevel) level);
    }
}

NAN_MODULE_INIT(RocketMQPushConsumer::Init)
{
    uv_mutex_init(&_get_msg_ext_column_lock);
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RocketMQPushConsumer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "shutdown", Shutdown);
    Nan::SetPrototypeMethod(tpl, "subscribe", Subscribe);
    Nan::SetPrototypeMethod(tpl, "setListener", SetListener);
    Nan::SetPrototypeMethod(tpl, "setSessionCredentials", SetSessionCredentials);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("PushConsumer").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(RocketMQPushConsumer::New)
{
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = Context::New(isolate);

    if(!info.IsConstructCall())
    {
        const int argc = 3;
        Local<Value> argv[argc] = { info[0], info[1], info[2] };
        Local<Function> _constructor = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(_constructor->NewInstance(context, argc, argv).ToLocalChecked());
        return;
    }

    Nan::Utf8String v8_group_id(info[0]);
    Nan::Utf8String v8_instance_name(info[1]);
    string group_id = *v8_group_id;
    string instance_name = *v8_instance_name;
    Local<Object> options = Nan::To<Object>(info[2]).ToLocalChecked();
    RocketMQPushConsumer* consumer = new RocketMQPushConsumer(group_id.c_str(), info[1]->IsNull() ? NULL : instance_name.c_str());

    consumer->Wrap(info.This());

    // try to set options
    try
    {
        consumer->SetOptions(options);
    }
    catch(const runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }
    catch(const std::exception& e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RocketMQPushConsumer::Start)
{
    NAN_GET_CPUSHCONSUMER();

    Nan::Callback* callback = (info[0]->IsFunction()) ?
        new Nan::Callback(Nan::To<Function>(info[0]).ToLocalChecked()) :
        NULL;

    Nan::AsyncQueueWorker(new PushConsumerStartOrShutdownWorker(callback, consumer_ptr, PushConsumerWorkerType::START_PUSH_CONSUMER));
}

NAN_METHOD(RocketMQPushConsumer::Shutdown)
{
    NAN_GET_CPUSHCONSUMER();

    Nan::Callback* callback = (info[0]->IsFunction()) ?
        new Nan::Callback(Nan::To<Function>(info[0]).ToLocalChecked()) :
        NULL;

    Nan::AsyncQueueWorker(new PushConsumerStartOrShutdownWorker(callback, consumer_ptr, PushConsumerWorkerType::SHUTDOWN_PUSH_CONSUMER));
}

NAN_METHOD(RocketMQPushConsumer::Subscribe)
{
    NAN_GET_CPUSHCONSUMER();

    Nan::Utf8String v8_topic(info[0]);
    Nan::Utf8String v8_expression(info[1]);
    string topic = *v8_topic;
    string expression = *v8_expression;

    int ret;
    try
    {
        ret = ::Subscribe(consumer_ptr, topic.c_str(), expression.c_str());
    }
    catch(const runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }
    catch(const std::exception& e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(RocketMQPushConsumer::SetListener)
{
    RocketMQPushConsumer* consumer = ObjectWrap::Unwrap<RocketMQPushConsumer>(info.Holder());
    if(!consumer->listener_func.IsEmpty())
    {
        consumer->listener_func.Reset();
    }

    consumer->listener_func.Reset(Nan::To<Function>(info[0]).ToLocalChecked());
}

NAN_METHOD(RocketMQPushConsumer::SetSessionCredentials)
{
    NAN_GET_CPUSHCONSUMER();

    Nan::Utf8String access_key(info[0]);
    Nan::Utf8String secret_key(info[1]);
    Nan::Utf8String ons_channel(info[2]);

    int ret;
    try
    {
        ret = SetPushConsumerSessionCredentials(consumer_ptr, *access_key, *secret_key, *ons_channel);
    }
    catch(const runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }
    catch(const std::exception& e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(ret);
}

string RocketMQPushConsumer::GetMessageColumn(char* name, CMessageExt* msg)
{
    const char* orig = NULL;

    uv_mutex_lock(&_get_msg_ext_column_lock);
    switch(name[0])
    {
    // topic / tags
    case 't':
        orig = name[1] == 'o' ? GetMessageTopic(msg) : GetMessageTags(msg);
        break;

    // keys
    case 'k':
        orig = GetMessageKeys(msg);
        break;

    // body
    case 'b':
        orig = GetMessageBody(msg);
        break;

    // msgId
    case 'm':
        orig = GetMessageId(msg);
        break;

    default:
        orig = NULL;
        break;
    }

    uv_mutex_unlock(&_get_msg_ext_column_lock);

    if(!orig) return "";
    return orig;
}

void close_async_done(uv_handle_t* handle)
{
    free(handle);
}

void RocketMQPushConsumer::HandleMessageInEventLoop(uv_async_t* async)
{
    Nan::HandleScope scope;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    MessageHandlerParam* param = (MessageHandlerParam*)(async->data);
    RocketMQPushConsumer* consumer = param->consumer;
    ConsumerAckInner* ack_inner = param->ack;
    CMessageExt* msg = param->msg;

    // create the JavaScript ack object and then set inner ack object
    Local<Function> cons = Nan::New<Function>(ConsumerAck::GetConstructor());
    Local<Object> ack_obj = cons->NewInstance(context, 0, 0).ToLocalChecked();
    ConsumerAck* ack = ObjectWrap::Unwrap<ConsumerAck>(ack_obj);
    ack->SetInner(ack_inner);

    // TODO: const char *GetMessageProperty(CMessageExt *msgExt, const char *key);
    Local<Object> result = Nan::New<Object>();
    for(int i = 0; i < 5; i++)
    {
        Nan::Set(
            result,
            Nan::New(message_handler_param_keys[i]).ToLocalChecked(),
            Nan::New(RocketMQPushConsumer::GetMessageColumn(message_handler_param_keys[i], msg)).ToLocalChecked());
    }

    Local<Value> argv[2] = {
        result,
        ack_obj
    };
    Nan::Callback* callback = consumer->GetListenerFunction();
    callback->Call(2, argv);

    uv_close((uv_handle_t*)async, close_async_done);
}

int RocketMQPushConsumer::OnMessage(CPushConsumer* consumer_ptr, CMessageExt* msg_ext)
{
    RocketMQPushConsumer* consumer = _push_consumer_map[consumer_ptr];
    if (!consumer)
    {
        // TODO: error handle
        return CConsumeStatus::E_RECONSUME_LATER;
    }

    ConsumerAckInner ack_inner;

    // create async parameter
    MessageHandlerParam param;
    param.consumer = consumer;
    param.ack = &ack_inner;
    param.msg = msg_ext;

    // create a new async handler and bind with `RocketMQPushConsumer::HandleMessageInEventLoop`
    uv_async_t* async = (uv_async_t*)malloc(sizeof(uv_async_t));
    uv_async_init(uv_default_loop(), async, RocketMQPushConsumer::HandleMessageInEventLoop);
    async->data = (void*)(&param);

    // send async handler
    uv_async_send(async);

    // wait for result
    CConsumeStatus status = ack_inner.WaitResult();

    return status;
}

}
