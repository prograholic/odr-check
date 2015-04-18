#include "OdrCheckASTMerger.h"

#include "clang/AST/ASTImporter.h"

#include "ASTsTagDeclVisitor.h"
#include "OdrViolationsScanner.h"

namespace clang {
namespace odr_check {

namespace {

class MergeTagDeclProcessor : public TagDeclProcessor {
public:
  explicit MergeTagDeclProcessor(ASTImporter& importer)
    : m_importer(importer) {
  }

  bool Process(TagDecl *left, TagDecl *right) override {
    OdrViolationsScanner scanner;

    if (!scanner.Scan(left, right)) {
      /// found ODR violation(s)
      return false;
    }

    if (!m_importer.Import(right)) {
      right->dump(llvm::errs());
      assert(0 && "import failed");
      return false;
    }

    return true;
  }

private:
  ASTImporter& m_importer;
};

} // end namespace

OdrCheckASTMerger::OdrCheckASTMerger() {
}

void OdrCheckASTMerger::Merge(ASTContext &Ctx) {
  if (!MergedContext) {
    /// simply create context from another context
    LangOpts = Ctx.getLangOpts();
    Idents.reset(new IdentifierTable(LangOpts));
    Builtins = Ctx.BuiltinInfo;

    MergedContext = llvm::make_unique<ASTContext>(LangOpts,
                                                  Ctx.getSourceManager(),
                                                  *Idents,
                                                  Selectors,
                                                  Builtins);
  }

  ASTImporter importer(*MergedContext,
                       MergedContext->getSourceManager().getFileManager(),
                       Ctx,
                       Ctx.getSourceManager().getFileManager(),
                       false);

  MergeTagDeclProcessor proc(importer);
  ASTsTagDeclVisitor visitor(proc);

  if (!visitor.VisitASTs(*MergedContext, Ctx)) {
    llvm::errs() << "failed to visit Ctx\n";
  }
}


} // end namespace odr_check
} // end namespace clang
