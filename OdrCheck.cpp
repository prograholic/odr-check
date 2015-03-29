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

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Serialization/ASTWriter.h"
#include "clang/Serialization/ASTReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Signals.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

namespace odr_check {

typedef std::vector<std::unique_ptr<ASTUnit>> ASTList;

class OdrCheckingStrategy {
public:
  virtual bool Check(ASTList& ASTs) = 0;
};

class OdrCheckAction : public ToolAction {
public:


private:
  ASTList ASTs;

public:
  OdrCheckAction(){}

  bool runInvocation(CompilerInvocation *Invocation, FileManager *Files,
                     DiagnosticConsumer *DiagConsumer) override {

    std::unique_ptr<ASTUnit> AST = ASTUnit::LoadFromCompilerInvocation(
        Invocation, CompilerInstance::createDiagnostics(
                        &Invocation->getDiagnosticOpts(), DiagConsumer,
                        /*ShouldOwnClient=*/false));
    if (!AST)
      return false;

    ASTs.push_back(std::move(AST));
    return true;
  }

  ASTList& getASTList() {
    return ASTs;
  }

  virtual std::unique_ptr<OdrCheckingStrategy> createOdrCheckingStrategy() = 0;
};


class OdrCheckTool : private ClangTool {
public:
  OdrCheckTool(const CompilationDatabase &Compilations,
               ArrayRef<std::string> SourcePaths)
    : ClangTool(Compilations, SourcePaths) {
  }

  int runOdrCheck(OdrCheckAction *action) {
    int Res = run(action);
    if (Res != EXIT_SUCCESS) {
      return Res;
    }
    auto CheckingStrategy = action->createOdrCheckingStrategy();
    return CheckOdr(CheckingStrategy.get(), action->getASTList());
  }

private:
  int CheckOdr(OdrCheckingStrategy* CheckingStrategy, ASTList& ASTs) {
    if (!CheckingStrategy->Check(ASTs)) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }
};

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


class FindCxxRecordDecl : public RecursiveASTVisitor<FindCxxRecordDecl>
{
public:
  FindCxxRecordDecl(CXXRecordDecl* root, raw_ostream& out) : Root(root), Out(out) {
  }

  bool VisitCXXRecordDecl(CXXRecordDecl* D) {
    if (D->getName() == Root->getName()) {
      Out << "found decl with name [" << D->getName() << "]\n";
      Out.flush();
    }

    return true;
  }
private:
  CXXRecordDecl* Root;
  raw_ostream& Out;
};


class RootVisitor : public RecursiveASTVisitor<RootVisitor>
{
public:

  RootVisitor(TranslationUnitDecl* other, raw_ostream& out): Other(other), Out(out) {
  }
#if 0
  bool VisitDecl(Decl *D) {
    Out << "decl begin {\n";
    Out.flush();
    D->dump(Out);
    Out.flush();
    Out << "} cxx record end\n";
    Out.flush();

    return true;
  }
#endif //0

  bool VisitCXXRecordDecl(CXXRecordDecl* D) {

    FindCxxRecordDecl other(D, Out);

    return other.TraverseDecl(Other);
#if 0
    Out << "cxx record begin {\n";
    Out.flush();
    D->dump(Out);
    Out.flush();
    Out << "} cxx record end\n";
    Out.flush();
    return true;
#endif //0
  }

private:
  TranslationUnitDecl* Other;
  raw_ostream& Out;
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
    return EXIT_SUCCESS;
  }

private:
  bool MergeAsts(ASTUnit* root, ASTUnit* other) const {
    ASTContext& rootCtx = root->getASTContext();
    ASTContext& otherCtx = other->getASTContext();

    TranslationUnitDecl* rootTuDecl = rootCtx.getTranslationUnitDecl();
    TranslationUnitDecl* otherTuDecl = otherCtx.getTranslationUnitDecl();

    raw_ostream& out = outs();

    out << "root\n";
    out.flush();
    rootTuDecl->dump(out);
    out.flush();

    out << "\nother\n";
    out.flush();
    otherTuDecl->dump();
    out.flush();

    out << "\nvisitor\n";
    out.flush();
    RootVisitor V(otherTuDecl, out);
    V.TraverseDecl(rootTuDecl);


    return false;
  }
};


class MergeAstsAction : public OdrCheckAction {
public:
  std::unique_ptr<OdrCheckingStrategy> createOdrCheckingStrategy() override {
    return std::unique_ptr<OdrCheckingStrategy>(new ASTMergingCheckingStrategy);
  }

};

} // odr_check namespace

using namespace odr_check;

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
