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

const assert = require("assert");
const childProcess = require("child_process");
const readline = require("readline");

const { Producer, PushConsumer } = require("..");

const EVENT_PREFIX = "BROADCAST_E2E ";
const CHILD_TIMEOUT = 90000;

function requiredEnv(name) {
    const value = process.env[name];
    assert(value, `${name} is required`);
    return value;
}

function emitEvent(event) {
    process.stdout.write(`${EVENT_PREFIX}${JSON.stringify(event)}\n`);
}

function errorText(error) {
    return error && error.stack || String(error);
}

function delay(timeout) {
    return new Promise(resolve => setTimeout(resolve, timeout));
}

async function runConsumer() {
    const endpoint = requiredEnv("RMQ_ENDPOINT");
    const topic = requiredEnv("RMQ_TOPIC");
    const group = requiredEnv("RMQ_GROUP");
    const accessKey = requiredEnv("RMQ_ACCESS_KEY");
    const secretKey = requiredEnv("RMQ_SECRET_KEY");
    const runId = requiredEnv("BROADCAST_E2E_RUN_ID");
    const label = requiredEnv("BROADCAST_E2E_LABEL");
    const expectedCount = Number(requiredEnv("BROADCAST_E2E_COUNT"));
    const received = new Set();
    let started = false;
    let finishing = false;

    const consumer = new PushConsumer(group, `${runId}-${label}`, {
        nameServer: endpoint,
        threadCount: 1,
        messageModel: "BROADCASTING",
        logLevel: "warn"
    });
    consumer.setSessionCredentials(accessKey, secretKey, "ALIYUN");
    consumer.subscribe(topic, "*");

    async function finish() {
        if(finishing) return;
        finishing = true;
        emitEvent({
            type: "result",
            label,
            messages: Array.from(received).sort()
        });
        if(started) await consumer.shutdown();
    }

    consumer.on("message", (message, ack) => {
        ack.done();
        if(message.body.indexOf(`${runId}:`) !== 0) return;
        received.add(message.body);
        if(received.size === expectedCount) {
            finish().then(() => process.exit(0)).catch(error => {
                emitEvent({ type: "error", label, error: errorText(error) });
                process.exit(1);
            });
        }
    });

    await consumer.start();
    started = true;
    emitEvent({ type: "ready", label });

    setTimeout(() => {
        emitEvent({
            type: "error",
            label,
            error: `timed out after receiving ${received.size}/${expectedCount} messages`
        });
        process.exit(1);
    }, CHILD_TIMEOUT);
}

async function runProducer() {
    const endpoint = requiredEnv("RMQ_ENDPOINT");
    const topic = requiredEnv("RMQ_TOPIC");
    const accessKey = requiredEnv("RMQ_ACCESS_KEY");
    const secretKey = requiredEnv("RMQ_SECRET_KEY");
    const runId = requiredEnv("BROADCAST_E2E_RUN_ID");
    const messageCount = Number(requiredEnv("BROADCAST_E2E_COUNT"));
    let started = false;

    const producer = new Producer(`PID_${runId}`, `${runId}-producer`, {
        nameServer: endpoint,
        sendMessageTimeout: 10000,
        logLevel: "warn"
    });
    producer.setSessionCredentials(accessKey, secretKey, "ALIYUN");

    try {
        await producer.start();
        started = true;
        const messages = [];
        for(let i = 0; i < messageCount; i++) {
            const body = `${runId}:${i}`;
            const result = await producer.send(topic, body, {
                keys: `${runId}-${i}`,
                tags: "broadcast-e2e"
            });
            assert.strictEqual(result.status, Producer.SEND_RESULT.OK);
            messages.push(body);
        }
        emitEvent({ type: "sent", messages });
    } finally {
        if(started) await producer.shutdown();
    }
}

function spawnRole(role, extraEnv) {
    const child = childProcess.spawn(process.execPath, [ __filename ], {
        env: Object.assign({}, process.env, extraEnv, {
            BROADCAST_E2E_ROLE: role
        }),
        stdio: [ "ignore", "pipe", "pipe" ]
    });
    const events = [];
    const errors = [];
    const lines = readline.createInterface({ input: child.stdout });
    lines.on("line", line => {
        if(line.indexOf(EVENT_PREFIX) === 0) {
            events.push(JSON.parse(line.slice(EVENT_PREFIX.length)));
        }
    });
    child.stderr.on("data", chunk => errors.push(chunk.toString()));

    const completed = new Promise((resolve, reject) => {
        child.on("error", reject);
        child.on("exit", code => {
            if(code === 0) return resolve(events);
            const eventError = events.find(event => event.type === "error");
            reject(new Error(eventError && eventError.error || errors.join("") || `${role} exited with ${code}`));
        });
    });
    return { child, events, completed };
}

async function waitForReady(children) {
    const deadline = Date.now() + CHILD_TIMEOUT;
    while(Date.now() < deadline) {
        if(children.every(child => child.events.some(event => event.type === "ready"))) return;
        await delay(100);
    }
    throw new Error("consumers did not become ready");
}

async function runParent() {
    [ "RMQ_ENDPOINT", "RMQ_TOPIC", "RMQ_GROUP", "RMQ_ACCESS_KEY", "RMQ_SECRET_KEY" ].forEach(requiredEnv);
    const runId = `node-broadcast-${Date.now()}`;
    const messageCount = Number(process.env.BROADCAST_E2E_COUNT || 6);
    const sharedEnv = {
        BROADCAST_E2E_RUN_ID: runId,
        BROADCAST_E2E_COUNT: String(messageCount)
    };
    const consumers = [ "consumer-a", "consumer-b" ].map(label => spawnRole("consumer", Object.assign({}, sharedEnv, {
        BROADCAST_E2E_LABEL: label
    })));

    try {
        await waitForReady(consumers);
        await delay(5000);
        const producer = spawnRole("producer", sharedEnv);
        const producerEvents = await producer.completed;
        const sentEvent = producerEvents.find(event => event.type === "sent");
        assert(sentEvent, "producer did not report sent messages");

        const consumerEvents = await Promise.all(consumers.map(consumer => consumer.completed));
        const expected = sentEvent.messages.slice().sort();
        consumerEvents.forEach(events => {
            const result = events.find(event => event.type === "result");
            assert(result, "consumer did not report received messages");
            assert.deepStrictEqual(result.messages, expected);
        });
        console.log(JSON.stringify({
            messageModel: "BROADCASTING",
            sent: expected.length,
            consumerAReceived: expected.length,
            consumerBReceived: expected.length,
            verified: true
        }));
    } finally {
        consumers.forEach(consumer => {
            if(consumer.child.exitCode === null) consumer.child.kill("SIGTERM");
        });
    }
}

const role = process.env.BROADCAST_E2E_ROLE;
const task = role === "consumer" ? runConsumer() : role === "producer" ? runProducer() : runParent();
task.catch(error => {
    if(role) emitEvent({ type: "error", error: errorText(error) });
    else console.error(errorText(error));
    process.exit(1);
});
