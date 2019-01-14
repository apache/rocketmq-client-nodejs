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
#ifndef __ROCKETMQ_SEND_MESSAGE_H__
#define __ROCKETMQ_SEND_MESSAGE_H__

#include <nan.h>
#include <CProducer.h>
#include <MQClientException.h>

namespace __node_rocketmq__ {

using namespace std;

class ProducerSendMessageWorker : public Nan::AsyncWorker {
public:
    ProducerSendMessageWorker(Nan::Callback* callback, RocketMQProducer* producer, CMessage* msg) :
        AsyncWorker(callback),
        msg(msg),
        producer(producer)
    {
    }

    ~ProducerSendMessageWorker()
    {
        DestroyMessage(msg);
    }

    void Execute()
    {
        try
        {
            SendMessageSync(producer->GetProducer(), msg, &send_ret);
        }
        catch(const runtime_error e)
        {
            SetErrorMessage(e.what());
        }
        catch(const std::exception& e)
        {
            SetErrorMessage(e.what());
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        Local<Value> argv[] = {
            Nan::Undefined(),
            Nan::New<v8::Number>((unsigned int)send_ret.sendStatus),
            Nan::New<v8::String>(send_ret.msgId).ToLocalChecked(),
            Nan::New<v8::Number>((long long)send_ret.offset)
        };
        callback->Call(4, argv);
    }

private:
    CMessage* msg;
    RocketMQProducer* producer;

    CSendResult send_ret;
};

}

#endif
