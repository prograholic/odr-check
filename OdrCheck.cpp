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
#include "DeclContextChainsComparer.h"
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


/**
 * @brief The TagDeclFinder class finds corresponding TagDecl from another AST
 * for given left TagDecl
 */
class TagDeclFinder : public RecursiveASTVisitor<TagDeclFinder>
{
public:
  TagDeclFinder(TagDecl* right, raw_ostream& out)
    : m_right(right)
    , m_out(out) {
  }

  /**
   * @brief VisitTagDecl - visit TagDecl declaration and check it with Root
   *
   * This function tries to compare given tag declaration (D) with (Root) tag declaration
   * We ensure that both decls have same parent contexts and have same name and tag.
   * If all preconditions are met, we check two decls for ODR violations.
   * If violation is detected we return false
   */
  bool VisitTagDecl(TagDecl* left) {
    if (!IsSameDecls(left)) {
      return true;
    }

    return FindOdrViolationsForDecl(left);
  }
private:
  TagDecl* m_right;
  raw_ostream& m_out;

  bool IsSameDecls(TagDecl* left) {
    if (left->getName() != m_right->getName()) {
      m_out << "skipping decl with name [" << left->getName() << "], "
             "expected [" << m_right->getName()<< "]\n";
      return false;
    }
    m_out << "name [" << left->getName() << "]\n";

    if (left->getTagKind() != m_right->getTagKind()) {
      m_out << "skipping decl with tag [" << left->getTagKind() << "], "
             "expected [" << m_right->getTagKind()<< "]\n";
      return false;
    }
    m_out << "tag [" << left->getTagKind() << "]\n";

    DeclContext* leftDeclCtx = left->getDeclContext();
    DeclContext* rightDeclCtx = m_right->getDeclContext();

    if (!IsSameDeclContexts(leftDeclCtx, rightDeclCtx)) {
      m_out << "decl contexts are different\n";
      return false;
    }

    m_out << "decls are same\n";

    return true;
  }

  bool IsSameDeclContexts(DeclContext* left, DeclContext* right) {
    DeclContextChainsComparer cmp(m_out);

    return cmp.isSame(left, right);
  }

  bool FindOdrViolationsForDecl(TagDecl* left) {
    OdrViolationsScanner Scanner(m_out);

    return Scanner.Scan(left, m_right);
  }
};

/**
 * @brief The TagDeclVisitor class visits each TagDecl object
 * and compares it with given (left) TagDecl object
 */
class TagDeclVisitor : public RecursiveASTVisitor<TagDeclVisitor>
{
public:

  TagDeclVisitor(TranslationUnitDecl* left, raw_ostream& out)
    : m_left(left)
    , m_out(out) {
  }

  bool VisitTagDecl(TagDecl* right) {
    TagDeclFinder other(right, m_out);
    return other.TraverseDecl(m_left);
  }

private:
  TranslationUnitDecl* m_left;
  raw_ostream& m_out;
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

    TranslationUnitDecl* leftTuDecl = leftCtx.getTranslationUnitDecl();
    TranslationUnitDecl* rightTuDecl = rightCtx.getTranslationUnitDecl();

    raw_ostream& out = errs();

#if 0
    out << "root\n";
    rootTuDecl->dump(out);

    out << "\nother\n";
    otherTuDecl->dump();

    out << "\nvisitor\n";
#endif //0

    TagDeclVisitor V(leftTuDecl, out);
    V.TraverseDecl(rightTuDecl);


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
