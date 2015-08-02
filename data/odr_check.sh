BD=`pwd`/../../build_dbg
DUMP_AST_ARG=--should_dump_ast
XCLANG_ARG_PFX="-Xclang -plugin-arg-odr-check -Xclang"
ODR_PLUGIN_CLANG_ARGS="-Xclang -load -Xclang $BD/lib/odr_check_plugin.so -Xclang -plugin -Xclang odr-check $XCLANG_ARG_PFX $DUMP_AST_ARG $XCLANG_ARG_PFX -odr_output $XCLANG_ARG_PFX"
COMMON_INCLUDE="-I /usr/include"


echo "starting plugin..."
echo "" > plugin.log 2>&1

$BD/bin/clang src/m1.cpp -o output/m1.o -c $COMMON_INCLUDE $ODR_PLUGIN_CLANG_ARGS `pwd`/output/m1.odr >> plugin.log 2>&1
$BD/bin/clang src/m2.cpp -o output/m2.o -c $COMMON_INCLUDE $ODR_PLUGIN_CLANG_ARGS `pwd`/output/m2.odr >> plugin.log 2>&1
$BD/bin/clang src/m3.cpp -o output/m3.o -c $COMMON_INCLUDE $ODR_PLUGIN_CLANG_ARGS `pwd`/output/m3.odr >> plugin.log 2>&1
$BD/bin/clang src/m4.cpp -o output/m4.o -c $COMMON_INCLUDE $ODR_PLUGIN_CLANG_ARGS `pwd`/output/m4.odr >> plugin.log 2>&1
$BD/bin/clang src/simple_template_struct.cpp -o output/simple_template_struct.o -c $COMMON_INCLUDE $ODR_PLUGIN_CLANG_ARGS `pwd`/output/simple_template_struct.odr >> plugin.log 2>&1
#$BD/bin/clang src/boost.cpp -o output/boost.o -c $COMMON_INCLUDE $ODR_PLUGIN_CLANG_ARGS `pwd`/output/boost.odr >> plugin.log 2>&1


echo "starting tool..."
echo odr-check is not ready for boost yet
echo "" > tool.log 2>&1


# /usr/src/projects/github/prograholic/llvm/data/odr-check/output/m1.odr /usr/src/projects/github/prograholic/llvm/data/odr-check/output/m2.odr  /usr/src/projects/github/prograholic/llvm/data/odr-check/output/m3.odr  /usr/src/projects/github/prograholic/llvm/data/odr-check/output/simple_template_struct.odr

#$DUMP_AST_ARG
#FILES="output/m1.odr output/m2.odr output/m3.odr output/m4.odr output/simple_template_struct.odr output/boost.odr"
FILES="output/m1.odr output/m2.odr output/m3.odr output/m4.odr output/simple_template_struct.odr"
$BD/bin/odr-check  $FILES >> tool.log 2>&1
# echo odr-check finished with $?

