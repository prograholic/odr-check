#include "OdrViolationsScanner.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ASTContext.h"

namespace clang {
namespace odr_check {

bool OdrViolationsScanner::Scan(TagDecl *left, TagDecl *right) {
  return true;
  assert(left->getKind() == right->getKind());

  if (CXXRecordDecl* leftCxxRec = cast<CXXRecordDecl>(left)) {
    CXXRecordDecl* rightCxxRec = cast<CXXRecordDecl>(right);
    assert(rightCxxRec);

    if (!ScanCxxRecordDeclSpecific(leftCxxRec, rightCxxRec)) {
      return false;
    }
  }

  if (RecordDecl* leftRec = cast<RecordDecl>(left)) {
    RecordDecl* rightRec = cast<RecordDecl>(right);
    assert(rightRec);

    if (!ScanRecordDeclSpecific(leftRec, rightRec)) {
      return false;
    }
  }

  return true;
}

/// private implementation

bool OdrViolationsScanner::ScanCxxRecordDeclSpecific(CXXRecordDecl* left, CXXRecordDecl* right) {
  CXXRecordDecl::base_class_iterator leftBaseIt = left->bases_begin();
  CXXRecordDecl::base_class_iterator rightBaseIt = right->bases_begin();

  while ((leftBaseIt != left->bases_end()) && (rightBaseIt != right->bases_end())) {
    CXXBaseSpecifier& leftBase = *leftBaseIt;
    CXXBaseSpecifier& rightBase = *rightBaseIt;

    //assert(0 && "not implemented");

    //leftBase.


    ++leftBaseIt;
    ++rightBaseIt;
  }

  /// @todo scan for methods, ctors

  if (leftBaseIt != left->bases_end()) {
    return false;
  }

  if (rightBaseIt != right->bases_end()) {
    return false;
  }

  return true;
}

bool OdrViolationsScanner::ScanRecordDeclSpecific(RecordDecl* left, RecordDecl* right) {
  RecordDecl::field_iterator leftFieldIt = left->field_begin();
  RecordDecl::field_iterator rightFieldIt = right->field_begin();

  while ((leftFieldIt != left->field_end()) && (rightFieldIt != right->field_end())) {
    FieldDecl* leftBase = *leftFieldIt;
    FieldDecl* rightBase = *rightFieldIt;

    if (leftBase->isBitField() != rightBase->isBitField()) {
    }

    //assert(0 && "not implemented");

    ++leftFieldIt;
    ++rightFieldIt;
  }

  if (leftFieldIt != left->field_end()) {
    FieldDecl* unprocessed = *leftFieldIt;
    //Diag(unprocessed, "found field") << "AAA";
    return false;
  }

  if (rightFieldIt != right->field_end()) {
    FieldDecl* unprocessed = *rightFieldIt;
    //Diag(unprocessed, "found field") << "BBB";
    return false;
  }

  return true;
}

DiagnosticBuilder OdrViolationsScanner::Diag(Decl* decl, StringRef formatString) {
  DiagnosticsEngine& engine = decl->getASTContext().getDiagnostics();
  unsigned diagId = engine.getDiagnosticIDs()->getCustomDiagID(DiagnosticIDs::Error, formatString);

  return engine.Report(decl->getSourceRange().getBegin(), diagId) << decl->getSourceRange();
}

} // end namespace odr_check
} // end namespace clang
