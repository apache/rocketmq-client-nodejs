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
#include <nan.h>

#include "producer.h"
#include "push_consumer.h"
#include "consumer_ack.h"

namespace __node_rocketmq__ {

#if defined(__APPLE__)
uv_lib_t lib;

NAN_METHOD(DLOpen)
{
    Nan::Utf8String filename(info[0]);
    uv_dlopen(*filename, &lib);
}
#endif

NAN_MODULE_INIT(Init)
{
    RocketMQProducer::Init(target);
    RocketMQPushConsumer::Init(target);
    ConsumerAck::Init(target);

#if defined(__APPLE__)
    Nan::Set(target, Nan::New("macosDLOpen").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(DLOpen)->GetFunction());
#endif
}

NODE_MODULE(rocketmq, Init)

}
