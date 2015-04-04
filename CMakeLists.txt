project(odr-check)

set(LLVM_LINK_COMPONENTS
  Support
)

add_clang_executable(odr-check
  DeclContextChainsComparer.cpp
  DeclContextChainsComparer.h
  OdrCheckTool.cpp
  OdrCheckTool.h
  OdrCheckAction.cpp
  OdrCheckAction.h
  OdrCheck.cpp
  OdrViolationsScanner.cpp
  OdrViolationsScanner.h
)

target_link_libraries(odr-check
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
)
