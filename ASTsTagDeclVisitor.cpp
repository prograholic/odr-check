#include "ASTsTagDeclVisitor.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTContext.h"

#include "DeclContextChainsComparer.h"


namespace clang {
namespace odr_check {

namespace {


struct CheckTagDeclInOutParams
{
  TagDecl* decl;
  bool continueScanning;
};

/**
 * @brief The TagDeclFinder class finds corresponding TagDecl from another AST
 * for given left TagDecl
 */
class TagDeclFinder : public RecursiveASTVisitor<TagDeclFinder>
{
public:
  TagDeclFinder(raw_ostream& out, CheckTagDeclInOutParams& params, TagDeclProcessor& proc)
    : m_out(out)
    , m_params(params)
    , m_proc(proc) {
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

    m_params.continueScanning = m_proc.Process(left, m_params.decl);

    /** We stop scanning for given TagDecl
     * because in given AST there should not be else one Decl with same name and DeclContext
     *
     * However we may continue scanning for another TagDecl.
     * It depends on value m_params.continueScanning
     */
    return false;
  }
private:
  raw_ostream& m_out;
  CheckTagDeclInOutParams& m_params;
  TagDeclProcessor& m_proc;


  bool IsSameDecls(TagDecl* left) {
    if (left->getName() != m_params.decl->getName()) {
      m_out << "skipping decl with name [" << left->getName() << "], "
             "expected [" << m_params.decl->getName()<< "]\n";
      return false;
    }
    m_out << "name [" << left->getName() << "]\n";

    if (left->getTagKind() != m_params.decl->getTagKind()) {
      m_out << "skipping decl with tag [" << left->getTagKind() << "], "
             "expected [" << m_params.decl->getTagKind()<< "]\n";
      return false;
    }
    m_out << "tag [" << left->getTagKind() << "]\n";

    DeclContext* leftDeclCtx = left->getDeclContext();
    DeclContext* rightDeclCtx = m_params.decl->getDeclContext();

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
};

/**
 * @brief The RightASTVisitor class enumerates all TagDecl objects in `right` AST
 * and starts scanning of another TU decl (m_left) with given TagDecl object.
 */
class RightASTVisitor : public RecursiveASTVisitor<RightASTVisitor>
{
public:

  RightASTVisitor(llvm::raw_ostream &out, TagDeclProcessor &proc, ASTContext& left)
    : m_out(out)
    , m_proc(proc)
    , m_left(left) {
  }

  bool VisitTagDecl(TagDecl* right) {
    CheckTagDeclInOutParams params = {right, false};
    TagDeclFinder other(m_out, params, m_proc);
    if (!other.TraverseDecl(m_left.getTranslationUnitDecl())) {
      /**
       * We successfully found same decl
       */
    }

    return params.continueScanning;
  }

private:
  raw_ostream& m_out;
  TagDeclProcessor& m_proc;
  ASTContext& m_left;

};

} // end namespace


ASTsTagDeclVisitor::ASTsTagDeclVisitor(llvm::raw_ostream &out, TagDeclProcessor &proc)
  : m_out(out)
  , m_proc(proc) {
}

bool ASTsTagDeclVisitor::VisitASTs(ASTContext& left, ASTContext& right) {
  RightASTVisitor rightASTVisitor(m_out, m_proc, left);

  return rightASTVisitor.TraverseDecl(right.getTranslationUnitDecl());
}

} // end namespace odr_check
} // end namespace clang
