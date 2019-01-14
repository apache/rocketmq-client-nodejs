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
#include "consumer_ack.h"

namespace __node_rocketmq__ {

Nan::Persistent<Function> ConsumerAck::constructor;

ConsumerAck::ConsumerAck() :
    inner(NULL)
{
}

ConsumerAck::~ConsumerAck()
{
    inner = NULL;
}

NAN_MODULE_INIT(ConsumerAck::Init)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ConsumerAck").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "done", Done);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("ConsumerAck").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(ConsumerAck::New)
{
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = Context::New(isolate);

    if(!info.IsConstructCall())
    {
        Local<Function> _constructor = Nan::New<Function>(constructor);
        info.GetReturnValue().Set(_constructor->NewInstance(context, 0, 0).ToLocalChecked());
        return;
    }

    ConsumerAck* producer = new ConsumerAck();
    producer->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(ConsumerAck::Done)
{
    ConsumerAck* ack = ObjectWrap::Unwrap<ConsumerAck>(info.Holder());
    bool succ = true;

    if(info.Length() >= 1)
    {
        succ = (info[0]->IsUndefined() || info[0]->IsNull() || Nan::To<bool>(info[0]).FromJust());
    }

    // call inner ack's `Ack` function to emit the true `Acker`'s `Ack` function
    // and finish waiting of consume thread
    CConsumeStatus status = succ ?
        CConsumeStatus::E_CONSUME_SUCCESS :
        CConsumeStatus::E_RECONSUME_LATER;
    ack->Ack(status);
}

void ConsumerAck::Ack(CConsumeStatus status)
{
    if(inner)
    {
        // call inner ack in the main event loop
        inner->Ack(status);
        inner = NULL;
    }
}

}
