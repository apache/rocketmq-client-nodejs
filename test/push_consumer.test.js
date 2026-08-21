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
"use strict";

const assert = require("assert");

const common = require("../lib/common");
const originalRequireBinding = common.requireBinding;
const constructedOptions = [];
const credentialsCalls = [];

class PushConsumerBinding {
    constructor(groupId, instanceName, options) {
        constructedOptions.push(options);
    }

    setListener() {}

    setSessionCredentials(accessKey, secretKey, onsChannel) {
        credentialsCalls.push({ accessKey, secretKey, onsChannel });
        return 0;
    }
}

common.requireBinding = function() {
    return {
        macosDLOpen() {},
        PushConsumer: PushConsumerBinding
    };
};

const PushConsumer = require("../lib/push_consumer");

const defaultConsumer = new PushConsumer("default-model");
assert(defaultConsumer);
assert.strictEqual(constructedOptions.pop().messageModel, 1);

const broadcastConsumer = new PushConsumer("broadcast-model", {
    messageModel: "BROADCASTING"
});
assert(broadcastConsumer);
assert.strictEqual(constructedOptions.pop().messageModel, 0);

const caseInsensitiveConsumer = new PushConsumer("case-insensitive-model", {
    messageModel: "clustering"
});
assert(caseInsensitiveConsumer);
assert.strictEqual(constructedOptions.pop().messageModel, 1);

assert.throws(() => new PushConsumer("invalid-model", {
    messageModel: "INVALID"
}), /messageModel must be BROADCASTING or CLUSTERING/);

assert.strictEqual(broadcastConsumer.setSessionCredentials("test-ak", "test-sk", "ALIYUN"), true);
assert.deepStrictEqual(credentialsCalls.pop(), {
    accessKey: "test-ak",
    secretKey: "test-sk",
    onsChannel: "ALIYUN"
});

common.requireBinding = originalRequireBinding;
