#ifndef LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_AST_MERGER_H
#define LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_AST_MERGER_H

#include <memory>

namespace clang {

class ASTContext;
class CompilerInstance;
class TargetOptions;

namespace odr_check {

class OdrCheckASTMerger {
public:
  OdrCheckASTMerger();

  ~OdrCheckASTMerger();

  void Merge(ASTContext& Ctx);

private:
  std::unique_ptr<CompilerInstance> CI;

  void CreateCompilerInstance(const TargetOptions &Opts);
};

} // end namespace odr_check
} // end namespace clang


#endif /* LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_AST_MERGER_H */
