/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
