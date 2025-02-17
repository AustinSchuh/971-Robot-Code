def _aos_downloader_impl(ctx):
    all_files = ctx.files.srcs + ctx.files.start_srcs + [ctx.outputs._startlist]
    target_files = []

    # downloader looks for : in the inputs and uses the part after the : as
    # the directory to copy to.
    for d in ctx.attr.dirs:
        target_files += [src.short_path + ":" + d.downloader_dir for src in d.downloader_srcs]

    ctx.actions.write(
        output = ctx.outputs.executable,
        is_executable = True,
        content = "\n".join([
            "#!/bin/bash",
            "set -e",
            'cd "${BASH_SOURCE[0]}.runfiles/%s"' % ctx.workspace_name,
            'for T in "$@";',
            "do",
            '  time %s --target "${T}" --type %s %s %s' % (
                ctx.executable._downloader.short_path,
                ctx.attr.target_type,
                " ".join([src.short_path for src in all_files]),
                " ".join(target_files),
            ),
            "done",
        ]),
    )

    ctx.actions.write(
        output = ctx.outputs._startlist,
        content = "\n".join([f.basename for f in ctx.files.start_srcs]) + "\n",
    )

    to_download = [ctx.outputs._startlist]
    to_download += all_files
    for d in ctx.attr.dirs:
        to_download += d.downloader_srcs

    return struct(
        runfiles = ctx.runfiles(
            files = to_download + ctx.files._downloader,
            transitive_files = ctx.attr._downloader.default_runfiles.files,
            collect_data = True,
            collect_default = True,
        ),
        files = depset([ctx.outputs.executable]),
    )

def _aos_downloader_dir_impl(ctx):
    return struct(
        downloader_dir = ctx.attr.dir,
        downloader_srcs = ctx.files.srcs,
    )

"""Creates a binary which downloads code to a robot.

Running this with `bazel run` will actually download everything.

This also generates a start_list.txt file with the names of binaries to start.

Attrs:
  srcs: The files to download. They currently all get shoved into one folder.
  dirs: A list of aos_downloader_dirs to download too.
  start_srcs: Like srcs, except they also get put into start_list.txt.
"""

aos_downloader = rule(
    attrs = {
        "_downloader": attr.label(
            executable = True,
            cfg = "host",
            default = Label("//frc971/downloader"),
        ),
        "start_srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "target_type": attr.string(
            default = "roborio",
        ),
        "dirs": attr.label_list(
            mandatory = False,
            providers = [
                "downloader_dir",
                "downloader_srcs",
            ],
        ),
    },
    executable = True,
    outputs = {
        "_startlist": "%{name}.start_list.dir/start_list.txt",
    },
    implementation = _aos_downloader_impl,
)

"""Downloads files to a specific directory.

This rule does nothing by itself. Use it by adding to the dirs attribute of an
aos_downloader rule.

Attrs:
  srcs: The files to download. They all go in the same directory.
  dir: The directory (relative to the standard download directory) to put all
       the files in.
"""

aos_downloader_dir = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "dir": attr.string(
            mandatory = True,
        ),
    },
    implementation = _aos_downloader_dir_impl,
)
