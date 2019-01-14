# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  "targets": [
    {
      "target_name": "rocketmq",
      "sources": [
        "src/rocketmq.cpp",
        "src/producer.cpp",
        "src/push_consumer.cpp",
        "src/consumer_ack.cpp",
        "src/consumer_ack_inner.cpp"
      ],
      "include_dirs": [
        "deps/rocketmq/include",
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        ["OS==\"linux\"", {
          "libraries": [
            "<(module_root_dir)/deps/lib/librocketmq.a"
          ],
          "cflags_cc!": [ "-fno-exceptions", "-pthread", "-Wl,--no-as-needed", "-ldl" ],
          "cflags_cc": [ "-Wno-ignored-qualifiers" ],
          "cflags": [ "-std=c++11", "-g" ]
        }],
        ["OS==\"win\"", {
          "libraries": [
            "<(module_root_dir)/deps/lib/rocketmq-client-cpp.lib"
          ],
          "copies": [
            {
              "destination": "<(module_root_dir)/build/Release/",
              "files": [ "<(module_root_dir)/deps/lib/rocketmq-client-cpp.dll" ]
            }
          ]
        }],
        ["OS==\"mac\"", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
          },
          "cflags!": [ "-fno-exceptions" ],
          "cflags_cc!": [ "-fno-exceptions", "-pthread", "-Wl,--no-as-needed", "-ldl" ],
          "cflags_cc": [ "-Wno-ignored-qualifiers" ],
          "cflags": [ "-std=c++11", "-stdlib=libc++" ]
        }]
      ]
    }
  ]
}
