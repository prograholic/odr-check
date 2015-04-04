#include "OdrViolationsScanner.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"

namespace clang {
namespace odr_check {

OdrViolationsScanner::OdrViolationsScanner(llvm::raw_ostream &out)
  : Out(out) {
}

bool OdrViolationsScanner::Scan(TagDecl *left, TagDecl *right) {
  assert(left->getKind() == right->getKind());

  Out << "kind: " << left->getKindName() << "\n";

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
#if 0
  Out << "left:\n";
  left->dump(Out);

  Out << "right:\n";
  right->dump(Out);
#endif //0

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
    Out << "left has base spec\n";
    return false;
  }

  if (rightBaseIt != right->bases_end()) {
    Out << "right has base spec\n";
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
      Out << "bit field ";
    }

    //Out << "left:\n";
    //leftBase->dump(Out);
    //Out << "right:\n";
    //rightBase->dump(Out);

    //assert(0 && "not implemented");

    ++leftFieldIt;
    ++rightFieldIt;
  }

  if (leftFieldIt != left->field_end()) {
    Out << "left has field\n";
    leftFieldIt->dump(Out);
    return false;
  }

  if (rightFieldIt != right->field_end()) {
    Out << "right has field\n";
    rightFieldIt->dump(Out);
    return false;
  }

  return true;
}


} // end namespace odr_check
} // end namespace clang
