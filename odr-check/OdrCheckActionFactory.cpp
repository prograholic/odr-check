#include "OdrCheckActionFactory.h"

#include "clang/Frontend/CompilerInstance.h"

#include "OdrCheckASTMerger.h"

namespace clang {
namespace odr_check {

namespace {

class OdrCheckASTConsumer : public ASTConsumer {
public:
  explicit OdrCheckASTConsumer(OdrCheckASTMerger& ASTMerger) : ASTMerger(ASTMerger) {
  }

  void HandleTranslationUnit(ASTContext &Ctx) override {
    ASTMerger.Merge(Ctx);
  }

private:
  OdrCheckASTMerger& ASTMerger;
};

} // end namespace

OdrCheckActionFactory::OdrCheckActionFactory(OdrCheckASTMerger& ASTMerger) : ASTMerger(ASTMerger) {
}

std::unique_ptr<ASTConsumer> OdrCheckActionFactory::newASTConsumer() {
  return llvm::make_unique<OdrCheckASTConsumer>(ASTMerger);
}

} // end namespace odr_check
} // end namespace clang
