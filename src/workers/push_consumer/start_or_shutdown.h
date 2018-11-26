#ifndef __ROCKETMQ_PUSH_CONSUMER_START_OR_SHUTDOWN_H__
#define __ROCKETMQ_PUSH_CONSUMER_START_OR_SHUTDOWN_H__

#include <nan.h>
#include <CPushConsumer.h>

namespace __node_rocketmq__ {

enum PushConsumerWorkerType {
    START_PUSH_CONSUMER = 0,
    SHUTDOWN_PUSH_CONSUMER
};

class PushConsumerStartOrShutdownWorker : public Nan::AsyncWorker {
public:
    PushConsumerStartOrShutdownWorker(Nan::Callback* callback, CPushConsumer* consumer_ptr, PushConsumerWorkerType type) :
        AsyncWorker(callback),
        consumer_ptr(consumer_ptr),
        ret(0),
        type(type)
    {
    }

    ~PushConsumerStartOrShutdownWorker()
    {
    }

    void Execute()
    {
        try
        {
            switch(type) {
            case START_PUSH_CONSUMER:
                ret = StartPushConsumer(consumer_ptr); break;
            case SHUTDOWN_PUSH_CONSUMER:
                ret = ShutdownPushConsumer(consumer_ptr); break;
            default: break;
            }
        }
        catch (runtime_error e)
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
            Nan::New<v8::Number>((int)ret),
        };
        callback->Call(2, argv);
    }

private:
    CPushConsumer* consumer_ptr;
    int ret;
    PushConsumerWorkerType type;
};

}

#endif
