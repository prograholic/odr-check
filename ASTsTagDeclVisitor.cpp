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
  bool declFoundInMergedCtx;
};

/**
 * @brief The TagDeclFinder class finds corresponding TagDecl from another AST
 * for given left TagDecl
 */
class TagDeclFinder : public RecursiveASTVisitor<TagDeclFinder>
{
public:
  TagDeclFinder(CheckTagDeclInOutParams& params, TagDeclProcessor& proc)
    : m_params(params)
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

    m_params.declFoundInMergedCtx = true;
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
  CheckTagDeclInOutParams& m_params;
  TagDeclProcessor& m_proc;


  bool IsSameDecls(TagDecl* left) {
    if (left->getName() != m_params.decl->getName()) {
      return false;
    }

    if (left->getTagKind() != m_params.decl->getTagKind()) {
      return false;
    }

    DeclContext* leftDeclCtx = left->getDeclContext();
    DeclContext* rightDeclCtx = m_params.decl->getDeclContext();

    if (!IsSameDeclContexts(leftDeclCtx, rightDeclCtx)) {
      return false;
    }

    /// two decls are same
    return true;
  }

  bool IsSameDeclContexts(DeclContext* left, DeclContext* right) {
    DeclContextChainsComparer cmp;

    return cmp.isSame(left, right);
  }
};

/**
 * @brief The NewASTVisitor class enumerates all TagDecl objects in `new` AST
 * and starts scanning of another TU decl (m_left) with given TagDecl object.
 */
class NewASTVisitor : public RecursiveASTVisitor<NewASTVisitor>
{
public:

  NewASTVisitor(TagDeclProcessor &proc, ASTContext& mergedCtx)
    : m_proc(proc)
    , m_mergedCtx(mergedCtx) {
  }

  bool VisitTagDecl(TagDecl* newDecl) {
    CheckTagDeclInOutParams params = {newDecl, false, false};
    TagDeclFinder other(params, m_proc);

    other.TraverseDecl(m_mergedCtx.getTranslationUnitDecl());

    if (!params.declFoundInMergedCtx) {
      m_proc.OnNewDecl(newDecl);
    }

    return params.continueScanning;
  }

private:
  TagDeclProcessor& m_proc;
  ASTContext& m_mergedCtx;

};

} // end namespace


ASTsTagDeclVisitor::ASTsTagDeclVisitor(TagDeclProcessor &proc)
  : m_proc(proc) {
}

bool ASTsTagDeclVisitor::VisitASTs(ASTContext& mergedCtx, ASTContext& newCtx) {
  NewASTVisitor newASTVisitor(m_proc, mergedCtx);

  return newASTVisitor.TraverseDecl(newCtx.getTranslationUnitDecl());
}

} // end namespace odr_check
} // end namespace clang
