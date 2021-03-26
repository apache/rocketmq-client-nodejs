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
#include <napi.h>

#include "consumer_ack.h"
#include "producer.h"
#include "push_consumer.h"

namespace __node_rocketmq__ {

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  RocketMQProducer::Init(env, exports);
  RocketMQPushConsumer::Init(env, exports);
  ConsumerAck::Init(env, exports);
  return exports;
}

NODE_API_MODULE(rocketmq, Init)

}  // namespace __node_rocketmq__
