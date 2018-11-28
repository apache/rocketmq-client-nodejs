"use strict";

require("./env_init");

const assert = require("assert");
const EventEmitter = require("events").EventEmitter;

const binding = require("../build/Release/rocketmq");

const START_OR_SHUTDOWN = Symbol("RocketMQPushConsumer#startOrShutdown");

const DEFAULT_OPTIONS = {};

let producerRef = 0;
let timer;

class RocketMQPushConsumer extends EventEmitter {
    constructor(groupId, options) {
        super();

        options = Object.assign({}, DEFAULT_OPTIONS, options || {});
        this.core = new binding.PushConsumer(groupId, options);
        this.core.setListener(this.emit.bind(this, "message"));
    }

    [START_OR_SHUTDOWN](method, callback) {
        let promise;
        let resolve;
        let reject;
        if(!callback) {
            promise = new Promise((_resolve, _reject) => {
                resolve = _resolve;
                reject = _reject;
            });
        } else {
            resolve = reject = callback;
        }

        this.core[method]((err, ret) => {
            if(method === "shutdown") {
                producerRef--;
                if(!producerRef) clearInterval(timer);
            }

            if(err) return reject(err);

            if(method === "start") {
                if(!producerRef) timer = setInterval(function() {}, 24 * 3600 * 1000);
                producerRef++;
            }

            return callback ? resolve(undefined, ret) : resolve(ret);
        });

        if(!callback) return promise;
    }

    start(callback) {
        return this[START_OR_SHUTDOWN]("start", callback);
    }

    shutdown(callback) {
        return this[START_OR_SHUTDOWN]("shutdown", callback);
    }

    subscribe(topic, expression) {
        assert(topic && typeof topic === "string");
        assert(!expression || expression && typeof expression === "string");
        return this.core.subscribe(topic, expression || "");
    }
}

module.exports = RocketMQPushConsumer;
