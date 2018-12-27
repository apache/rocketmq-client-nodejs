"use strict";

exports.requireBinding = function(name) {
    let mod;
    try {
        mod = require(`../build/Debug/${name}`);
    } catch(e) {
        mod = require(`../build/Release/${name}`);
    }

    return mod;
};
