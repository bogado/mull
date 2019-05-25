#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class AndOrReplacementVisitor
    : public clang::RecursiveASTVisitor<AndOrReplacementVisitor> {
public:
  AndOrReplacementVisitor(const VisitorParameters &parameters);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool foundMutant();
  clang::SourceRange getSourceRange();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull
