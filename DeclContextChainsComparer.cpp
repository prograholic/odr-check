#include "DeclContextChainsComparer.h"

#include "clang/AST/Decl.h"

namespace clang {
namespace odr_check {

DeclContextChainsComparer::DeclContextChainsComparer(llvm::raw_ostream& out) : Out(out) {
}

bool DeclContextChainsComparer::isSame(DeclContext* left, DeclContext* right) {
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


bool DeclContextChainsComparer::isSameDeclContexts(DeclContext* left, DeclContext* right) {
  if (left->getDeclKind() != right->getDeclKind()) {
    Out << "left decl kind ["<< left->getDeclKindName() << "], "
           "right decl kind  [" << right->getDeclKindName() << "\n";

    return false;
  }
  Out << "decl kind ["<< left->getDeclKindName() << "]\n";

  if (NamespaceDecl* leftNsDecl = cast<NamespaceDecl>(left)) {
    NamespaceDecl* rightNsDecl = cast<NamespaceDecl>(right);
    assert(rightNsDecl);

    if (leftNsDecl->getName() != rightNsDecl->getName()) {
      Out << "left namespace [" << leftNsDecl->getName() << "], "
             "right namespace [" << rightNsDecl->getName() << "]\n";

      return false;
    }

    Out << "namespace [" << leftNsDecl->getName() << "]\n";
  }

  return true;
}

} // end namespace odr_check
} // end namespace clang
