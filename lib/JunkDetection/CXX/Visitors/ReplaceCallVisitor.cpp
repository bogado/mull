#include "mull/JunkDetection/CXX/Visitors/ReplaceCallVisitor.h"

using namespace mull;

ReplaceCallVisitor::ReplaceCallVisitor(const VisitorParameters &parameters)
    : visitor(parameters), astContext(parameters.astContext) {}

bool ReplaceCallVisitor::VisitCallExpr(clang::CallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

bool ReplaceCallVisitor::VisitCXXMemberCallExpr(
    clang::CXXMemberCallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

bool ReplaceCallVisitor::VisitCXXOperatorCallExpr(
    clang::CXXOperatorCallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

void ReplaceCallVisitor::handleCallExpr(clang::CallExpr *callExpression) {
  auto *type = callExpression->getType().getTypePtrOrNull();
  /// Real Type = float, double, long double, integer
  if (type && type->isRealType()) {
    visitor.visitRangeWithLocation(callExpression->getSourceRange());
  }
}

bool ReplaceCallVisitor::foundMutant() { return visitor.foundRange(); }

clang::SourceRange ReplaceCallVisitor::getSourceRange() {
  return visitor.getSourceRange();
}
