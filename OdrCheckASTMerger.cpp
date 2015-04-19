#include "OdrCheckASTMerger.h"

#include "clang/AST/ASTImporter.h"

#include "clang/Basic/TargetInfo.h"

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"

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

    return true;
  }

  void OnNewDecl(TagDecl* newDecl) override {

    //llvm::errs() << "importing [" << newDecl->getName() << "]...\n";

    if (!m_importer.Import(newDecl)) {
      newDecl->dump(llvm::errs());
      llvm::errs() << "import failed\n";
    }
  }

private:
  ASTImporter& m_importer;
};

} // end namespace

OdrCheckASTMerger::OdrCheckASTMerger() :CI() {
}

OdrCheckASTMerger::~OdrCheckASTMerger() {
  /// needed for destroying CI
}

void OdrCheckASTMerger::Merge(ASTContext &Ctx) {
  if (!CI) {
    CreateCompilerInstance(Ctx.getTargetInfo().getTargetOpts());
  }

  DiagnosticConsumer& DiagClient = CI->getDiagnosticClient();
  DiagClient.BeginSourceFile(CI->getLangOpts(),
                             &CI->getPreprocessor());

  ASTContext& mergedCtx = CI->getASTContext();

  ASTImporter importer(mergedCtx,
                       CI->getFileManager(),
                       Ctx,
                       Ctx.getSourceManager().getFileManager(),
                       false);

  MergeTagDeclProcessor proc(importer);
  ASTsTagDeclVisitor visitor(proc);

  if (!visitor.VisitASTs(mergedCtx, Ctx)) {
  }

  DiagClient.EndSourceFile();
}


void OdrCheckASTMerger::CreateCompilerInstance(const TargetOptions &Opts) {
  CI = llvm::make_unique<CompilerInstance>();

  CI->createFileManager();
  CI->createDiagnostics();
  CI->createSourceManager(CI->getFileManager());

  std::shared_ptr<TargetOptions> OptsPtr = std::make_shared<TargetOptions>(Opts);

  CI->setTarget(TargetInfo::CreateTargetInfo(CI->getDiagnostics(), OptsPtr));
  CI->createPreprocessor(TU_Complete);
  CI->createASTContext();
}

} // end namespace odr_check
} // end namespace clang
