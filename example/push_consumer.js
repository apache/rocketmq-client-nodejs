const PushConsumer = require("../lib/push_consumer");
process.stdin.resume();

const consumer = new PushConsumer("producerName", {
    nameServer: "127.0.0.1:9876"
});
consumer.subscribe("test");
consumer.on("message", function(msg, ack) {
    console.log(msg, ack, "<<<");
    ack.done();
});

consumer.start();
