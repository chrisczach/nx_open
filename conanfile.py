#!/usr/bin/env python

## Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

from conan.tools.files import save
from conans import ConanFile, __version__ as conan_version
from conans.model import Generator
from conans.tools import Version
from itertools import chain
from pathlib import Path


required_conan_version = ">=1.53.0"


# Conan does not provide a generator which makes it possible to easily access package folders for
# packages in both host and build contexts. This is the replacement.
def generate_conan_package_paths(conanfile):
    def toPosixPath(path):
        return path.replace("\\", "/")

    packages = {reference.ref.name: toPosixPath(package.package_folder) \
        for reference, package in chain(
            conanfile.dependencies.build.items(),
            conanfile.dependencies.host.items())}

    content = "set(CONAN_DEPENDENCIES {})\n\n".format(" ".join(name for name in packages.keys()))

    for name, path in packages.items():
        content += f"set(CONAN_{name.upper()}_ROOT \"{path}\")\n"

    save(conanfile, "conan_paths.cmake", content)


def generate_conan_package_refs(conanfile):
    refs = [package.pref.full_str() \
        for _, package in chain(
                conanfile.dependencies.build.items(),
                conanfile.dependencies.host.items())]

    save(conanfile, "conan_refs.txt", "\n".join(refs))


class NxOpenConan(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    generators = "cmake_find_package", "virtualrunenv"

    options = {
        "targetDevice": "ANY",
        "useClang": (True, False),
        "skipCustomizationPackage": (True, False),
        "customization": "ANY",
    }
    default_options = {
        "targetDevice": None,
        "useClang": False,
        "skipCustomizationPackage": False,
        "customization": "default",
    }

    ffmpeg_version_and_revision = "4.4#c466a4cd83fb41e24104060a371c688c"
    requires = (
        f"ffmpeg/{ffmpeg_version_and_revision}",
        "openssl/1.1.1q" "#a23bd98469b500b2d658a17351fa279c",
        "qt/6.4.1" "#96ca0ec284aed22726178b2e0d1d0e16",
        "roboto-fonts/1.0" "#a1d64ec2d6a2e16f8f476b2b47162123",
    )

    def configure(self):
        # The open-source Customization Package coming from Conan has the name "opensource-meta",
        # but its id (the "id" field in description.json inside the zip) is "metavms".
        self.options["customization"].customization = "opensource-meta"

    def generate(self):
        generate_conan_package_paths(self)
        generate_conan_package_refs(self)

    def build_requirements(self):
        self.build_requires("qt-host/6.4.1" "#32f7f5ac57afb4ee664383966dd2111f")

        if self.isLinux:
            # Note: For gcc-toolchain requirement see open/cmake/conan_profiles/gcc.profile.
            if self.options.useClang:
                self.build_requires("clang/15.0.2" "#0fa5a9fcbe580a0ecff8364eb948f845")
            self.build_requires("sdk-gcc/7.5" "#8e4d5d65d516a14449a95f3c314189f8")

        if self.isWindows:
            self.build_requires("wix/3.11" "#a662d89b677af4a98ac8cd2371be63b8")

        if self.haveDesktopClient or self.haveMediaserver:
            self.build_requires("doxygen/1.8.14" "#ad17490b6013c61d63e10c0e8ea4d6c9")

    def requirements(self):
        if not self.options.skipCustomizationPackage:
            self.requires("customization/1.0" "#b6c2fa68ea52e82bf8c83eb214ae18d7")

        self.requires("boost/1.78.0" "#298dce0adb40278309cc5f76fc92b47a")

        if self.isWindows or self.isLinux:
            if self.settings.arch == "x86_64":
                self.requires("cuda-toolkit/11.7" "#78798fb75c85a676d43c9d1a8af4fe18")
                self.requires("zlib/1.2.12" "#bb959a1d68d4c35d0fba4cc66f5bb25f")

        if self.isLinux:
            if self.settings.arch == "x86_64":
                self.requires("libva/2.16" "#65762acd637b7ab1d359128aca9e5119")
                self.requires("intel-media-sdk/19.4" "#860f17b9422f0baaa3c7a31163d408eb")

                self.requires("libmysqlclient/8.0.17" "#87d0d0dca416ff91ff910c66b57eab1a")
                self.requires("libpq/13.4" "#3c130555eda25ad50be3824716b0ce4d")

            if not self.isArm32:
                self.requires("os_deps_for_desktop_linux/ubuntu_bionic"
                    "#b979f9ed1c28142391346aaae5abad42")
                self.requires("legacy_os_deps_from_ubuntu_xenial/1.0"
                    "#665d5fc7371ad819e30bb2d25b529846")

        if self.haveDesktopClient:
            self.requires("hidapi/0.10.1" "#569759f2f39447fe3dbef070243585cc")

            if not self.isEdge1:
                self.requires("flite/2.2" "#069d57cbc32aa09dcbae1c79e94e48ef")
                self.requires("range-v3/0.11.0" "#8d874cb9cdd7b81806d624493b82f9c0")

        if self.isLinux or self.isWindows or self.isAndroid:
            self.requires("openal/1.19.1" "#1047ec92368ace234da430098bffa65a")

        if self.isWindows:
            self.requires("directx/JUN2010" "#ca268f1b54e3874ad43524cd81447b01")
            self.requires("intel-media-sdk-bin/2019R1" "#0a123266dd20b84b16a4cc60b752fc4b")
            self.requires("msvc-redist/14.29.30133" "#9f10aa59e4671ce0669d6181e6b0a269")
            self.requires("winsdk-redist/10.0.20348.0" "#bf8bf6131653c35ae84474f399fe5113")
            self.requires("icu-win/70.1" "#1c50009a7165b74563ba149c315737f1")

        if self.isArm32 or self.isArm64:
            self.requires("sse2neon/7bd15ea" "#d5c087ce33dbf1425b29d6435284d2c7")

        self._add_documentation_requirements()

    def _add_documentation_requirements(self):
        self.requires(f"vms_help/5.0_patch" "#8ef6477cf684f14f582906a5633460ad")
        self.requires(f"vms_quick_start_guide/5.0" "#cf0e958c5edf4045eb6bbc41ee2cba5e")

    def imports(self):
        if self.isLinux:
            self.copy("*.so*", "lib", "lib", root_package="qt")
            self.copy("QtWebEngineProcess", "libexec", "libexec", root_package="qt")

        if self.isLinux and self.settings.arch == "x86_64":
            self.copy("*.so*", "lib", "lib64/", root_package="cuda-toolkit")

        copy_packages = [
            "openssl",
            "hidapi",
            "cuda-toolkit"
        ]

        if self.isLinux:
            if self.settings.arch == "x86_64":
                copy_packages.append("libva")
                copy_packages.append("intel-media-sdk")

            if not self.isArm32:
                copy_packages.append("ffmpeg")
        else:
            if self.isWindows:
                copy_packages.append("intel-media-sdk-bin")
                copy_packages.append("directx/JUN2010")

            copy_packages.append("ffmpeg")

        if self.isLinux or self.isWindows or self.isAndroid:
            copy_packages.append("openal")

        self._copy_packages(copy_packages)

        self.copy("*.ttf", "bin", "bin", root_package="roboto-fonts")
        if "vms_help" in list(self.requires):
            self.copy("*", "bin", "bin", root_package="vms_help")

    def _copy_packages(self, packages):
        for package in packages:
            if package in self.deps_cpp_info.deps:
                if self.isLinux or self.isAndroid:
                    self.copy("*.so*", "lib", "lib", root_package=package)
                if self.isMacos:
                    self.copy("*.dylib", "lib", "lib", root_package=package)
                if self.isWindows:
                    self.copy("*.dll", "bin", "bin", root_package=package)
                    self.copy("*.pdb", "bin", "bin", root_package=package)

        self.fixLibraryPermissions()

    # Temporary workaround for conan issue 6470.
    def fixLibraryPermissions(self):
        if self.isWindows:
            return

        if self._lib_path.is_dir():
            self.run(f"chmod -R u+w {self._lib_path}")

    @property
    def isLinux(self):
        return self.settings.os == "Linux"

    @property
    def isWindows(self):
        return self.settings.os == "Windows"

    @property
    def isMacos(self):
        return self.settings.os == "Macos"

    @property
    def isAndroid(self):
        return self.settings.os == "Android"

    @property
    def isLinux(self):
        return self.settings.os == "Linux"

    @property
    def isArm32(self):
        return self.settings.arch in ("armv7", "armv7hf")

    @property
    def isArm64(self):
        return self.settings.arch == "armv8"

    @property
    def isEdge1(self):
        return self.options.targetDevice == "edge1"

    @property
    def isIos(self):
        return self.settings.os == "iOS"

    @property
    def haveMediaserver(self):
        return not (self.isAndroid or self.isIos)

    @property
    def haveDesktopClient(self):
        return self.isWindows or self.isMacos \
               or (self.isLinux and (self.settings.arch in ("x86_64", "armv8")))

    @property
    def haveMobileClient(self):
        return self.haveDesktopClient or self.isAndroid or self.isIos

    @property
    def _lib_path(self):
        return Path(self.install_folder) / "lib"
