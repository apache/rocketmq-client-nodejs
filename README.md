# RocketMQ Client for Node.js

It still under development.

## Current Preparation for Development

+ Linux or VM (macOS and Windows are not supported yet)
+ [Node.js](https://nodejs.org/en/)
+ [node-gyp](https://github.com/nodejs/node-gyp)

### Steps

1. Clone this repo: `$ git clone git@github.com:apache/rocketmq-client-nodejs.git`;
2. Install dependencies and build C++ source code: `$ npm install`;
3. Run `$ node-gyp rebuild` every time you want to rebuild the code.

> Now we are using  the temporary `librocketmq.a` which is compiled by @Jonnxu
> for developing.
>
> After all we may use a Apache Hosted Mirror of static link libraries for all
> platforms.

## Commit Message Guide

We use
[AngularJS's commit message convention](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#-git-commit-guidelines)
for commit message.

E.g.

```
chore(package): ...
docs(README.md): ...
feat(...): ...
fix(...): ...
refactor(...): ...
```

## References

+ [C++ Addons | Node.js](https://nodejs.org/dist/latest-v10.x/docs/api/addons.html)
+ [NAN](https://github.com/nodejs/nan)
+ [Node.js: Let's Write a Dozen of C++ Add-ons](https://book.douban.com/subject/30247892/)
