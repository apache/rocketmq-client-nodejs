"use strict";

const os = require("os");
const path = require("path");

const binding = require("../build/Release/rocketmq");

switch (os.platform()) {
    case "darwin":
        process.env.EVENT_NOKQUEUE = "1";
        binding.macosDLOpen(path.join(__dirname, "../build/Release/librocketmq.dylib"));
        break;
    default: break;
}
