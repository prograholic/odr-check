#include "DeclContextChainsComparer.h"

#include "clang/AST/Decl.h"

namespace clang {
namespace odr_check {

bool DeclContextChainsComparer::isSame(DeclContext* left, DeclContext* right) {
  /// @todo See NamedDecl::printQualifiedName for visiting decl contexts

  while (left && right) {
    if (left->isTranslationUnit() && right->isTranslationUnit()) {
      return true;
    }
    else if (left->isTranslationUnit() || right->isTranslationUnit()) {
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


bool DeclContextChainsComparer::isSameDeclContexts(DeclContext* left, DeclContext* right) {
  if (left->getDeclKind() != right->getDeclKind()) {
    /// different kinds of DeclContexts
    return false;
  }

  if (NamespaceDecl* leftNsDecl = cast<NamespaceDecl>(left)) {
    NamespaceDecl* rightNsDecl = cast<NamespaceDecl>(right);
    assert(rightNsDecl);

    if (leftNsDecl->getName() != rightNsDecl->getName()) {
      /// different namespace names
      return false;
    }
  }

  return true;
}

} // end namespace odr_check
} // end namespace clang
