#ifndef __ROCKETMQ_CONSUMER_ACK_INNER_H__
#define __ROCKETMQ_CONSUMER_ACK_INNER_H__

#include <uv.h>
#include <CPushConsumer.h>

namespace __node_rocketmq__ {

class ConsumerAckInner {
public:
    ConsumerAckInner();
    ~ConsumerAckInner();

    void Ack(CConsumeStatus _status);
    CConsumeStatus WaitResult();

private:
    bool acked;
    CConsumeStatus status;
    uv_mutex_t mutex;
    uv_cond_t cond;
};

}

#endif