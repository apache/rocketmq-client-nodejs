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
#ifndef __ROCKETMQ_CONSUMER_ACK_H__
#define __ROCKETMQ_CONSUMER_ACK_H__

#include "consumer_ack_inner.h"
#include <nan.h>

namespace __node_rocketmq__ {

using v8::Context;
using v8::Function;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

class ConsumerAck : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);

private:
    explicit ConsumerAck();
    ~ConsumerAck();

    static NAN_METHOD(New);
    static NAN_METHOD(Done);

    void Ack(CConsumeStatus status);

    static Nan::Persistent<v8::Function> constructor;

public:
    void SetInner(ConsumerAckInner* _inner)
    {
        inner = _inner;
    }

    static Nan::Persistent<v8::Function>& GetConstructor()
    {
        return constructor;
    }

private:
    ConsumerAckInner* inner;
};

}

#endif
