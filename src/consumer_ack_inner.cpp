#include "consumer_ack_inner.h"

namespace __node_rocketmq__ {

ConsumerAckInner::ConsumerAckInner() :
    acked(false)
{
    uv_cond_init(&cond);
    uv_mutex_init(&mutex);
}

ConsumerAckInner::~ConsumerAckInner()
{
    uv_mutex_destroy(&mutex);
    uv_cond_destroy(&cond);
}

void ConsumerAckInner::Ack(CConsumeStatus _status)
{
    uv_mutex_lock(&mutex);
    bool _acked = acked;

    if(_acked)
    {
        uv_mutex_unlock(&mutex);
        return;
    }

    status = _status;
    acked = true;

    // tell `this->WaitResult()` to continue
    uv_cond_signal(&cond);
    uv_mutex_unlock(&mutex);
}

CConsumeStatus ConsumerAckInner::WaitResult()
{
    uv_mutex_lock(&mutex);

    // if `cond signal` send before `WaitResult()`,
    // `uv_cond_wait` will be blocked and never continue
    //
    // so we have to return result directly without `uv_cond_wait`
    if(acked)
    {
        CConsumeStatus _status = status;
        uv_mutex_unlock(&mutex);
        return _status;
    }

    // wait for `this->Ack()` and that will emit `uv_cond_signal` to let it stop wait
    uv_cond_wait(&cond, &mutex);

    CConsumeStatus _status = status;
    uv_mutex_unlock(&mutex);

    return _status;
}

}
