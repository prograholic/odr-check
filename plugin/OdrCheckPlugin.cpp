

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Serialization/ASTWriter.h"
#include "clang/Lex/Preprocessor.h"


namespace clang
{

namespace odr_check
{


class StoreDeclaration : public SemaConsumer
{
public:

  explicit StoreDeclaration(const std::string& filename, const std::string& sysRoot, raw_ostream& os, bool shouldDumpAST)
    : m_filename(filename)
    , m_sysRoot(sysRoot)
    , m_sema(nullptr)
    , m_os(os)
    , m_shouldDumpAST(shouldDumpAST) {
  }

  void InitializeSema(Sema &S) {
    m_sema = &S;
  }

  void HandleTranslationUnit(ASTContext &Ctx) override {

    if (Ctx.getDiagnostics().hasFatalErrorOccurred()) {
      llvm::errs() << "fatal error occurred\n";
      return;
    }

    if (m_shouldDumpAST) {
      Ctx.getTranslationUnitDecl()->dump(llvm::errs());
    }

    SmallVector<char, 128> buffer;
    llvm::BitstreamWriter bitstreamWriter(buffer);

    ASTWriter astWriter(bitstreamWriter);

    assert(m_sema);
    astWriter.WriteAST(*m_sema, m_filename, nullptr, m_sysRoot);

    m_os.write((char *)&buffer.front(), buffer.size());

    // Make sure it hits disk now.
    m_os.flush();

    // Free up some memory, in case the process is kept alive.
    buffer.clear();

    llvm::errs() << "TU stored\n";
  }

private:
  const std::string m_filename;
  const std::string m_sysRoot;
  Sema* m_sema;
  raw_ostream& m_os;
  bool m_shouldDumpAST;
};


class OdrCheckPluginAction : public PluginASTAction
{
public:

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance & CI,
                                                 llvm::StringRef InFile) override {

    raw_ostream* os = CI.createOutputFile(m_odrOutputFileName, /*Binary=*/true,
                                            /*RemoveFileOnSignal=*/false, InFile,
                                            /*Extension=*/"", /*useTemporary=*/true);

    if (!os) {
      return nullptr;
    }

    llvm::errs() << "writing data to [" << m_odrOutputFileName << "]...\n";

    return llvm::make_unique<StoreDeclaration>(m_odrOutputFileName, CI.getHeaderSearchOpts().Sysroot, *os, m_shouldDumpAST);
  }

  bool ParseArgs(const CompilerInstance &/* CI */,
                 const std::vector<std::string> & args) override {

    enum {
      AS_None,
      AS_OdrOutputConsumed,
      AS_OdrFileConsumed,
    } ArgsState = AS_None;

    for (auto arg : args) {
      switch (ArgsState) {
      case AS_None:
        if ("-odr_output" == arg) {
          ArgsState = AS_OdrOutputConsumed;
        }
        break;

      case AS_OdrOutputConsumed:
        if (arg == "--should_dump_ast") {
          llvm::errs() << "incorrect argument position [" << arg << "], please fix args list\n";
          return false;
        }
        m_odrOutputFileName = arg;
        ArgsState = AS_OdrFileConsumed;
        break;

      case AS_OdrFileConsumed:
        /// invalid input;
        break;
      }

      if (arg == "--should_dump_ast") {
        m_shouldDumpAST = true;
      }
    }

    if (ArgsState != AS_OdrFileConsumed) {
      llvm::errs() << "please set -odr_output parameter with filename\n";
      return false;
    }

    return true;
  }

private:
  std::string m_odrOutputFileName;
  bool m_shouldDumpAST = false;

};


namespace
{

FrontendPluginRegistry::Add<OdrCheckPluginAction> OdrCheckPlugin("odr-check", "odr check plugin");

} // end of namespace

} // end of namespace odr_check

} // end of namespace clang
