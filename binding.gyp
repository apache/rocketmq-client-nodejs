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
            "../deps/rocketmq/lib/librocketmq.so"
          ],
          "cflags_cc!": [ "-fno-exceptions", "-pthread", "-Wl,--no-as-needed", "-ldl" ],
          "cflags_cc": [ "-Wno-ignored-qualifiers" ],
          "cflags": [ "-std=c++11", "-g" ]
        }]
      ]
    }
  ]
}
