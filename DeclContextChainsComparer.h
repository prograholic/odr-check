#ifndef LLVM_CLANG_TOOL_ODR_CHECK_DECL_CONTEXT_COMPARER_H
#define LLVM_CLANG_TOOL_ODR_CHECK_DECL_CONTEXT_COMPARER_H

namespace llvm {

class raw_ostream;

} // end namespace llvm

namespace clang {

class DeclContext;

namespace odr_check {

/**
 * @brief The DeclContextComparer class performs simple comparation of two DeclContexts
 *
 * Its main purpose - to check that two TagDecls have same declaration context
 *
 * For example
 * namespace a {
 * namespace b {
 * struct X{};
 * }
 * }
 *
 * and
 *
 * namespace a {
 * namespace c {
 * struct X{};
 * }
 * }
 *
 * should have different DeclContext chains and method isSame should return false
 */
class DeclContextChainsComparer {
public:
  explicit DeclContextChainsComparer(llvm::raw_ostream& out);


  // Check that two DeclContext objects are same.
  bool isSame(DeclContext* left, DeclContext* right);

private:
  llvm::raw_ostream& Out;

  bool isSameDeclContexts(DeclContext* left, DeclContext* right);
};

} // end namespace odr_check
} // end namespace clang

#endif /* LLVM_CLANG_TOOL_ODR_CHECK_DECL_CONTEXT_COMPARER_H */
