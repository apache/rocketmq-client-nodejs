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
            "<(module_root_dir)/deps/rocketmq/lib/librocketmq.a"
          ],
          "cflags_cc!": [ "-fno-exceptions", "-pthread", "-Wl,--no-as-needed", "-ldl" ],
          "cflags_cc": [ "-Wno-ignored-qualifiers" ],
          "cflags": [ "-std=c++11", "-g" ]
        }],
        ["OS==\"win\"", {
          "libraries": [
            "<(module_root_dir)/deps/rocketmq/lib/rocketmq-client-cpp.lib"
          ],
          "copies": [
            {
              "destination": "<(module_root_dir)/build/Release/",
              "files": [ "<(module_root_dir)/deps/rocketmq/lib/rocketmq-client-cpp.dll" ]
            }
          ]
        }]
      ]
    }
  ]
}
