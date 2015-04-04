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

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/Signals.h"

#include "OdrCheckAction.h"
#include "OdrCheckTool.h"
#include "DeclContextComparer.h"
#include "OdrViolationsScanner.h"

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


class FindTagDecl : public RecursiveASTVisitor<FindTagDecl>
{
public:
  FindTagDecl(TagDecl* root, raw_ostream& out) : Root(root), Out(out) {
  }

  /**
   * @brief VisitTagDecl - visit TagDecl declaration and check it with Root
   *
   * This function tries to compare given tag declaration (D) with (Root) tag declaration
   * We ensure that both decls have same parent contexts and have same name and tag.
   * If all preconditions are met, we check two decls for ODR violations.
   * If violation is detected we return false
   */
  bool VisitTagDecl(TagDecl* D) {
    if (!IsSameDecls(D)) {
      return true;
    }

    return FindOdrViolationsForDecl(D);
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
    DeclContextComparer cmp(Out);

    return cmp.isSame(ddc, rdc);
  }

  bool FindOdrViolationsForDecl(TagDecl* D) {
    OdrViolationsScanner Scanner(Out);

    return Scanner.Scan(Root, D);
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
    return true;
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
