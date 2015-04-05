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

#include "OdrCheckAction.h"
#include "OdrCheckTool.h"
#include "OdrViolationsScanner.h"
#include "ASTsTagDeclVisitor.h"

using namespace clang;
using namespace clang::odr_check;
using namespace clang::tooling;
using namespace llvm;

namespace {

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

class MergeTagDeclProcessor : public TagDeclProcessor {
public:
  MergeTagDeclProcessor(llvm::raw_ostream& out, ASTImporter& importer)
    : m_out(out)
    , m_importer(importer) {
  }

  bool Process(TagDecl *left, TagDecl *right) override {
    OdrViolationsScanner scanner(m_out);

    if (!scanner.Scan(left, right)) {
      m_out << "found ODR violation\n";
      return false;
    }

    if (!m_importer.Import(right)) {
      /// actually this should be assertion failure
      m_out << "failed to import decl:\n";
      right->dump(m_out);
      return false;
    }

    return true;
  }

private:
  llvm::raw_ostream& m_out;
  ASTImporter& m_importer;
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

    raw_ostream& out = errs();

    ASTImporter importer(leftCtx, left->getFileManager(), rightCtx, right->getFileManager(), false);

    MergeTagDeclProcessor proc(out, importer);

    ASTsTagDeclVisitor visitor(out, proc);

    return visitor.VisitASTs(leftCtx, rightCtx);
  }
};


class MergeAstsAction : public OdrCheckAction {
public:
  std::unique_ptr<OdrCheckingStrategy> createOdrCheckingStrategy() override {
    return std::unique_ptr<OdrCheckingStrategy>(new ASTMergingCheckingStrategy);
  }

};

} // end namespace


// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory ToolTemplateCategory("odr-check options");

int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();
  CommonOptionsParser OptionsParser(argc, argv, ToolTemplateCategory);
  OdrCheckTool Tool(OptionsParser.getCompilations(),
                    OptionsParser.getSourcePathList());

  MergeAstsAction Action;
  return Tool.runOdrCheck(&Action);
}
