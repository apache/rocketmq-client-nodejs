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
