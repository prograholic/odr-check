#include "OdrCheckAction.h"

#include "clang/Frontend/CompilerInstance.h"

namespace clang {
namespace odr_check {

OdrCheckAction::OdrCheckAction(){
}

bool OdrCheckAction::runInvocation(CompilerInvocation *Invocation, FileManager *Files,
                   DiagnosticConsumer *DiagConsumer) {

  std::unique_ptr<ASTUnit> AST = ASTUnit::LoadFromCompilerInvocation(
      Invocation, CompilerInstance::createDiagnostics(
                      &Invocation->getDiagnosticOpts(), DiagConsumer,
                      /*ShouldOwnClient=*/false));
  if (!AST)
    return false;

  ASTs.push_back(std::move(AST));
  return true;
}

ASTList& OdrCheckAction::getASTList() {
  return ASTs;
}

} // end namespace odr_check
} // end namespace clang
