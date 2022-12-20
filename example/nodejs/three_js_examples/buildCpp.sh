#!/bin/sh

function contains {
    return $([[ $1 =~ (^|[[:space:]])"$2"($|[[:space:]]) ]] && echo 0 || echo 1)
}

arguments="$@"
debug_build=$(contains "$arguments" "debug" && echo 1 || echo 0)
clean_build=$(contains "$arguments" "clean" && echo 1 || echo 0)
gnu_build=$(contains "$arguments" "gnu" && echo 1 || echo 0)
wasm_build=$(contains "$arguments" "wasm" && echo 1 || echo 0)
test_build=$(contains "$arguments" "test" && echo 1 || echo 0)

(($wasm_build != 0)) && echo "build WASM"
(($test_build != 0)) && echo "build test"

cmake_program="cmake"
cmake_configuration=""
make_program="make"
make_configuration=""

if (($debug_build != 0)); then
    cmake_configuration="$cmake_configuration -D CMAKE_BUILD_TYPE=Debug -D ADD_DEBUG_INFO=ON" 
    echo "build with DEBUG configuration"
else
    cmake_configuration="$cmake_configuration -DCMAKE_BUILD_TYPE=MinSizeRel" 
    echo "build with RELEASE configuration"
fi
if (($clean_build != 0)); then
    make_configuration="$make_configuration -j8 clean all" 
    echo "clean all, build"
else
    make_configuration="$make_configuration -j8 " 
fi

if (($gnu_build != 0)); then
    gnu_gcc=gcc-12
    gnu_gpp=g++-12
    gnu_make=gmake
    if [ -f $(which $gnu_gcc) ]; then
        echo $(which $gnu_gcc)
        cmake_configuration="$cmake_configuration -D CMAKE_C_COMPILER=$gnu_gcc"
    fi
    if [ -f $(which $gnu_gpp) ]; then
        echo $(which $gnu_gpp)
        cmake_configuration="$cmake_configuration -D CMAKE_CXX_COMPILER=$gnu_gpp"
    fi
    if [ -f $(which $gnu_make) ]; then
        echo $(which $gnu_make)
        make_program=$gnu_make
        cmake_configuration="$cmake_configuration -D CMAKE_MAKE_PROGRAM=$gnu_make"
    fi
fi 

if (($wasm_build != 0)); then
    emscripten_tool_chain_file="/Users/$USER/source/emscripten/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" 
    if [ ! -f $emscripten_tool_chain_file ]; then
        emscripten_tool_chain_file="/Users/$USER/Dev/devtools/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" 
        if [ ! -f $emscripten_tool_chain_file ]; then
            echo "EMSCRIPTEN toolchain not found"
        fi
    fi    
    WASM_build_directory="./buildWASM"

    mkdir -p $WASM_build_directory
    $cmake_program \
        install \
        -S ./src/wasm \
        -B $WASM_build_directory \
        -G "Unix Makefiles" \
        -D CMAKE_TOOLCHAIN_FILE=$emscripten_tool_chain_file \
        $cmake_configuration

    cd $WASM_build_directory
    $make_program MeshUtility MeshUtilityJs $make_configuration
    cd ..
fi

if (($test_build != 0)); then
    test_build_directory="./buildTest"
    
    mkdir -p $test_build_directory
    $cmake_program \
        install \
        -S ./src/wasm \
        -B $test_build_directory \
        -G "Unix Makefiles" \
        $cmake_configuration

    cd $test_build_directory
    $make_program MeshUtilityTest $make_configuration
    cd ./test
    ./MeshUtilityTest
    cd ../..
fi  