"use strict";

const co = require("co");

const Producer = require("../lib/producer");

co(async () => {
    const producer = new Producer("testGroup", {
        nameServer: "127.0.0.1:9876"
    });

    console.time("producer start");
    await producer.start();
    console.timeEnd("producer start");
    for(let i = 0; i < 15; i++) {
        console.time(`send ${i}`);
        const ret = await producer.send("test", "baz", {
            keys: "foo",
            tags: "bar"
        });
        console.timeEnd(`send ${i}`);
        console.log(ret);
    }

    console.time("producer end");
    await producer.shutdown();
    console.timeEnd("producer end");
});
