project(odr_check_plugin)

set(LLVM_LINK_COMPONENTS
  Support
)

add_llvm_loadable_module(odr_check_plugin
  OdrCheckPlugin.cpp
)

if(LLVM_ENABLE_PLUGINS AND (WIN32 OR CYGWIN))
  target_link_libraries(odr_check_plugin ${cmake_2_8_12_PRIVATE}
    clangAnalysis
    clangAST
    clangStaticAnalyzerCore
    LLVMSupport
    )
endif()
