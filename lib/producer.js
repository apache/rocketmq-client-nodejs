"use strict";

const assert = require("assert");

const binding = require("../build/Release/rocketmq");

const SEND_RESULT_STATUS = {
    OK: 0,
    FLUSH_DISK_TIMEOUT: 1,
    FLUSH_SLAVE_TIMEOUT: 2,
    SLAVE_NOT_AVAILABLE: 3
};
const SEND_RESULT_STATUS_STR = {
    "0": "OK",
    "1": "FLUSH_DISK_TIMEOUT",
    "2": "FLUSH_SLAVE_TIMEOUT",
    "3": "SLAVE_NOT_AVAILABLE"
};
const DEFAULT_OPTIONS = {};

class RocketMQProducer {
    constructor(groupId, options) {
        options = Object.assign({}, DEFAULT_OPTIONS, options || {});
        this.core = new binding.Producer(groupId, options);
    }

    start() {
        // TODO: make it async
        return !this.core.start();
    }

    shutdown() {
        // TODO: make it async
        return !this.core.shutdown();
    }

    send(topic, body, options, callback) {
        assert(typeof topic === "string");
        assert(typeof body === "string" || Buffer.isBuffer(body));

        if (typeof options === "function") {
            callback = options;
            options = {};
        }

        options = options || {};

        this.core.send(topic, body, options, function sendMessageCallback(err, status, msgId, offset) {
            if(!callback) return;

            if(err) {
                return callback(err);
            }

            return callback(undefined, {
                status,
                statusStr: SEND_RESULT_STATUS_STR[status] || "UNKNOWN",
                msgId,
                offset
            });
        });
    }
}

RocketMQProducer.SEND_RESULT = SEND_RESULT_STATUS;

module.exports = RocketMQProducer;
