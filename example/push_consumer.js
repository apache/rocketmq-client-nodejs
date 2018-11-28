const co = require("co");

const PushConsumer = require("../lib/push_consumer");

co(async () => {
    const consumer = new PushConsumer("testGroup", {
        nameServer: "127.0.0.1:9876"
    });
    consumer.setSessionCredentials("*", "*", "*");

    consumer.subscribe("test", "*");
    consumer.on("message", function(msg, ack) {
        console.log(msg);
        ack.done();
    });

    console.time("consumer start");
    await consumer.start();
    console.timeEnd("consumer start");
});
