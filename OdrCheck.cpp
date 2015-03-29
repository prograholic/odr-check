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


class FindTagDecl : public RecursiveASTVisitor<FindTagDecl>
{
public:
  FindTagDecl(TagDecl* root, raw_ostream& out) : Root(root), Out(out) {
  }

  bool VisitTagDecl(TagDecl* D) {
    if (!IsSameDecls(D)) {
      return true;
    }

    return CheckDecls(D);
  }
private:
  TagDecl* Root;
  raw_ostream& Out;

  bool IsSameDecls(TagDecl* D) {
    if (D->getName() != Root->getName()) {
      Out << "skipping decl with name [" << D->getName() << "], "
             "expected [" << Root->getName()<< "]\n";
      return false;
    }
    Out << "name [" << D->getName() << "]\n";

    if (D->getTagKind() != Root->getTagKind()) {
      Out << "skipping decl with tag [" << D->getTagKind() << "], "
             "expected [" << Root->getTagKind()<< "]\n";
      return false;
    }
    Out << "tag [" << D->getTagKind() << "]\n";

    DeclContext* ddc = D->getDeclContext();
    DeclContext* rdc = Root->getDeclContext();

    if (!IsSameDeclContexts(ddc, rdc)) {
      Out << "decl contexts are different\n";
      return false;
    }

    Out << "decls are same\n";

    return true;
  }

  bool IsSameDeclContexts(DeclContext* ddc, DeclContext* rdc) {
    while (ddc && rdc) {
      if (ddc->isTranslationUnit() && rdc->isTranslationUnit()) {
        Out << "both decl contexts are TU\n";
        return true;
      }
      else if (ddc->isTranslationUnit() || rdc->isTranslationUnit()) {
        Out << "ddc is TU[" << ddc->isTranslationUnit() << "], "
               "rdc is TU[" << rdc->isTranslationUnit() << "]\n";
        return false;
      }

      if (!IsSameDeclContextsImp(ddc, rdc)) {
        return false;
      }

      ddc = ddc->getParent();
      rdc = rdc->getParent();
    }

    return false;
  }

  bool CheckDecls(TagDecl* D) {
    Out << "D: \n";
    D->dump(Out);

    Out << "R: \n";
    Root->dump(Out);



    return true;
  }

  bool IsSameDeclContextsImp(DeclContext* ddc, DeclContext* rdc) {
    if (ddc->getDeclKind() != rdc->getDeclKind()) {
      Out << "expected decl kind ["<< rdc->getDeclKindName() << "], "
             "got [" << ddc->getDeclKindName() << "\n";

      return false;
    }
    Out << "decl kind ["<< rdc->getDeclKindName() << "]\n";

    if (isa<NamespaceDecl>(ddc)) {
      NamespaceDecl* dnd = cast<NamespaceDecl>(ddc);
      NamespaceDecl* rnd = cast<NamespaceDecl>(rdc);

      if (dnd->getName() != rnd->getName()) {
        Out << "expected namespace [" << rnd->getName() << "], "
               "but got namespace [" << dnd->getName() << "]\n";

        return false;
      }

      Out << "namespace [" << rnd->getName() << "]\n";
    }

    return true;
  }
};


class RootVisitor : public RecursiveASTVisitor<RootVisitor>
{
public:

  RootVisitor(TranslationUnitDecl* other, raw_ostream& out): Other(other), Out(out) {
  }
#if 0
  bool VisitDecl(Decl *D) {
    Out << "decl begin {\n";
    D->dump(Out);
    Out << "} cxx record end\n";

    return true;
  }
#endif //0

  bool VisitTagDecl(TagDecl* D) {

    FindTagDecl other(D, Out);

    return other.TraverseDecl(Other);
#if 0
    Out << "cxx record begin {\n";
    D->dump(Out);
    Out << "} cxx record end\n";
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

    raw_ostream& out = errs();

#if 0
    out << "root\n";
    rootTuDecl->dump(out);

    out << "\nother\n";
    otherTuDecl->dump();

    out << "\nvisitor\n";
#endif //0

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
