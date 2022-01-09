#!/usr/bin/env python3

from setuptools import setup, Extension
import sys

def build(args):
    setup(
        name = "mesh_generator",
        version = "1.0",
        ext_modules = [Extension("mesh_generator", ["mesh_generator_main.cpp"])],
        script_args = args
    )

if __name__ == "__main__":
    build(sys.argv[1:])