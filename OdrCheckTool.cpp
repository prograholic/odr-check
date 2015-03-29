#include "OdrCheckTool.h"

namespace clang {
namespace odr_check {

OdrCheckTool::OdrCheckTool(const tooling::CompilationDatabase &Compilations,
                           ArrayRef<std::string> SourcePaths)
  : ClangTool(Compilations, SourcePaths) {
}

int OdrCheckTool::runOdrCheck(OdrCheckAction *action) {
  int Res = run(action);
  if (Res != EXIT_SUCCESS) {
    return Res;
  }
  auto CheckingStrategy = action->createOdrCheckingStrategy();
  return CheckOdr(CheckingStrategy.get(), action->getASTList());
}

int OdrCheckTool::CheckOdr(OdrCheckingStrategy* CheckingStrategy, ASTList& ASTs) {
  if (!CheckingStrategy->Check(ASTs)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

} // end namespace odr_check
} // end namespace clang
