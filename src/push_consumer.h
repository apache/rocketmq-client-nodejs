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
#ifndef __ROCKETMQ_PUSH_CONSUMER_H__
#define __ROCKETMQ_PUSH_CONSUMER_H__

#include <string>

#include <napi.h>

#include <DefaultMQPushConsumer.h>

namespace __node_rocketmq__ {

class ConsumerMessageListener;

class RocketMQPushConsumer : public Napi::ObjectWrap<RocketMQPushConsumer> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  RocketMQPushConsumer(const Napi::CallbackInfo& info);
  ~RocketMQPushConsumer();

 private:
  Napi::Value SetSessionCredentials(const Napi::CallbackInfo& info);

  Napi::Value Start(const Napi::CallbackInfo& info);
  Napi::Value Shutdown(const Napi::CallbackInfo& info);

  Napi::Value Subscribe(const Napi::CallbackInfo& info);
  Napi::Value SetListener(const Napi::CallbackInfo& info);

 private:
  void SetOptions(const Napi::Object& options);

 private:
  rocketmq::DefaultMQPushConsumer consumer_;
  std::unique_ptr<ConsumerMessageListener> listener_;
};

}  // namespace __node_rocketmq__

#endif
