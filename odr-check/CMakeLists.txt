project(odr-check)

set(LLVM_LINK_COMPONENTS
  Support
)

add_clang_executable(odr-check
  OdrCheck.cpp
)

target_link_libraries(odr-check
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
)
