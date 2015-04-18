project(odr-check)

set(LLVM_LINK_COMPONENTS
  Support
)

add_clang_executable(odr-check
  DeclContextChainsComparer.cpp
  DeclContextChainsComparer.h
  OdrCheckASTMerger.cpp
  OdrCheckASTMerger.h
  OdrCheckActionFactory.cpp
  OdrCheckActionFactory.h
  OdrCheck.cpp
  OdrViolationsScanner.cpp
  OdrViolationsScanner.h
  ASTsTagDeclVisitor.h
  ASTsTagDeclVisitor.cpp
)

target_link_libraries(odr-check
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
)
