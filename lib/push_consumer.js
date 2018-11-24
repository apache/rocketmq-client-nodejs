"use strict";

require("./env_init");

const assert = require("assert");
const EventEmitter = require("events").EventEmitter;

const binding = require("../build/Release/rocketmq");

const DEFAULT_OPTIONS = {};

class RocketMQPushConsumer extends EventEmitter {
    constructor(groupId, options) {
        super();
        
        options = Object.assign({}, DEFAULT_OPTIONS, options || {});
        this.core = new binding.PushConsumer(groupId, options);
        this.core.setListener(this.emit.bind(this, "message"));
    }

    start() {
        // TODO: make it async
        return !this.core.start();
    }

    shutdown() {
        // TODO: make it async
        return !this.core.shutdown();
    }

    subscribe(topic, expression) {
        assert(topic && typeof topic === "string");
        assert(!expression || (expression && typeof expression === "string"));
        return this.core.subscribe(topic, expression || "");
    }
}

module.exports = RocketMQPushConsumer;
