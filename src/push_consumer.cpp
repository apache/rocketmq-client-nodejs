#include <map>
#include "push_consumer.h"
#include "consumer_ack.h"

namespace __node_rocketmq__ {

struct MessageHandlerParam
{
    RocketMQPushConsumer* consumer;
    CMessageExt* msg;
    ConsumerAckInner* ack;
};

map<CPushConsumer*, RocketMQPushConsumer*> _push_consumer_map;

#define NAN_GET_CPUSHCONSUMER() \
    RocketMQPushConsumer* _v8_consumer = ObjectWrap::Unwrap<RocketMQPushConsumer>(info.Holder()); \
    CPushConsumer* consumer_ptr = _v8_consumer->GetConsumer();

Nan::Persistent<Function> RocketMQPushConsumer::constructor;

RocketMQPushConsumer::RocketMQPushConsumer(string group_id)
{
    consumer_ptr = CreatePushConsumer(group_id.c_str());
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
    catch (...)
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
}

NAN_MODULE_INIT(RocketMQPushConsumer::Init)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RocketMQPushConsumer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "shutdown", Shutdown);
    Nan::SetPrototypeMethod(tpl, "subscribe", Subscribe);
    Nan::SetPrototypeMethod(tpl, "setListener", SetListener);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("PushConsumer").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(RocketMQPushConsumer::New)
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
    RocketMQPushConsumer* consumer = new RocketMQPushConsumer(*group_id);

    consumer->Wrap(info.This());

    // try to set options
    try
    {
        consumer->SetOptions(options);
    }
    catch (runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RocketMQPushConsumer::Start)
{
    NAN_GET_CPUSHCONSUMER();

    int ret;
    try
    {
        ret = StartPushConsumer(consumer_ptr);
    }
    catch (runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(RocketMQPushConsumer::Shutdown)
{
    NAN_GET_CPUSHCONSUMER();

    int ret;
    try
    {
        ret = ShutdownPushConsumer(consumer_ptr);
    }
    catch (runtime_error e)
    {
        Nan::ThrowError(e.what());
        return;
    }

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(RocketMQPushConsumer::Subscribe)
{
    NAN_GET_CPUSHCONSUMER();

    Nan::Utf8String topic(info[0]);
    Nan::Utf8String expression(info[1]);

    int ret;
    try
    {
        ret = ::Subscribe(consumer_ptr, *topic, *expression);
    }
    catch (runtime_error e)
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

void RocketMQPushConsumer::HandleMessageInEventLoop(uv_async_t* async)
{
    Nan::HandleScope scope;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    MessageHandlerParam* param = (MessageHandlerParam*)async->data;
    CMessageExt* msg = param->msg;
    RocketMQPushConsumer* consumer = param->consumer;
    ConsumerAckInner* ack_inner = param->ack;

    // create the JavaScript ack object and then set inner ack object
    Local<Function> cons = Nan::New<Function>(ConsumerAck::GetConstructor());
    Local<Object> ack_obj = cons->NewInstance(context, 0, 0).ToLocalChecked();
    ConsumerAck* ack = ObjectWrap::Unwrap<ConsumerAck>(ack_obj);
    ack->SetInner(ack_inner);

    // create message object:
    //
    //   const char *GetMessageTopic(CMessageExt *msgExt);
    //   const char *GetMessageTags(CMessageExt *msgExt);
    //   const char *GetMessageKeys(CMessageExt *msgExt);
    //   const char *GetMessageBody(CMessageExt *msgExt);
    //   const char *GetMessageProperty(CMessageExt *msgExt, const char *key);
    //   const char *GetMessageId(CMessageExt *msgExt);
    Local<Object> result = Nan::New<Object>();
    char keys[5][8] = { "topic", "tags", "keys", "body", "id" };
    const char* values[5] = {
        GetMessageTopic(msg), GetMessageTags(msg), GetMessageKeys(msg),
        GetMessageBody(msg), GetMessageId(msg)
    };

    for(int i = 0; i < 5; i++)
    {
        Nan::Set(
            result,
            Nan::New(keys[i]).ToLocalChecked(),
            Nan::New(values[i]).ToLocalChecked());
    }

    Local<Value> argv[2] = {
        result,
        ack_obj
    };
    Nan::Callback* callback = consumer->GetListenerFunction();
    callback->Call(2, argv);
}

int RocketMQPushConsumer::OnMessage(CPushConsumer* consumer_ptr, CMessageExt* msg_ext)
{
    printf("!!!!!\n");
    RocketMQPushConsumer* consumer = _push_consumer_map[consumer_ptr];
    if (!consumer)
    {
        // TODO: error handle
        return CConsumeStatus::E_RECONSUME_LATER;
    }

    ConsumerAckInner* ack_inner = new ConsumerAckInner();

    // create a new async handler and bind with `RocketMQPushConsumer::HandleMessageInEventLoop`
    uv_async_t* async = (uv_async_t*)malloc(sizeof(uv_async_t));
    uv_async_init(uv_default_loop(), async, RocketMQPushConsumer::HandleMessageInEventLoop);

    // create async parameter
    MessageHandlerParam* param = (MessageHandlerParam*)malloc(sizeof(MessageHandlerParam));
    param->consumer = consumer;
    param->msg = msg_ext;
    param->ack = ack_inner;
    async->data = (void*)param;

    // send async handler
    uv_async_send(async);

    // wait for result
    CConsumeStatus status = ack_inner->WaitResult();

    param->consumer = NULL;
    param->msg = NULL;
    param->ack = NULL;
    async->data = NULL;

    delete ack_inner;
    free(param);
    free(async);

    return status;
}

}
