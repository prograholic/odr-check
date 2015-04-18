//===---- tools/extra/ToolTemplate.cpp - Template for refactoring tool ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements an empty refactoring tool using the clang tooling.
//  The goal is to lower the "barrier to entry" for writing refactoring tools.
//
//  Usage:
//  tool-template <cmake-output-dir> <file1> <file2> ...
//
//  Where <cmake-output-dir> is a CMake build directory in which a file named
//  compile_commands.json exists (enable -DCMAKE_EXPORT_COMPILE_COMMANDS in
//  CMake to get this output).
//
//  <file1> ... specify the paths of files in the CMake source tree. This path
//  is looked up in the compile command database. If the path of a file is
//  absolute, it needs to point into CMake's source tree. If the path is
//  relative, the current working directory needs to be in the CMake source
//  tree and the file must be in a subdirectory of the current working
//  directory. "./" prefixes in the relative files will be automatically
//  removed, but the rest of a relative path must be a suffix of a path in
//  the compile command line database.
//
//  For example, to use tool-template on all files in a subtree of the
//  source tree, use:
//
//    /path/in/subtree $ find . -name '*.cpp'|
//        xargs tool-template /path/to/build
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTImporter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/Signals.h"

#include "OdrCheckActionFactory.h"
#include "OdrCheckASTMerger.h"
#include "OdrViolationsScanner.h"
#include "ASTsTagDeclVisitor.h"

using namespace clang;
using namespace clang::odr_check;
using namespace clang::tooling;
using namespace llvm;

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
      right->dump(errs());
      assert(0 && "import failed");
      return false;
    }

    return true;
  }

private:
  ASTImporter& m_importer;
};

#if 0
class SimpleComparisonCheckingStrategy : public OdrCheckingStrategy {
public:
  virtual bool Check(ASTList &ASTs) override {
    for (auto& leftAstIt : ASTs) {
      for (auto& rightAstIt : ASTs) {
        if (leftAstIt != rightAstIt) {
          if (!CheckOdrForAsts(*leftAstIt, *rightAstIt)) {
            return EXIT_FAILURE;
          }
        }
      }
    }
    return EXIT_SUCCESS;
  }

private:
  bool CheckOdrForAsts(ASTUnit& left, ASTUnit& right) const {
    return false;
  }
};




class ASTMergingCheckingStrategy : public OdrCheckingStrategy {
public:
  virtual bool Check(ASTList &ASTs) override {
    if (ASTs.empty()) {
      return true;
    }

    auto& rootAST = ASTs.front();
    for (auto astIt = ASTs.begin() + 1; astIt != ASTs.end(); ++astIt) {
      if (!MergeAsts(rootAST.get(), astIt->get())) {
        return false;
      }
    }
    return true;
  }

private:
  bool MergeAsts(ASTUnit* left, ASTUnit* right) const {
    ASTContext& leftCtx = left->getASTContext();
    ASTContext& rightCtx = right->getASTContext();

    ASTImporter importer(leftCtx, left->getFileManager(), rightCtx, right->getFileManager(), false);
    MergeTagDeclProcessor proc(importer);
    ASTsTagDeclVisitor visitor(proc);

    return visitor.VisitASTs(leftCtx, rightCtx);
  }
};


class MergeAstsAction : public OdrCheckAction {
public:
  std::unique_ptr<OdrCheckingStrategy> createOdrCheckingStrategy() override {
    return std::unique_ptr<OdrCheckingStrategy>(new ASTMergingCheckingStrategy);
  }

};


#endif //0

} // end namespace


// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory ToolTemplateCategory("odr-check options");

int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();
  CommonOptionsParser OptionsParser(argc, argv, ToolTemplateCategory);

  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  OdrCheckASTMerger ASTMerger;
  OdrCheckActionFactory ActionFactory(ASTMerger);

  std::unique_ptr<FrontendActionFactory> FrontendFactory = newFrontendActionFactory(&ActionFactory);
  return Tool.run(FrontendFactory.get());
}
