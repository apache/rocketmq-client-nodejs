"use strict";

const assert = require("assert");

const co = require("co");

const common = require("./common");
const PushConsumer = require("../lib/push_consumer");

co(function *() {
    const msgs = [];
    const consumer = new PushConsumer("testGroup", {
        nameServer: common.nameServer
    });

    consumer.subscribe("test", "*");
    consumer.on("message", function(msg, ack) {
        msgs.push(msg);
        ack.done();

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
