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
#include "producer.h"
#include "workers/producer/send_message.h"
#include "workers/producer/start_or_shutdown.h"

#include <MQClientException.h>
#include <string>
using namespace std;

namespace __node_rocketmq__ {

#define NAN_GET_CPRODUCER() \
    RocketMQProducer* _v8_producer = ObjectWrap::Unwrap<RocketMQProducer>(info.Holder()); \
    CProducer* producer_ptr = _v8_producer->GetProducer();

Nan::Persistent<Function> RocketMQProducer::constructor;

RocketMQProducer::RocketMQProducer(const char* group_id, const char* instance_name)
{
    producer_ptr = CreateProducer(group_id);
    if(instance_name)
    {
        SetProducerInstanceName(producer_ptr, instance_name);
    }
}

RocketMQProducer::~RocketMQProducer()
{
    try
    {
        ShutdownProducer(producer_ptr);
    }
    catch (...)
    {
        //
    }

    DestroyProducer(producer_ptr);
}

void RocketMQProducer::SetOptions(Local<Object> options)
{
    // set name server
    Local<Value> _name_server_v = Nan::Get(options, Nan::New<String>("nameServer").ToLocalChecked()).ToLocalChecked();
    if(_name_server_v->IsString())
    {
        Nan::Utf8String namesrv(_name_server_v);
        SetProducerNameServerAddress(producer_ptr, *namesrv);
    }

    // set group name
    Local<Value> _group_name_v = Nan::Get(options, Nan::New<String>("groupName").ToLocalChecked()).ToLocalChecked();
    if(_group_name_v->IsString())
    {
        Nan::Utf8String group_name(_group_name_v);
        SetProducerGroupName(producer_ptr, *group_name);
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
    SetProducerLogFileNumAndSize(producer_ptr, file_num, file_size);

    // set log level
    Local<Value> _log_level_v = Nan::Get(options, Nan::New<String>("logLevel").ToLocalChecked()).ToLocalChecked();
    if(_log_level_v->IsNumber())
    {
        int level = _log_level_v->Int32Value();
        SetProducerLogLevel(producer_ptr, (CLogLevel)level);
    }

    // set compress level
    Local<Value> _compress_level_v = Nan::Get(options, Nan::New<String>("compressLevel").ToLocalChecked()).ToLocalChecked();
    if(_compress_level_v->IsNumber()) {
        int level = _compress_level_v->Int32Value();
        SetProducerCompressLevel(producer_ptr, level);
    }

    // set send message timeout
    Local<Value> _send_message_timeout_v = Nan::Get(options, Nan::New<String>("sendMessageTimeout").ToLocalChecked()).ToLocalChecked();
    if(_send_message_timeout_v->IsNumber())
    {
        int timeout = _send_message_timeout_v->Int32Value();
        SetProducerSendMsgTimeout(producer_ptr, timeout);
    }

    // set max message size
    Local<Value> _max_message_size_v = Nan::Get(options, Nan::New<String>("maxMessageSize").ToLocalChecked()).ToLocalChecked();
    if(_max_message_size_v->IsNumber())
    {
        int size = _max_message_size_v->Int32Value();
        SetProducerMaxMessageSize(producer_ptr, size);
    }
}

NAN_MODULE_INIT(RocketMQProducer::Init)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RocketMQProducer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "shutdown", Shutdown);
    Nan::SetPrototypeMethod(tpl, "send", Send);
    Nan::SetPrototypeMethod(tpl, "setSessionCredentials", SetSessionCredentials);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("Producer").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(RocketMQProducer::New)
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

    Nan::Utf8String group_id(info[0]);
    Nan::Utf8String instance_name(info[1]);
    Local<Object> options = Nan::To<Object>(info[2]).ToLocalChecked();
    RocketMQProducer* producer = new RocketMQProducer(*group_id, info[1]->IsNull() ? NULL : *instance_name);

    producer->Wrap(info.This());

    // try to set options
    try
    {
        producer->SetOptions(options);
    }
    catch (runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RocketMQProducer::Start)
{
    NAN_GET_CPRODUCER();

    Nan::Callback* callback = (info[0]->IsFunction()) ?
        new Nan::Callback(Nan::To<Function>(info[0]).ToLocalChecked()) :
        NULL;

    Nan::AsyncQueueWorker(new ProducerStartOrShutdownWorker(callback, producer_ptr, ProducerWorkerType::START_PRODUCER));
}

NAN_METHOD(RocketMQProducer::Shutdown)
{
    NAN_GET_CPRODUCER();

    Nan::Callback* callback = (info[0]->IsFunction()) ?
        new Nan::Callback(Nan::To<Function>(info[0]).ToLocalChecked()) :
        NULL;

    Nan::AsyncQueueWorker(new ProducerStartOrShutdownWorker(callback, producer_ptr, ProducerWorkerType::SHUTDOWN_PRODUCER));
}

NAN_METHOD(RocketMQProducer::SetSessionCredentials)
{
    NAN_GET_CPRODUCER();

    Nan::Utf8String access_key(info[0]);
    Nan::Utf8String secret_key(info[1]);
    Nan::Utf8String ons_channel(info[2]);

    int ret;
    try
    {
        ret = SetProducerSessionCredentials(producer_ptr, *access_key, *secret_key, *ons_channel);
    }
    catch(runtime_error e)
    {
        Nan::ThrowError(e.what());
    }
    catch(std::exception& e)
    {
        Nan::ThrowError(e.what());
    }
    catch(rocketmq::MQException& e)
    {
        Nan::ThrowError(e.what());
    }
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(RocketMQProducer::Send)
{
    Nan::Utf8String topic(info[0]);
    Local<Object> options = Nan::To<Object>(info[2]).ToLocalChecked();

    CMessage* msg = CreateMessage(*topic);

    Local<Value> _tags_to_be_checked = Nan::Get(options, Nan::New<String>("tags").ToLocalChecked()).ToLocalChecked();
    Local<Value> _keys_to_be_checked = Nan::Get(options, Nan::New<String>("keys").ToLocalChecked()).ToLocalChecked();

    if(_tags_to_be_checked->IsString())
    {
        Nan::Utf8String tags(_tags_to_be_checked);
        SetMessageTags(msg, *tags);
    }

    if(_keys_to_be_checked->IsString())
    {
        Nan::Utf8String keys(_keys_to_be_checked);
        SetMessageKeys(msg, *keys);
    }

    // set message body:
    //   1. if it's a string, call `SetMessageBody`;
    //   2. if it's a buffer, call `SetByteMessageBody`.
    if(info[1]->IsString())
    {
        Nan::Utf8String body(info[1]);
        SetMessageBody(msg, *body);
    }
    else
    {
        Local<Object> node_buff_object = Nan::To<Object>(info[1]).ToLocalChecked();
        unsigned int length = node::Buffer::Length(node_buff_object);
        const char* buff = node::Buffer::Data(node_buff_object);
        SetByteMessageBody(msg, buff, length);
    }

    Nan::Callback* callback = (info[3]->IsFunction()) ?
        new Nan::Callback(Nan::To<Function>(info[3]).ToLocalChecked()) :
        NULL;

    RocketMQProducer* producer = ObjectWrap::Unwrap<RocketMQProducer>(info.Holder());
    Nan::AsyncQueueWorker(new ProducerSendMessageWorker(callback, producer, msg));
}

}
