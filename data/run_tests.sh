BD=/usr/src/projects/github/prograholic/llvm/build_dbg
SD=/usr/src/projects/github/prograholic/llvm/src

cd $BD/tools/clang/test
/usr/bin/python $SD/utils/lit/lit.py -sv --param clang_site_config=$BD/tools/clang/test/lit.site.cfg $BD/tools/clang/test/ASTMerge/
