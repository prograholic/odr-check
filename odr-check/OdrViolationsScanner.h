#ifndef LLVM_CLANG_TOOL_ODR_CHECK_ODR_VIOLATIONS_SCANNER_H
#define LLVM_CLANG_TOOL_ODR_CHECK_ODR_VIOLATIONS_SCANNER_H

#include "clang/Basic/Diagnostic.h"

namespace clang {

class Decl;
class TagDecl;
class CXXRecordDecl;
class RecordDecl;

namespace odr_check {

/**
 * @brief Scans for ODR violations in two TagDecl objects
 *
 * It looks like RecursiveASTVisitor
 * so maybe it is possible to implement ParallelRecursiveVisitor
 * to handle two TagDecl instead of one TagDecl (in RecursiveASTVisitor).
 */
class OdrViolationsScanner {
public:

  bool Scan(TagDecl *left, TagDecl *right);

private:

  /// Scan only CXXRecordDecl-specific fields, attributes and so on
  bool ScanCxxRecordDeclSpecific(CXXRecordDecl* left, CXXRecordDecl* right);

  /// Scan only RecordDecl-specific fields, attributes and so on
  bool ScanRecordDeclSpecific(RecordDecl* left, RecordDecl* right);

  DiagnosticBuilder Diag(Decl* decl, StringRef formatString);
};

} // end namespace odr_check
} // end namespace clang


#endif /* LLVM_CLANG_TOOL_ODR_CHECK_ODR_VIOLATIONS_SCANNER_H */
