"use strict";

const co = require("co");

const Producer = require("../lib/producer");

co(async () => {
    const producer = new Producer("testGroup", {
        nameServer: "127.0.0.1:9876"
    });
    producer.setSessionCredentials("*", "*", "*");

    console.time("producer start");
    try {
        await producer.start();
    } catch(e) {
        console.error(e);
        process.exit(4);
    }
    console.timeEnd("producer start");
    for(let i = 0; i < 15; i++) {
        console.time(`send ${i}`);
        try {
            const ret = await producer.send("test", "baz", {
                keys: "foo",
                tags: "bar"
            });
            console.timeEnd(`send ${i}`);
            console.log(ret);
        } catch(e) {
            console.error(e);
            process.exit(4);
        }
    }

    console.time("producer end");
    await producer.shutdown();
    console.timeEnd("producer end");
});
