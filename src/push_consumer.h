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
#ifndef __ROCKETMQ_PUSH_CONSUMER_H__
#define __ROCKETMQ_PUSH_CONSUMER_H__

#include <CPushConsumer.h>
#include <uv.h>
#include <nan.h>
#include <string>

namespace __node_rocketmq__ {

using v8::Context;
using v8::Function;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

class RocketMQPushConsumer : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
    static int OnMessage(CPushConsumer* consumer_ptr, CMessageExt* msg_ext);
    static std::string GetMessageColumn(char* name, CMessageExt* msg);

private:
    explicit RocketMQPushConsumer(const char* group_id, const char* instance_name);
    ~RocketMQPushConsumer();

    static NAN_METHOD(New);
    static NAN_METHOD(Start);
    static NAN_METHOD(Shutdown);
    static NAN_METHOD(Subscribe);
    static NAN_METHOD(SetListener);
    static NAN_METHOD(SetSessionCredentials);

    static Nan::Persistent<v8::Function> constructor;

    void SetOptions(Local<Object>);
    static void HandleMessageInEventLoop(uv_async_t* async);

protected:
    CPushConsumer* GetConsumer()
    {
        return consumer_ptr;
    }

    Nan::Callback* GetListenerFunction()
    {
        Nan::Callback* cb;
        cb = &listener_func;
        return cb;
    }

private:
    CPushConsumer* consumer_ptr;
    Nan::Callback listener_func;
};

}

#endif
