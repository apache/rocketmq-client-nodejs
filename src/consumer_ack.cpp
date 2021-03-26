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
#include "consumer_ack.h"
#include <exception>
#include "napi.h"

namespace __node_rocketmq__ {

Napi::Object ConsumerAck::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(
      env, "ConsumerAck", {InstanceMethod<&ConsumerAck::Done>("done")});

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData<Napi::FunctionReference>(constructor);

  exports.Set("ConsumerAck", func);
  return exports;
}

Napi::Object ConsumerAck::NewInstance(Napi::Env env) {
  Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New({});
  return obj;
}

ConsumerAck::ConsumerAck(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<ConsumerAck>(info) {}

void ConsumerAck::SetPromise(std::promise<bool>&& promise) {
  promise_ = std::move(promise);
}

void ConsumerAck::Done(bool ack) {
  promise_.set_value(ack);
}

void ConsumerAck::Done(std::exception_ptr exception) {
  promise_.set_exception(exception);
}

Napi::Value ConsumerAck::Done(const Napi::CallbackInfo& info) {
  if (info.Length() >= 1) {
    Napi::Value ack = info[0];
    if (ack.IsBoolean() && !ack.ToBoolean()) {
      Done(false);
    }
  }
  Done(true);
  return info.Env().Undefined();
}

}  // namespace __node_rocketmq__
