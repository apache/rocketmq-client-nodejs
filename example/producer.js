const Producer = require("../lib/producer");
process.stdin.resume();

const producer = new Producer("producerName", {
    nameServer: "127.0.0.1:9876"
});

producer.start();
producer.send("test", "baz", {
    keys: "foo",
    tags: "bar",
}, function(err, result) {
    console.log(err, result);
    producer.shutdown();
});
