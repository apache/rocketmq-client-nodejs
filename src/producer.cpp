#include "producer.h"
#include "workers/send_message.h"

namespace __node_rocketmq__ {

#define NAN_GET_CPRODUCER() \
    RocketMQProducer* _v8_producer = ObjectWrap::Unwrap<RocketMQProducer>(info.Holder()); \
    CProducer* producer_ptr = _v8_producer->GetProducer();

Nan::Persistent<Function> RocketMQProducer::constructor;

RocketMQProducer::RocketMQProducer(string group_id)
{
    producer_ptr = CreateProducer(group_id.c_str());
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
}

NAN_MODULE_INIT(RocketMQProducer::Init)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RocketMQProducer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "shutdown", Shutdown);
    Nan::SetPrototypeMethod(tpl, "send", Send);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("Producer").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(RocketMQProducer::New)
{
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = Context::New(isolate);

    if(!info.IsConstructCall())
    {
        const int argc = 2;
        Local<Value> argv[argc] = { info[0], info[1] };
        Local<Function> _constructor = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(_constructor->NewInstance(context, argc, argv).ToLocalChecked());
        return;
    }

    Nan::Utf8String group_id(info[0]);
    Local<Object> options = Nan::To<Object>(info[1]).ToLocalChecked();
    RocketMQProducer* producer = new RocketMQProducer(*group_id);

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

    int ret;
    try
    {
        ret = StartProducer(producer_ptr);
    }
    catch (runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(RocketMQProducer::Shutdown)
{
    NAN_GET_CPRODUCER();

    int ret;
    try
    {
        ret = ShutdownProducer(producer_ptr);
    }
    catch (runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
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
    Nan::AsyncQueueWorker(new SendMessageWorker(callback, producer, msg));
}

}