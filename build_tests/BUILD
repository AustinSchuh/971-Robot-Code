load("@com_google_protobuf//:protobuf.bzl", "cc_proto_library")
load("@com_github_google_flatbuffers//:build_defs.bzl", "flatbuffer_py_library")
load("@io_bazel_rules_go//go:def.bzl", "go_binary")

cc_test(
    name = "gflags_build_test",
    size = "small",
    srcs = [
        "gflags.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "@com_github_gflags_gflags//:gflags",
    ],
)

cc_binary(
    name = "tcmalloc_build_test_binary",
    srcs = [
        "tcmalloc.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

sh_test(
    name = "tcmalloc_build_test",
    size = "small",
    srcs = [
        "tcmalloc_test.sh",
    ],
    data = [
        ":tcmalloc_build_test_binary",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

cc_proto_library(
    name = "proto_build_test_library",
    srcs = ["proto.proto"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":proto_build_test_library_base",
        "@com_google_protobuf//:cc_wkt_protos",
    ],
)

cc_proto_library(
    name = "proto_build_test_library_base",
    srcs = ["proto_base.proto"],
    target_compatible_with = ["@platforms//os:linux"],
)

cc_test(
    name = "proto_build_test",
    size = "small",
    srcs = [
        "proto.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":proto_build_test_library",
        "//aos/testing:googletest",
    ],
)

flatbuffer_py_library(
    name = "test_python_fbs",
    srcs = ["test.fbs"],
    namespace = "aos.examples",
    tables = [
        "Foo",
        "Bar",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)

py_test(
    name = "python_fbs",
    srcs = ["python_fbs.py"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [":test_python_fbs"],
)

py_test(
    name = "python3_opencv",
    srcs = ["python_opencv.py"],
    main = "python_opencv.py",
    target_compatible_with = ["@platforms//os:linux"],
    deps = ["@opencv_contrib_nonfree_amd64//:python_opencv"],
)

py_test(
    name = "python_jinja2",
    srcs = ["python_jinja2.py"],
    srcs_version = "PY2AND3",
    target_compatible_with = ["@platforms//os:linux"],
    deps = ["@python_jinja2"],
)

go_binary(
    name = "hello_go",
    srcs = ["hello.go"],
    target_compatible_with = ["@platforms//cpu:x86_64"],
)
