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
