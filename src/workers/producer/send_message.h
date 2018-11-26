#ifndef __ROCKETMQ_SEND_MESSAGE_H__
#define __ROCKETMQ_SEND_MESSAGE_H__

#include <nan.h>
#include <CProducer.h>

namespace __node_rocketmq__ {

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
        catch(runtime_error e)
        {
            SetErrorMessage(e.what());
        }
        catch(exception& e)
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
