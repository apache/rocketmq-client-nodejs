# RocketMQ Client for Node.js

It still under development.

## Current Preparation for Development

+ Linux or VM (macOS and Windows are not supported yet)
+ [Node.js](https://nodejs.org/en/)
+ [node-gyp](https://github.com/nodejs/node-gyp)

### Steps

1. Clone this repo: `$ git clone --recurse-submodules git@github.com:apache/rocketmq-client-nodejs.git`;
2. Install dependencies and build C++ source code: `$ npm install`;
3. Run `$ node-gyp rebuild` every time you want to rebuild the code.

> Now we are using  the temporary `librocketmq.a` which is compiled by @Jonnxu
> for developing.
>
> After all we may use a Apache Hosted Mirror of static link libraries for all
> platforms.

## Commit Message Guide

We use
[AngularJS's commit message convention](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#-git-commit-guidelines)
for commit message.

E.g.

```
chore(package): ...
docs(README.md): ...
feat(...): ...
fix(...): ...
refactor(...): ...
```

## References

+ [C++ Addons | Node.js](https://nodejs.org/dist/latest-v10.x/docs/api/addons.html)
+ [NAN](https://github.com/nodejs/nan)
+ [Node.js: Let's Write a Dozen of C++ Add-ons](https://book.douban.com/subject/30247892/)

## Usage

### Producer

```
const producer = new Producer(groupId[, instanceName][, options]);
```
options:
- nameServer: server address,
- groupName: group name,
- logFileNum: log file number, default 3, log file path: $HOME/logs/rocketmq-cpp
- logFileSize: single log file size, unit(B), Int32 type so max value is 2147483647
- logLevel: log level of C++ core logic
    - "fatal"
    - "error"
    - "warn"
    - "info"
    - "debug"
    - "trace"
    - "num"
- compressLevel: [0, 9], default 5, where 0 is fastest 9 is most compressed
- sendMessageTimeout: send message timeout millisecond, default 3000ms, suggest 2000ms - 3000ms
- maxMessageSize: max message size, unit(B), default 128K(1024 * 128)

### PushConsumer

```
const consumer = new PushConsumer(groupId[, instanceName][, options]);
```
options:
- nameServer: server address,
- logFileNum: log file number, default 3, log file path: $HOME/logs/rocketmq-cpp
- logFileSize: single log file size, unit(B), Int32 type so max value is 2147483647
- logLevel: log level of C++ core logic
    - "fatal"
    - "error"
    - "warn"
    - "info"
    - "debug"
    - "trace"
    - "num"
