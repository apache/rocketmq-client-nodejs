/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
     * @param {String} groupId the group id
     * @param {String} [instanceName] the instance name
     * @param {Object} options the options
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

    /**
     * Set session credentials (usually used in Alibaba MQ)
     * @param {String} accessKey the access key
     * @param {String} secretKey the secret key
     * @param {String} onsChannel the ons channel
     * @return {Number} the result
     */
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

    /**
     * Start the producer
     * @param {Function} [callback] the callback function
     * @return {Promise|undefined} returns a Promise if no callback
     */
    start(callback) {
        assert(this.status === START_STATUS.STOPPED);
        this.status = START_STATUS.STARTING;
        return this[START_OR_SHUTDOWN]("start", callback);
    }

    /**
     * Shutdown the producer
     * @param {Function} [callback] the callback function
     * @return {Promise|undefined} returns a Promise if no callback
     */
    shutdown(callback) {
        assert(this.status === START_STATUS.STARTED);
        this.status = START_STATUS.STOPPING;
        return this[START_OR_SHUTDOWN]("shutdown", callback);
    }

    /**
     * Send a message
     * @param {String} topic the topic
     * @param {String} body the body
     * @param {Object} [options] the options
     * @param {Function} [callback] the callback function
     * @return {Promise|undefined} returns a Promise if no callback
     */
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
