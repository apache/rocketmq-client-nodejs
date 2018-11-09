#ifndef __ROCKETMQ_PRODUCER_H__
#define __ROCKETMQ_PRODUCER_H__

#include <CProducer.h>
#include <nan.h>

#include <string>
using namespace std;

namespace __node_rocketmq__ {

using v8::Context;
using v8::Function;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

class RocketMQProducer : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);

public:
    CProducer* GetProducer() { return producer_ptr; }

private:
    explicit RocketMQProducer(string group_id);
    ~RocketMQProducer();

    static NAN_METHOD(New);
    static NAN_METHOD(Start);
    static NAN_METHOD(Shutdown);
    static NAN_METHOD(Send);

    static Nan::Persistent<Function> constructor;

private:
    void SetOptions(Local<Object> options);

private:
    CProducer* producer_ptr;
};

}

#endif