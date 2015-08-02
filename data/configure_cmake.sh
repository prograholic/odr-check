BD=`pwd`/../../../build_dbg

mkdir cmake_build
cd cmake_build
CC=$BD/bin/clang CXX=$BD/bin/clang++ cmake ../src


make