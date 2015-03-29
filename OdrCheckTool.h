#ifndef LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_TOOL_H
#define LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_TOOL_H

#include "OdrCheckAction.h"

namespace clang {

namespace tooling {
class CompilationDatabase;
} // end namespace tooling

namespace odr_check {

class OdrCheckTool : private tooling::ClangTool {
public:
  OdrCheckTool(const tooling::CompilationDatabase &Compilations,
               ArrayRef<std::string> SourcePaths);

  int runOdrCheck(OdrCheckAction *action);

private:
  int CheckOdr(OdrCheckingStrategy* CheckingStrategy, ASTList& ASTs);
};

} // end namespace odr_check
} // end namespace clang


#endif /* LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_TOOL_H */
