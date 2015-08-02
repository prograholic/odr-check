BD=`pwd`/../../build_dbg

$BD/bin/clang -cc1 -emit-pch -o output/m1.ast src/m1.cpp
$BD/bin/clang -cc1 -emit-pch -o output/m2.ast src/m2.cpp
$BD/bin/clang -cc1 -emit-pch -o output/m3.ast src/m3.cpp
$BD/bin/clang -cc1 -emit-pch -o output/m4.ast src/m4.cpp

$BD/bin/clang -cc1 -emit-pch -o output/m_res.ast -ast-merge output/m1.ast -ast-merge output/m2.ast -ast-merge output/m3.ast -ast-merge output/m4.ast src/empty.cpp

#// RUN: %clang_cc1 -emit-pch -o %t.without_attr.ast %S/Inputs/struct_without_attributes.cpp
#// RUN: %clang_cc1 -emit-pch -o %t.with_attr.ast %S/Inputs/struct_with_attributes.cpp
#// RUN: not %clang_cc1 -ast-merge %t.without_attr.ast -ast-merge %t.with_attr.ast -fsyntax-only %s 2>&1 | FileCheck %s
