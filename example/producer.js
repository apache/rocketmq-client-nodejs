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

const co = require("co");

const common = require("./common");
const Producer = require("../").Producer;

co(function *() {
    const producer = new Producer("testGroup", {
        nameServer: common.nameServer,
        groupName: "testGroupName",
        logFileNum: 5,
        logFileSize: 1048576000,
        logLevel: "debug",
        compressLevel: 3,
        sendMessageTimeout: 5000,
        maxMessageSize: 1024 * 256
    });

    console.time("producer start");
    try {
        yield producer.start();
    } catch(e) {
        console.error(e);
        process.exit(4);
    }
    console.timeEnd("producer start");
    for(let i = 0; i < common.messageCount; i++) {
        console.time(`send ${i}`);
        try {
            const ret = yield producer.send("test", `baz ${i}`, {
                keys: "foo",
                tags: "bar"
            });
            console.timeEnd(`send ${i}`);
            console.log(ret);
        } catch(e) {
            console.error(e);
            console.error(e.stack);
            process.exit(4);
        }
    }

    console.time("producer end");
    yield producer.shutdown();
    console.timeEnd("producer end");
});
