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