#ifndef LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_AST_MERGER_H
#define LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_AST_MERGER_H

#include "clang/AST/ASTContext.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/Builtins.h"

namespace clang {


namespace odr_check {

class OdrCheckASTMerger {
public:
  OdrCheckASTMerger();

  void Merge(ASTContext& Ctx);

private:
  LangOptions LangOpts;
  std::unique_ptr<IdentifierTable> Idents;
  SelectorTable Selectors;
  Builtin::Context Builtins;

  std::unique_ptr<ASTContext> MergedContext;
};

} // end namespace odr_check
} // end namespace clang


#endif /* LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_AST_MERGER_H */
