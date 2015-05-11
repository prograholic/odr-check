#ifndef LLVM_CLANG_TOOL_ODR_CHECK_ASTS_TAG_DECL_VISITOR_H
#define LLVM_CLANG_TOOL_ODR_CHECK_ASTS_TAG_DECL_VISITOR_H

namespace clang {

class TagDecl;
class ASTContext;

namespace odr_check {

/**
 * Method Process is invoked when two TagDecls from different ASTs have
 *  same DeclContexts, same name and decl type
 */
class TagDeclProcessor {
public:
  virtual ~TagDeclProcessor() {
  }

  /**
   * @brief perform operation for each TagDecls from both ASTs
   *
   * @return false if process of visiting should be stopped
   */
  virtual bool Process(TagDecl* left, TagDecl* right) = 0;

  /**
   * @brief Called each time we found new TagDecl which does not exists in merged ASTContext
   * @param newDecl
   */
  virtual void OnNewDecl(TagDecl* newDecl) = 0;
};

class ASTsTagDeclVisitor {
public:
  explicit ASTsTagDeclVisitor(TagDeclProcessor& proc);

  /**
   * @brief VisitASTs visit all TagDecls in both ASTs
   *
   * @returns false if visiting was terminated, otherwise true
   */
  bool VisitASTs(ASTContext& mergedCtx, ASTContext& newCtx);

private:
  TagDeclProcessor& m_proc;
};

} // end namespace odr_check
} // end namespace clang

#endif /* LLVM_CLANG_TOOL_ODR_CHECK_ASTS_TAG_DECL_VISITOR_H */
