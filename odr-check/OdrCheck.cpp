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
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Support/Signals.h"

#include "OdrCheckActionFactory.h"
#include "OdrCheckASTMerger.h"
#include "OdrViolationsScanner.h"
#include "ASTsTagDeclVisitor.h"

using namespace clang;
using namespace clang::odr_check;
using namespace clang::tooling;
using namespace llvm;


// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory ToolTemplateCategory("odr-check options");

typedef std::unique_ptr<ASTUnit> ASTUnitPtr;

class TagDeclVisitor : public RecursiveASTVisitor<TagDeclVisitor>
{
public:
  TagDeclVisitor(ASTUnit& to, ASTUnit& from)
    : m_to(to)
    , m_from(from)
    , m_importer(to.getASTContext(), to.getFileManager(),
                 from.getASTContext(), from.getFileManager(),
                 false) {
  }



  void MergeASTs() {
    TraverseDecl(m_from.getASTContext().getTranslationUnitDecl());
  }

  bool VisitTagDecl(TagDecl* fromDecl) {
    m_importer.Import(fromDecl);

    return true;
  }

private:
  ASTUnit& m_to;
  ASTUnit& m_from;
  ASTImporter m_importer;
};


int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();
  //CommonOptionsParser OptionsParser(argc, argv, ToolTemplateCategory);


  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter DiagnosticPrinter(
      llvm::errs(), &*DiagOpts);
  IntrusiveRefCntPtr<DiagnosticsEngine> Diagnostics = new DiagnosticsEngine(
      IntrusiveRefCntPtr<clang::DiagnosticIDs>(new DiagnosticIDs()), &*DiagOpts,
      &DiagnosticPrinter, false);



  std::vector<ASTUnitPtr> units;

  ++argv;
  while (*argv) {
    std::string file = *argv;
    llvm::errs() << "processing file [" << file << "]...\n";
    ASTUnitPtr unit = ASTUnit::LoadFromASTFile(file, Diagnostics, FileSystemOptions());
    if (unit) {
      units.push_back(std::move(unit));
    }

    ++argv;
  }

  if (units.empty()) {
    llvm::errs() << "not AST units found\n";
    return 0;
  }

  auto front = units.begin();
  auto next = front + 1;

  while (next != units.end()) {
    ASTUnit& to = **front;
    ASTUnit& from = **next;
    llvm::errs() << "merging AST [" << from.getASTFileName() << "] to [" << to.getASTFileName() << "]...\n";

    TagDeclVisitor visitor(to, from);
    visitor.MergeASTs();

    ++next;
  }

  return 0;
}
