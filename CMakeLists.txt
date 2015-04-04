project(odr-check)

set(LLVM_LINK_COMPONENTS
  Support
)

add_clang_executable(odr-check
  DeclContextComparer.cpp
  DeclContextComparer.h
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
