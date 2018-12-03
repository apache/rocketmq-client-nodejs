"use strict";

require("./env_init");

const assert = require("assert");

const common = require("./common");

const binding = common.requireBinding("rocketmq");

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
const START_STATUS = {
    STOPPED: 0,
    STARTED: 1,
    STOPPING: 2,
    STARTING: 3
};
const OPTIONS_LOG_LEVEL = {
    FATAL: 1,
    ERROR: 2,
    WARN: 3,
    INFO: 4,
    DEBUG: 5,
    TRACE: 6,
    NUM: 7
};

let producerRef = 0;
let timer;

class RocketMQProducer {
    /**
     * RocketMQ Producer constructor
     * @param {string} groupId the group id
     * @param {string} [instanceName] the instance name
     * @param {object} options the options
     */
    constructor(groupId, instanceName, options) {
        if(typeof instanceName !== "string" && !options) {
            options = instanceName;
            instanceName = null;
        }

        options = Object.assign({}, DEFAULT_OPTIONS, options || {});
        if(options.logLevel && typeof options.logLevel === "string") {
            options.logLevel = OPTIONS_LOG_LEVEL[options.logLevel.toUpperCase()] || OPTIONS_LOG_LEVEL.INFO;
        }
        this.core = new binding.Producer(groupId, instanceName, options);
        this.status = START_STATUS.STOPPED;
    }

    setSessionCredentials(accessKey, secretKey, onsChannel) {
        assert(typeof accessKey === "string");
        assert(typeof secretKey === "string");
        assert(typeof onsChannel === "string");
        return !this.core.setSessionCredentials(accessKey, secretKey, onsChannel);
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
            if(err) return reject(err);

            if(method === "start") {
                this.status = START_STATUS.STARTED;
                if(!producerRef) timer = setInterval(function() {}, 24 * 3600 * 1000);
                producerRef++;
            } else {
                this.status = START_STATUS.STOPPED;
                producerRef--;
                if(!producerRef) clearInterval(timer);
            }

            return callback ? resolve(undefined, ret) : resolve(ret);
        });

        if(!callback) return promise;
    }

    start(callback) {
        assert(this.status === START_STATUS.STOPPED);
        this.status = START_STATUS.STARTING;
        return this[START_OR_SHUTDOWN]("start", callback);
    }

    shutdown(callback) {
        assert(this.status === START_STATUS.STARTED);
        this.status = START_STATUS.STOPPING;
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
