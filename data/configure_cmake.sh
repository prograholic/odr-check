BD=`pwd`/../../build_dbg

cd cmake_build
CC=$BD/bin/clang CXX=$BD/bin/clang++ cmake ../src


make