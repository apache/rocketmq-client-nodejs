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

const co = require("co");

const common = require("./common");
const PushConsumer = require("../").PushConsumer;

co(function *() {
    const msgs = [];
    const consumer = new PushConsumer("testGroup", {
        nameServer: common.nameServer,
        logFileNum: 5,
        logFileSize: 1048576000,
        logLevel: "debug"
    });

    consumer.subscribe("test", "*");
    consumer.on("message", function(msg, ack) {
        msgs.push(msg);
        ack.done();

        // console.log(msg);
        // return;

        if(msgs.length === common.messageCount) {
            msgs.sort(function(a, b) {
                return a.body < b.body ? -1 : 1;
            });

            console.log(msgs);

            for(let i = 0; i < msgs.length; i++) {
                assert.deepStrictEqual(msgs[i], {
                    topic: "test",
                    tags: "bar",
                    keys: "foo",
                    body: `baz ${i}`,
                    msgId: msgs[i].msgId
                });
            }

            console.time("consumer end");
            consumer.shutdown().then(() => {
                console.timeEnd("consumer end");
                process.exit(0);
            }).catch(e => {
                console.error(e);
                process.exit(4);
            });
        }
    });

    console.time("consumer start");
    yield consumer.start();
    console.timeEnd("consumer start");
});
