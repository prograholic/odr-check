#include "DeclContextComparer.h"

#include "clang/AST/Decl.h"

namespace clang {
namespace odr_check {

DeclContextComparer::DeclContextComparer(llvm::raw_ostream& out) : Out(out) {
}

bool DeclContextComparer::isSame(DeclContext* left, DeclContext* right) {
  /// @todo See NamedDecl::printQualifiedName for visiting decl contexts

  while (left && right) {
    if (left->isTranslationUnit() && right->isTranslationUnit()) {
      Out << "both decl contexts are TU\n";
      return true;
    }
    else if (left->isTranslationUnit() || right->isTranslationUnit()) {
      Out << "left is TU[" << left->isTranslationUnit() << "], "
             "right is TU[" << right->isTranslationUnit() << "]\n";
      return false;
    }

    if (!isSameDeclContexts(left, right)) {
      return false;
    }

    left = left->getParent();
    right = right->getParent();
  }

  return false;
}


bool DeclContextComparer::isSameDeclContexts(DeclContext* left, DeclContext* right) {
  if (left->getDeclKind() != right->getDeclKind()) {
    Out << "left decl kind ["<< left->getDeclKindName() << "], "
           "right decl kind  [" << right->getDeclKindName() << "\n";

    return false;
  }
  Out << "decl kind ["<< left->getDeclKindName() << "]\n";

  /// @todo Maybe it is sufficient to check NamedDecl + some other decls
  if (isa<NamespaceDecl>(left)) {
    NamespaceDecl* leftNamespaceDecl = cast<NamespaceDecl>(left);
    NamespaceDecl* rightNamespaceDecl = cast<NamespaceDecl>(right);

    if (leftNamespaceDecl->getName() != rightNamespaceDecl->getName()) {
      Out << "left namespace [" << leftNamespaceDecl->getName() << "], "
             "right namespace [" << rightNamespaceDecl->getName() << "]\n";

      return false;
    }

    Out << "namespace [" << leftNamespaceDecl->getName() << "]\n";
  }

  return true;
}

} // end namespace odr_check
} // end namespace clang
