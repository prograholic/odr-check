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
#include "clang/AST/ASTDiagnostic.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/Support/Signals.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;


bool ShouldDumpAST = false;

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory ToolTemplateCategory("odr-check options");

typedef std::unique_ptr<ASTUnit> ASTUnitPtr;

class TagDeclVisitor : public RecursiveASTVisitor<TagDeclVisitor>
{
public:
  TagDeclVisitor(ASTContext& to, FileManager& toFileMgr, ASTUnit& from)
    : m_importer(to, toFileMgr,
                 from.getASTContext(), from.getFileManager(),
                 false) {
  }



  void MergeASTs() {
    TraverseDecl(m_importer.getFromContext().getTranslationUnitDecl());
  }

  bool VisitTagDecl(TagDecl* fromDecl) {
    m_importer.Import(fromDecl);
    if (ShouldDumpAST) {
      fromDecl->dump(llvm::errs());
    }

    return true;
  }

private:
  ASTImporter m_importer;
};


int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();
  //CommonOptionsParser OptionsParser(argc, argv, ToolTemplateCategory);

  std::vector<ASTUnitPtr> units;

  ++argv;
  while (*argv) {
    const std::string arg = *argv;
    ++argv;

    if ("--should_dump_ast" == arg) {
      ShouldDumpAST = true;
      continue;
    }

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    std::unique_ptr<TextDiagnosticPrinter> DiagnosticPrinter = llvm::make_unique<TextDiagnosticPrinter>(
        llvm::errs(), &*DiagOpts);

    IntrusiveRefCntPtr<DiagnosticsEngine> Diagnostics = new DiagnosticsEngine(
        IntrusiveRefCntPtr<clang::DiagnosticIDs>(new DiagnosticIDs()), &*DiagOpts,
        DiagnosticPrinter.release());

    llvm::errs() << "processing file [" << arg << "]...\n";
    ASTUnitPtr unit = ASTUnit::LoadFromASTFile(arg, Diagnostics, FileSystemOptions());
    if (unit) {
      units.push_back(std::move(unit));
    }
  }

  if (units.empty()) {
    llvm::errs() << "no AST units found\n";
    return 0;
  }


  CompilerInstance CI;
  // FIXME: prograholic: get proper target triplet
  CI.getInvocation().TargetOpts->Triple = "i386-pc-linux-gnu";
  CI.createDiagnostics();
  CI.setTarget(TargetInfo::CreateTargetInfo(CI.getDiagnostics(),
                                            CI.getInvocation().TargetOpts));

  CI.createFileManager();
  CI.createSourceManager(CI.getFileManager());
  CI.createPreprocessor(TU_Complete);
  CI.createASTContext();
  ASTContext& toContext = CI.getASTContext();

  CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), &CI.getPreprocessor());

  CI.getDiagnostics().SetArgToStringFn(&FormatASTNodeDiagnosticArgument,
                                       &CI.getASTContext());

  for (auto& unit : units) {
    ASTUnit& from = *unit;
    llvm::errs() << "merging AST [" << from.getASTFileName() << "]\n";

    TagDeclVisitor visitor(toContext, CI.getFileManager(), from);
    visitor.MergeASTs();
  }

  // FIXME: prograholic: assertion failure if ShouldDumpAST == true
  if (ShouldDumpAST) {
    llvm::errs() << "dumping final AST...\n";

    toContext.getTranslationUnitDecl()->dump(llvm::errs());
  }

  CI.getDiagnosticClient().EndSourceFile();

  // FIXME: prograholic: add proper result code
  return 0;
}
