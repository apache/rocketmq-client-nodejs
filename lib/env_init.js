"use strict";

const os = require("os");
const path = require("path");

const common = require("./common");

const binding = common.requireBinding("rocketmq");

switch(os.platform()) {
case "darwin":
    process.env.EVENT_NOKQUEUE = "1";
    binding.macosDLOpen(path.join(__dirname, "../deps/lib/librocketmq.dylib"));
    break;
default: break;
}
