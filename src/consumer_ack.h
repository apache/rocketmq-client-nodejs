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
#ifndef __ROCKETMQ_CONSUMER_ACK_H__
#define __ROCKETMQ_CONSUMER_ACK_H__

#include <future>

#include <napi.h>

namespace __node_rocketmq__ {

class ConsumerAck : public Napi::ObjectWrap<ConsumerAck> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(Napi::Env env);

  ConsumerAck(const Napi::CallbackInfo& info);

  void SetPromise(std::promise<bool>&& promise);

  void Done(bool ack);
  void Done(std::exception_ptr exception);

 private:
  Napi::Value Done(const Napi::CallbackInfo& info);

 private:
  std::promise<bool> promise_;
};

}  // namespace __node_rocketmq__

#endif
