#include <nan.h>

#include "producer.h"
#include "push_consumer.h"
#include "consumer_ack.h"

namespace __node_rocketmq__ {

NAN_MODULE_INIT(Init)
{
    RocketMQProducer::Init(target);
    RocketMQPushConsumer::Init(target);
    ConsumerAck::Init(target);
}

NODE_MODULE(rocketmq, Init)

}