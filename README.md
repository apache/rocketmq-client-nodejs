# RocketMQ Client for Node.js

[![Version](http://img.shields.io/npm/v/apache-rocketmq.svg)](https://www.npmjs.com/package/apache-rocketmq)
[![Downloads](http://img.shields.io/npm/dm/apache-rocketmq.svg)](https://www.npmjs.com/package/apache-rocketmq)
[![License](https://img.shields.io/npm/l/apache-rocketmq.svg?style=flat)](https://opensource.org/licenses/https://opensource.org/licenses/Apache-2.0)
[![TravisCI](https://travis-ci.org/apache/rocketmq-client-nodejs.svg)](https://travis-ci.org/apache/rocketmq-client-nodejs)
[![Dependency](https://david-dm.org/apache/rocketmq-client-nodejs.svg)](https://david-dm.org/apache/rocketmq-client-nodejs)

This official Node.js client is a lightweight wrapper around  [rocketmq-client-cpp](https://github.com/apache/rocketmq-client-cpp), a finely tuned CPP client.


> **Notice 1:** This client is still in `dev` version. Use it cautiously in production.

> **Notice 2:** This SDK is now only support macOS and Ubuntu **14.04**. Ubuntu 16+ is not supported and CentOS is not tested yet.

## Installation

```shell
$ npm install --save apache-rocketmq
```

## Examples

You may view [example/producer.js](https://github.com/apache/rocketmq-client-nodejs/blob/master/example/producer.js) and
[example/push_consumer.js](https://github.com/apache/rocketmq-client-nodejs/blob/master/example/push_consumer.js) for quick start.

## Usage

Require this package first.

```javascript
const { Producer, PushConsumer } = require("apache-rocketmq");
```

### Producer

#### Constructor

```javascript
new Producer(groupId[, instanceName][, options]);
```

`Producer`'s constructor receives three parameters:

+ `groupId`: the group id of the producer;
+ `instanceName`: the instance name of the producer, **optional**;
+ `options`: the options object, **optional**;
  - `nameServer`: the name server of RocketMQ;
  - `groupName`: the group name of this producer;
  - `compressLevel`: the compress level (0-9) of this producer, default to `5` where `0` is fastest and `9` is most compressed;
  - `sendMessageTimeout`: send message timeout millisecond, default to `3000` and suggestion is 2000 - 3000ms;
  - `maxMessageSize`: max message size with unit (B), default to `1024 * 128` which means 128K;
  - `logFileNum`: C++ core logic log file number, default to 3 and log file path is `$HOME/logs/rocketmq-cpp`;
  - `logFileSize`: size of each C++ core logic log file with unit (B);
  - `logLevel`: C++ core logic log level in `"fatal"`, `"error"`, `"warn"`, `"info"`, `"debug"`, `"trace"` and `"num"`.

e.g.

```javascript
const { Producer } = require("apache-rocketmq");
const producer = new Producer("GROUP_ID", "INSTANCE_NAME", {
    nameServer: "127.0.0.1:9876",
});
```

#### start

```javascript
producer.start([callback]);
```

`.start` receives a callback function. If no callback passed, this function will return a Promise object.

e.g.

```javascript
producer.start(function(err) {
    if(err) {
        //
    }
});

// or

producer.start().then(() => {
    //
}).catch(err => {
    //
});
```

#### shutdown

```javascript
producer.shutdown([callback]);
```

`.shutdown` receives a callback function. If no callback passed, this function will return a Promise object.

e.g.

```javascript
producer.shutdown(function(err) {
    if(err) {
        //
    }
});

// or

producer.shutdown().then(() => {
    //
}).catch(err => {
    //
});
```

#### send

```javascript
producer.send(topic, body[, options][, callback]);
```

`.send` receives 4 parameters including a callback. If no callback passed, this function will return a Promise object.

+ `topic`: the topic string;
+ `body`: the message body string;
+ `options`: the options object, **optional**;
  - `keys`: the keys for this message;
  - `tags`: the tags for this message;
+ `callback`: the callback function, **optional**.

e.g.

```javascript
producer.send("test", `baz ${i}`, {
    keys: "foo",
    tags: "bar"
}, function(err, result) {
    if(err) {
        // ...    
    } else {
        console.log(result);

        // console example:
        //
        //  { status: 0,
        //    statusStr: 'OK',
        //    msgId: '0101007F0000367E0000309DD68B0700',
        //    offset: 0 }
    }
});
```

##### send `status` and `statusStr`

| `status` | `statusStr`           |
|----------|-----------------------|
| `0`      | `OK`                  |
| `1`      | `FLUSH_DISK_TIMEOUT`  |
| `2`      | `FLUSH_SLAVE_TIMEOUT` |
| `3`      | `SLAVE_NOT_AVAILABLE` |

### PushConsumer

#### Constructor

```javascript
new PushConsumer(groupId[, instanceName][, options]);
```

`PushConsumer`'s constructor receives three parameters:

+ `groupId`: the group id of the push consumer;
+ `instanceName`: the instance name of the push consumer, **optional**;
+ `options`: the options object, **optional**;
  - `nameServer`: the name server of RocketMQ;
  - `threadCount`: the thread number of underlying C++ logic;
  - `maxBatchSize`: message max batch size;
  - `logFileNum`: C++ core logic log file number, default to 3 and log file path is `$HOME/logs/rocketmq-cpp`;
  - `logFileSize`: size of each C++ core logic log file with unit (B);
  - `logLevel`: C++ core logic log level in `"fatal"`, `"error"`, `"warn"`, `"info"`, `"debug"`, `"trace"` and `"num"`.

e.g.

```javascript
const { PushConsumer } = require("apache-rocketmq");
const consumer = new PushConsumer("GROUP_ID", "INSTANCE_NAME", {
    nameServer: "127.0.0.1:9876",
    threadCount: 3
});
```

#### start

```javascript
consumer.start([callback]);
```

`.start` receives a callback function. If no callback passed, this function will return a Promise object.

e.g.

```javascript
consumer.start(function(err) {
    if(err) {
        //
    }
});

// or

consumer.start().then(() => {
    //
}).catch(err => {
    //
});
```

#### shutdown

```javascript
consumer.shutdown([callback]);
```

`.shutdown` receives a callback function. If no callback passed, this function will return a Promise object.

e.g.

```javascript
consumer.shutdown(function(err) {
    if(err) {
        //
    }
});

// or

consumer.shutdown().then(() => {
    //
}).catch(err => {
    //
});
```

#### subscribe

Add a subscription relationship to consumer.

```javascript
consumer.subscribe(topic[, expression]);
```

`.subscribe` receives two parameters which the second parameter is optional.

+ `topic`: The topic to be subscribed;
+ `expression`: The additional expression to be subscribed, **optional**. e.g. `*`.

#### On Message Event

If you want to receive messages from RocketMQ Server, you should add a listener for `message` event which receives 2
parameters.

```javascript
function YOUR_LISTENER(msg, ack) {
    //
}
```

+ `msg`: the message object to be consumed;
+ `ack`: the Acknowledge object, which has a `.done()` function.

`msg` object looks like:

```javascript
{ topic: 'test',
  tags: 'bar',
  keys: 'foo',
  body: 'baz 7',
  msgId: '0101007F0000367E0000339DD68B0800' }
```

You may call `ack.done()` to tell RocketMQ that you've finished your message successfully which is same as `ack.done(true)`. And you may call `ack.done(false)` to tell it that you've failed.

e.g.

```javascript
consumer.on("message", function(msg, ack) {
    console.log(msg);
    ack.done();
});
```

## Apache RocketMQ Community

+ [RocketMQ Community Projects](https://github.com/apache/rocketmq-externals)

## Contact Us

+ Mailing Lists: https://rocketmq.apache.org/about/contact/
+ Home: https://rocketmq.apache.org
+ Docs: https://rocketmq.apache.org/docs/quick-start/
+ Issues: https://github.com/apache/rocketmq-client-nodejs/issues
+ Ask: https://stackoverflow.com/questions/tagged/rocketmq
+ Slack: https://rocketmq-community.slack.com/

## How to Contribute

Contributions are warmly welcome! Be it trivial cleanup, major new feature or other suggestion. Read this [how to contribute](CONTRIBUTING.md) guide for more details.

## License

[Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html) Copyright (C) Apache Software Foundation
