#ifndef LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_ACTION_H
#define LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_ACTION_H

#include "clang/Frontend/ASTUnit.h"
#include "clang/Tooling/Tooling.h"

namespace clang {
namespace odr_check {

class OdrCheckASTMerger;

class OdrCheckActionFactory {
public:
  explicit OdrCheckActionFactory(OdrCheckASTMerger& ASTMerger);

  std::unique_ptr<ASTConsumer> newASTConsumer();

private:
  OdrCheckASTMerger& ASTMerger;
};


} // end namespace odr_check
} // end namespace clang

#endif /* LLVM_CLANG_TOOL_ODR_CHECK_ODR_CHECK_ACTION_H */
