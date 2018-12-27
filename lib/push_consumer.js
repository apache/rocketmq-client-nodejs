"use strict";

require("./env_init");

const assert = require("assert");
const EventEmitter = require("events").EventEmitter;

const common = require("./common");

const binding = common.requireBinding("rocketmq");

const START_OR_SHUTDOWN = Symbol("RocketMQPushConsumer#startOrShutdown");
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

const DEFAULT_OPTIONS = {};

let producerRef = 0;
let timer;

class RocketMQPushConsumer extends EventEmitter {
    /**
     * RocketMQ PushConsumer constructor
     * @param {String} groupId the group id
     * @param {String} [instanceName] the instance name
     * @param {Object} options the options
     */
    constructor(groupId, instanceName, options) {
        super();

        if(typeof instanceName !== "string" && !options) {
            options = instanceName;
            instanceName = null;
        }

        options = Object.assign({}, DEFAULT_OPTIONS, options || {});
        if(options.logLevel && typeof options.logLevel === "string") {
            options.logLevel = OPTIONS_LOG_LEVEL[options.logLevel.toUpperCase()] || OPTIONS_LOG_LEVEL.INFO;
        }
        this.core = new binding.PushConsumer(groupId, instanceName, options);
        this.core.setListener(this.emit.bind(this, "message"));
        this.status = START_STATUS.STOPPED;
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

    /**
     * Start the push consumer
     * @param {Function} [callback] the callback function
     * @return {Promise|undefined} returns a Promise if no callback
     */
    start(callback) {
        assert(this.status === START_STATUS.STOPPED);
        this.status = START_STATUS.STARTING;
        return this[START_OR_SHUTDOWN]("start", callback);
    }

    /**
     * Shutdown the push consumer
     * @param {Function} [callback] the callback function
     * @return {Promise|undefined} returns a Promise if no callback
     */
    shutdown(callback) {
        assert(this.status === START_STATUS.STARTED);
        this.status = START_STATUS.STOPPING;
        return this[START_OR_SHUTDOWN]("shutdown", callback);
    }

    /**
     * subscribe a topic
     * @param {String} topic the topic to be subscribed
     * @param {String} [expression] the additional expression to be subscribed
     * @return {Number} the subscribe status result
     */
    subscribe(topic, expression) {
        assert(this.status === START_STATUS.STOPPED);
        assert(topic && typeof topic === "string");
        assert(!expression || expression && typeof expression === "string");
        return this.core.subscribe(topic, expression || "");
    }
}

module.exports = RocketMQPushConsumer;
