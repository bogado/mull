#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class NegateConditionVisitor
    : public clang::RecursiveASTVisitor<NegateConditionVisitor> {
public:
  NegateConditionVisitor(const VisitorParameters &parameters);

  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator);
  bool foundMutant();
  clang::SourceRange getSourceRange();
private:
  InstructionRangeVisitor visitor;
};

} // namespace mull