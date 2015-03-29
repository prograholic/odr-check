#ifndef LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_ACTION_H
#define LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_ACTION_H

#include "clang/Frontend/ASTUnit.h"
#include "clang/Tooling/Tooling.h"

namespace clang {
namespace odr_check {

typedef std::vector<std::unique_ptr<ASTUnit>> ASTList;

class OdrCheckingStrategy {
public:
  virtual bool Check(ASTList& ASTs) = 0;
};

class OdrCheckAction : public tooling::ToolAction {
private:
  ASTList ASTs;

public:
  OdrCheckAction();

  bool runInvocation(CompilerInvocation *Invocation, FileManager *Files,
                     DiagnosticConsumer *DiagConsumer) override;

  ASTList& getASTList();

  virtual std::unique_ptr<OdrCheckingStrategy> createOdrCheckingStrategy() = 0;
};

} // end namespace odr_check
} // end namespace clang

#endif /* LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_ACTION_H */
