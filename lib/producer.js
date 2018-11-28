"use strict";

require("./env_init");

const assert = require("assert");

const binding = require("../build/Release/rocketmq");

const START_OR_SHUTDOWN = Symbol("RocketMQProducer#startOrShutdown");

const SEND_RESULT_STATUS = {
    OK: 0,
    FLUSH_DISK_TIMEOUT: 1,
    FLUSH_SLAVE_TIMEOUT: 2,
    SLAVE_NOT_AVAILABLE: 3
};
const SEND_RESULT_STATUS_STR = {
    0: "OK",
    1: "FLUSH_DISK_TIMEOUT",
    2: "FLUSH_SLAVE_TIMEOUT",
    3: "SLAVE_NOT_AVAILABLE"
};
const DEFAULT_OPTIONS = {};

let producerRef = 0;
let timer;

class RocketMQProducer {
    constructor(groupId, options) {
        options = Object.assign({}, DEFAULT_OPTIONS, options || {});
        this.core = new binding.Producer(groupId, options);
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

    send(topic, body, options, callback) {
        assert(typeof topic === "string");
        assert(typeof body === "string" || Buffer.isBuffer(body));

        if(typeof options === "function") {
            callback = options;
            options = {};
        }

        options = options || {};

        let promise;
        let resolve;
        let reject;
        if(!callback) {
            promise = new Promise((_resolve, _reject) => {
                resolve = _resolve;
                reject = _reject;
            });
        }

        this.core.send(topic, body, options, function sendMessageCallback(err, status, msgId, offset) {
            if(err) {
                if(!callback) return reject(err);
                return callback(err);
            }

            const ret = {
                status,
                statusStr: SEND_RESULT_STATUS_STR[status] || "UNKNOWN",
                msgId,
                offset
            };
            if(!callback) return resolve(ret);
            callback(undefined, ret);
        });
        if(promise) return promise;
    }
}

RocketMQProducer.SEND_RESULT = SEND_RESULT_STATUS;

module.exports = RocketMQProducer;
