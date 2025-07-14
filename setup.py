import os
import sys
import shutil
from pathlib import Path
from setuptools import setup, find_packages
from setuptools.command.build_ext import build_ext

PROJECT_NAME = "embree_wrapper"
PROJECT_DIR = Path(PROJECT_NAME)
BUILD_DIR = Path("build")
OUTPUT_DIR = Path("output")
LIB_NAME = f"{PROJECT_NAME}.so"


class CMakeBuild(build_ext):
    def run(self):
        BUILD_DIR.mkdir(exist_ok=True)
        OUTPUT_DIR.mkdir(exist_ok=True)

        self._configure_cmake()
        self._build_project()

        self._copy_output()

    def _configure_cmake(self):
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={OUTPUT_DIR.absolute()}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            "-DCMAKE_BUILD_TYPE=Release"
        ]

        os.chdir(BUILD_DIR)
        self.spawn(["cmake", ".."] + cmake_args)
        os.chdir("..")

    def _build_project(self):
        os.chdir(BUILD_DIR)
        self.spawn(["cmake", "--build", ".", "--config", "Release"])
        os.chdir("..")

    def _copy_output(self):
        target_dir = Path(self.get_ext_fullpath(PROJECT_NAME)).parent
        target_dir.mkdir(parents=True, exist_ok=True)

shutil.copy2(
    OUTPUT_DIR.resolve() / LIB_NAME,
    PROJECT_DIR.resolve() / f"temp_{LIB_NAME}"
)
os.replace(PROJECT_DIR.resolve() / f"temp_{LIB_NAME}", PROJECT_DIR.resolve() / LIB_NAME)

setup(
    name=PROJECT_NAME,
    version="0.0.1",
    packages=find_packages(),
    ext_modules=[],
    cmdclass={
        "build_ext": CMakeBuild,
    },
    zip_safe=False,
    python_requires=">=3.10",
    install_requires=["numpy>=1.24"],
    include_package_data=True,
    package_data={
        PROJECT_NAME: [
            LIB_NAME,
            "linux/embree-4.4.0.x86_64.linux/lib/*",
            "linux/embree-4.4.0.x86_64.linux/include/*",
        ]
    },
)