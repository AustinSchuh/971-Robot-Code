package(default_visibility = ["@//debian:__pkg__"])

cc_library(
    name = "python3.7_lib",
    srcs = [
        "usr/lib/x86_64-linux-gnu/libpython3.7m.so.1.0",
    ],
    hdrs = glob(["usr/include/**/*.h"]),
    includes = [
        "usr/include/",
        "usr/include/python3.7m/",
    ],
    target_compatible_with = ["@platforms//cpu:x86_64"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "python3.7_f2py",
    srcs = [
        "usr/lib/python3/dist-packages/numpy/f2py/src/fortranobject.c",
    ],
    hdrs = [
        "usr/lib/python3/dist-packages/numpy/f2py/src/fortranobject.h",
    ],
    copts = [
        "-Wno-error",
        "-Wno-parentheses-equality",
    ],
    includes = [
        "usr/lib/python3/dist-packages/numpy/f2py/src/",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":python3.7_lib",
    ],
)

filegroup(
    name = "all_files",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

genrule(
    name = "copy_f2py",
    srcs = ["usr/bin/f2py"],
    outs = ["f2py.py"],
    cmd = "cp $< $@",
    executable = True,
)

py_binary(
    name = "f2py",
    srcs = ["f2py.py"],
    visibility = ["//visibility:public"],
)

py_library(
    name = "scipy",
    srcs = glob([
        "usr/lib/python3/dist-packages/scipy/**/*.py",
    ]),
    data = glob([
        "usr/lib/python3/dist-packages/scipy/**/*",
    ], exclude = [
        "usr/lib/python3/dist-packages/scipy/**/*.py",
    ]),
    deps = [
        ":numpy",
    ],
    visibility = ["//visibility:public"],
    imports = [
        "usr/lib/python3/dist-packages",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)

py_library(
    name = "numpy",
    srcs = glob([
        "usr/lib/python3/dist-packages/numpy/**/*.py",
    ]),
    data = glob([
        "usr/lib/python3/dist-packages/numpy/**/*",
    ], exclude = [
        "usr/lib/python3/dist-packages/numpy/**/*.py",
    ]),
    visibility = ["//visibility:public"],
    imports = [
        "usr/lib/python3/dist-packages",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)
