#!/usr/bin/env python

## Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

from __future__ import print_function
import os
import sys
import argparse
import subprocess
import shutil
import json
import re


class QmlDeployUtil:
    def __init__(self, qt_root, qt_host_root):
        self.qt_root = os.path.abspath(qt_root)
        self.qt_host_root = os.path.abspath(qt_host_root)

        self.scanner_path = self.find_qmlimportscanner(qt_host_root)
        if not self.scanner_path:
            exit(f"qmlimportscanner is not found in {qt_host_root}")
            raise

        self.import_path = self.find_qml_import_path(qt_root)
        if not self.import_path:
            exit(f"qml import path is not found in {qt_root}")
            raise

        self.modules_path = self.find_modules_path(qt_root)
        if not self.import_path:
            exit(f"modules path is not found in {qt_root}")
            raise

    @staticmethod
    def find_qmlimportscanner(qt_root):
        for name in ["qmlimportscanner", "qmlimportscanner.exe"]:
            for subdir in ["bin", "libexec"]:
                path = os.path.join(qt_root, subdir, name)

                if os.path.exists(path):
                    return path

        return None

    @staticmethod
    def find_qml_import_path(qt_root):
        path = os.path.join(qt_root, "qml")
        return path if os.path.exists(path) else None

    @staticmethod
    def find_modules_path(qt_root):
        path = os.path.join(qt_root, "mkspecs", "modules")
        return path if os.path.exists(path) else None

    def invoke_qmlimportscanner(self, qml_root):
        command = [self.scanner_path, "-rootPath", qml_root, "-importPath", self.import_path]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)

        if not process:
            exit(f'Cannot start {" ".join(command)}')
            return

        return json.load(process.stdout)

    @staticmethod
    def has_compiled_file(file, directory_contents):
        if file.endswith(".qml") or file.endswith(".js"):
            return f"{file}c" in directory_contents
        return False

    @staticmethod
    def make_ignore_function(prefer_compiled, skip_styles):
        styles = {"Fusion", "Imagine", "Material", "Universal"}
        styles_directory = "Controls.2"

        ignored_always = shutil.ignore_patterns("*.a", "*.prl", "*.pdb", "designer")

        def ignoref(directory, contents):
            skipped = ignored_always(directory, contents)
            if skip_styles and os.fsdecode(directory).endswith(styles_directory):
                skipped = skipped.union(styles)
            if prefer_compiled:
                skipped = skipped.union(f for f in contents if QmlDeployUtil.has_compiled_file(f, contents))
            return skipped
        return ignoref

    def get_qt_imports(self, imports):
        if type(imports) is not list:
            exit("Parsed imports is not a list.")

        qt_deps = []

        for item in imports:
            path = item.get("path")
            if not path:
                continue

            path = os.path.abspath(path)
            if os.path.commonprefix([self.qt_root, path]) != self.qt_root:
                continue

            qt_deps.append(
                {
                    "path": path,
                    "plugin": item.get("plugin"),
                    "class_name": item.get("classname")
                })

        qt_deps.sort(key=lambda item: item["path"])

        result = []
        previous_path = None

        for item in qt_deps:
            path = item["path"]
            if previous_path and path == previous_path:
                continue

            result.append(item)
            previous_path = path

        return result

    def get_plugin_information(self, plugin_name):
        pri_file_name = os.path.join(self.modules_path, f"qt_plugin_{plugin_name}.pri")

        if not os.path.exists(pri_file_name):
            return

        with open(pri_file_name) as pri_file:
            pri_data = pri_file.read()

        re_prefix = "QT_PLUGIN\\." + plugin_name + "\\."

        m = re.search(f"{re_prefix}TYPE = (.+)", pri_data)
        if not m:
            return
        plugin_type = m[1]

        file_path = os.path.join(
            self.qt_root, "plugins", plugin_type, f"lib{plugin_name}.a"
        )
        if not os.path.exists(file_path):
            return

        m = re.search(f"{re_prefix}CLASS_NAME = (.+)", pri_data)
        if not m:
            return
        plugin_class = m[1]

        return {
            "name": plugin_name,
            "path": file_path,
            "class_name": plugin_class
        }

    def copy_components(self, imports, output_dir, ignore_function):
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        paths = []
        previous_path = None

        for item in imports:
            path = item["path"]
            if previous_path and path.startswith(previous_path):
                continue

            paths.append(path)
            previous_path = path + os.sep

        for path in paths:
            subdir = os.path.relpath(path, self.import_path)
            dst = os.path.join(output_dir, subdir)
            if os.path.exists(dst):
                shutil.rmtree(dst)
            shutil.copytree(
                path,
                dst,
                symlinks=True,
                ignore=ignore_function)

    def deploy(self, qml_root, output_dir, ignore_function):
        imports_dict = self.invoke_qmlimportscanner(qml_root)
        if not imports_dict:
            return

        imports = self.get_qt_imports(imports_dict)
        self.copy_components(imports, output_dir, ignore_function)

    def print_static_plugins(self, qml_root, file_name=None, additional_plugins=[]):
        imports_dict = self.invoke_qmlimportscanner(qml_root)
        if not imports_dict:
            return

        imports = self.get_qt_imports(imports_dict)

        output = open(file_name, "w") if file_name else sys.stdout
        for item in imports:
            path = item["path"]
            plugin = item["plugin"]

            if not plugin:
                continue

            name = os.path.join(path, f"lib{plugin}.a")
            if os.path.exists(name):
                output.write(name + "\n")

        for plugin_name in additional_plugins:
            if info := self.get_plugin_information(plugin_name):
                output.write(info["path"] + "\n")

        if output != sys.stdout:
            output.close()

    def generate_import_cpp(self, qml_root, file_name, additional_plugins=[]):
        imports_dict = self.invoke_qmlimportscanner(qml_root)
        if not imports_dict:
            return

        imports = self.get_qt_imports(imports_dict)

        with open(file_name, "w") as out_file:
            out_file.write("// This file is generated by qmldeploy.py\n")
            out_file.write("#include <QtPlugin>\n\n")

            for item in imports:
                if class_name := item["class_name"]:
                    out_file.write(f"Q_IMPORT_PLUGIN({class_name})\n")

            for plugin_name in additional_plugins:
                if info := self.get_plugin_information(plugin_name):
                    out_file.write(f'Q_IMPORT_PLUGIN({info["class_name"]})\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--qmlimportscanner", type=str, help="Custom qmlimportscanner binary.")
    parser.add_argument("--qml-root", type=str, required=True, help="Root QML directory.")
    parser.add_argument("--qt-root", type=str, required=True, help="Qt root directory.")
    parser.add_argument("--qt-host-root", type=str, required=True, help="Host Qt root directory.")
    parser.add_argument("-o", "--output", type=str, help="Output.")
    parser.add_argument("--print-static-plugins", action="store_true", help="Print static plugins list.")
    parser.add_argument("--generate-import-cpp", action="store_true", help="Generate a file for importing plugins.")
    parser.add_argument("--prefer-compiled", action="store_true", help="Skip base version if compiled exists")
    parser.add_argument("--skip-styles", action="store_true", help="Skip QuickControls2 styles")
    parser.add_argument("--additional-plugins", nargs="+", help="Additional plugins to process.")
    parser.add_argument("--clean", action="store_true", help="Clean output folder before copying files.")

    args = parser.parse_args()

    deploy_util = QmlDeployUtil(args.qt_root, args.qt_host_root)
    if args.qmlimportscanner:
        if not os.path.exists(args.qmlimportscanner):
            exit(f"{args.qmlimportscanner}: not found")
        deploy_util.scanner_path = args.qmlimportscanner

    if args.print_static_plugins:
        deploy_util.print_static_plugins(args.qml_root, args.output, args.additional_plugins)
    elif args.generate_import_cpp:
        deploy_util.generate_import_cpp(args.qml_root, args.output, args.additional_plugins)
    else:
        if not args.output:
            exit("Output directory is not specified.")

        if args.clean and os.path.exists(args.output):
            shutil.rmtree(args.output)

        deploy_util.deploy(
            args.qml_root,
            args.output,
            deploy_util.make_ignore_function(args.prefer_compiled, args.skip_styles))


if __name__ == "__main__":
    main()
