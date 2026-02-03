#include "ir.h"
#include "../frontend/constant.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/type.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace IR {
ConstantValue::ConstantValue(std::unique_ptr<AST::Constant> astConstant)
    : astConstant(std::move(astConstant)) {
    if (!this->astConstant) {
        throw std::invalid_argument(
            "Creating ConstantValue with null astConstant in ConstantValue");
    }
}

const AST::Constant *ConstantValue::getASTConstant() const {
    return astConstant.get();
}

void ConstantValue::setASTConstant(
    std::unique_ptr<AST::Constant> newAstConstant) {
    if (!newAstConstant) {
        throw std::invalid_argument(
            "Setting astConstant to null in setASTConstant in ConstantValue");
    }
    astConstant = std::move(newAstConstant);
}

VariableValue::VariableValue(std::string_view identifier)
    : identifier(identifier) {}

const std::string &VariableValue::getIdentifier() const { return identifier; }

void VariableValue::setIdentifier(std::string_view newIdentifier) {
    identifier = newIdentifier;
}

ReturnInstruction::ReturnInstruction(std::unique_ptr<Value> returnValue)
    : returnValue(std::move(returnValue)) {
    if (!this->returnValue) {
        throw std::invalid_argument("Creating ReturnInstruction with null "
                                    "returnValue in ReturnInstruction");
    }
}

Value *ReturnInstruction::getReturnValue() const { return returnValue.get(); }

void ReturnInstruction::setReturnValue(std::unique_ptr<Value> newReturnValue) {
    if (!newReturnValue) {
        throw std::invalid_argument(
            "Setting returnValue to null in setReturnValue "
            "in ReturnInstruction");
    }
    returnValue = std::move(newReturnValue);
}

SignExtendInstruction::SignExtendInstruction(std::unique_ptr<Value> src,
                                             std::unique_ptr<Value> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!this->src) {
        throw std::invalid_argument(
            "Creating SignExtendInstruction with null src "
            "in SignExtendInstruction");
    }
    if (!this->dst) {
        throw std::invalid_argument(
            "Creating SignExtendInstruction with null dst "
            "in SignExtendInstruction");
    }
}

Value *SignExtendInstruction::getSrc() const { return src.get(); }

Value *SignExtendInstruction::getDst() const { return dst.get(); }

void SignExtendInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::invalid_argument(
            "Setting src to null in setSrc in SignExtendInstruction");
    }
    src = std::move(newSrc);
}

void SignExtendInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::invalid_argument(
            "Setting dst to null in setDst in SignExtendInstruction");
    }
    dst = std::move(newDst);
}

TruncateInstruction::TruncateInstruction(std::unique_ptr<Value> src,
                                         std::unique_ptr<Value> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!this->src) {
        throw std::invalid_argument(
            "Creating TruncateInstruction with null src in "
            "TruncateInstruction");
    }
    if (!this->dst) {
        throw std::invalid_argument(
            "Creating TruncateInstruction with null dst in "
            "TruncateInstruction");
    }
}

Value *TruncateInstruction::getSrc() const { return src.get(); }

Value *TruncateInstruction::getDst() const { return dst.get(); }

void TruncateInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::invalid_argument(
            "Setting src to null in setSrc in TruncateInstruction");
    }
    src = std::move(newSrc);
}

void TruncateInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::invalid_argument(
            "Setting dst to null in setDst in TruncateInstruction");
    }
    dst = std::move(newDst);
}

ZeroExtendInstruction::ZeroExtendInstruction(std::unique_ptr<Value> src,
                                             std::unique_ptr<Value> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!this->src) {
        throw std::invalid_argument(
            "Creating ZeroExtendInstruction with null src "
            "in ZeroExtendInstruction");
    }
    if (!this->dst) {
        throw std::invalid_argument(
            "Creating ZeroExtendInstruction with null dst "
            "in ZeroExtendInstruction");
    }
}

Value *ZeroExtendInstruction::getSrc() const { return src.get(); }

Value *ZeroExtendInstruction::getDst() const { return dst.get(); }

void ZeroExtendInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::invalid_argument(
            "Setting src to null in setSrc in ZeroExtendInstruction");
    }
    src = std::move(newSrc);
}

void ZeroExtendInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::invalid_argument(
            "Setting dst to null in setDst in ZeroExtendInstruction");
    }
    dst = std::move(newDst);
}

UnaryInstruction::UnaryInstruction(std::unique_ptr<UnaryOperator> unaryOperator,
                                   std::unique_ptr<Value> src,
                                   std::unique_ptr<Value> dst)
    : unaryOperator(std::move(unaryOperator)), src(std::move(src)),
      dst(std::move(dst)) {
    if (!this->unaryOperator) {
        throw std::invalid_argument("Creating UnaryInstruction with null "
                                    "unaryOperator in UnaryInstruction");
    }
    if (!this->src) {
        throw std::invalid_argument(
            "Creating UnaryInstruction with null src in UnaryInstruction");
    }
    if (!this->dst) {
        throw std::invalid_argument(
            "Creating UnaryInstruction with null dst in UnaryInstruction");
    }
}

UnaryOperator *UnaryInstruction::getUnaryOperator() const {
    return unaryOperator.get();
}

Value *UnaryInstruction::getSrc() const { return src.get(); }

Value *UnaryInstruction::getDst() const { return dst.get(); }

void UnaryInstruction::setUnaryOperator(
    std::unique_ptr<UnaryOperator> newUnaryOperator) {
    if (!newUnaryOperator) {
        throw std::invalid_argument("Setting unaryOperator to null in "
                                    "setUnaryOperator in UnaryInstruction");
    }
    unaryOperator = std::move(newUnaryOperator);
}

void UnaryInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::invalid_argument(
            "Setting src to null in setSrc in UnaryInstruction");
    }
    src = std::move(newSrc);
}

void UnaryInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::invalid_argument(
            "Setting dst to null in setDst in UnaryInstruction");
    }
    dst = std::move(newDst);
}

BinaryInstruction::BinaryInstruction(
    std::unique_ptr<BinaryOperator> binaryOperator, std::unique_ptr<Value> src1,
    std::unique_ptr<Value> src2, std::unique_ptr<Value> dst)
    : binaryOperator(std::move(binaryOperator)), src1(std::move(src1)),
      src2(std::move(src2)), dst(std::move(dst)) {
    if (!this->binaryOperator) {
        throw std::invalid_argument("Creating BinaryInstruction with null "
                                    "binaryOperator in BinaryInstruction");
    }
    if (!this->src1) {
        throw std::invalid_argument(
            "Creating BinaryInstruction with null src1 in BinaryInstruction");
    }
    if (!this->src2) {
        throw std::invalid_argument(
            "Creating BinaryInstruction with null src2 in BinaryInstruction");
    }
    if (!this->dst) {
        throw std::invalid_argument(
            "Creating BinaryInstruction with null dst in BinaryInstruction");
    }
}

BinaryOperator *BinaryInstruction::getBinaryOperator() const {
    return binaryOperator.get();
}

Value *BinaryInstruction::getSrc1() const { return src1.get(); }

Value *BinaryInstruction::getSrc2() const { return src2.get(); }

Value *BinaryInstruction::getDst() const { return dst.get(); }

void BinaryInstruction::setBinaryOperator(
    std::unique_ptr<BinaryOperator> newBinaryOperator) {
    if (!newBinaryOperator) {
        throw std::invalid_argument("Setting binaryOperator to null in "
                                    "setBinaryOperator in BinaryInstruction");
    }
    binaryOperator = std::move(newBinaryOperator);
}

void BinaryInstruction::setSrc1(std::unique_ptr<Value> newSrc1) {
    if (!newSrc1) {
        throw std::invalid_argument(
            "Setting src1 to null in setSrc1 in BinaryInstruction");
    }
    src1 = std::move(newSrc1);
}

void BinaryInstruction::setSrc2(std::unique_ptr<Value> newSrc2) {
    if (!newSrc2) {
        throw std::invalid_argument(
            "Setting src2 to null in setSrc2 in BinaryInstruction");
    }
    src2 = std::move(newSrc2);
}

void BinaryInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::invalid_argument(
            "Setting dst to null in setDst in BinaryInstruction");
    }
    dst = std::move(newDst);
}

CopyInstruction::CopyInstruction(std::unique_ptr<Value> src,
                                 std::unique_ptr<Value> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!this->src) {
        throw std::invalid_argument(
            "Creating CopyInstruction with null src in CopyInstruction");
    }
    if (!this->dst) {
        throw std::invalid_argument(
            "Creating CopyInstruction with null dst in CopyInstruction");
    }
}

Value *CopyInstruction::getSrc() const { return src.get(); }

Value *CopyInstruction::getDst() const { return dst.get(); }

void CopyInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::invalid_argument(
            "Setting src to null in setSrc in CopyInstruction");
    }
    src = std::move(newSrc);
}

void CopyInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::invalid_argument(
            "Setting dst to null in setDst in CopyInstruction");
    }
    dst = std::move(newDst);
}

JumpInstruction::JumpInstruction(std::string_view target) : target(target) {}

const std::string &JumpInstruction::getTarget() const { return target; }

void JumpInstruction::setTarget(std::string_view newTarget) {
    target = newTarget;
}

JumpIfZeroInstruction::JumpIfZeroInstruction(std::unique_ptr<Value> condition,
                                             std::string_view target)
    : condition(std::move(condition)), target(target) {
    if (!this->condition) {
        throw std::invalid_argument("Creating JumpIfZeroInstruction with null "
                                    "condition in JumpIfZeroInstruction");
    }
}

Value *JumpIfZeroInstruction::getCondition() const { return condition.get(); }

const std::string &JumpIfZeroInstruction::getTarget() const { return target; }

void JumpIfZeroInstruction::setCondition(std::unique_ptr<Value> newCondition) {
    if (!newCondition) {
        throw std::invalid_argument(
            "Setting condition to null in setCondition in "
            "JumpIfZeroInstruction");
    }
    condition = std::move(newCondition);
}

void JumpIfZeroInstruction::setTarget(std::string_view newTarget) {
    target = newTarget;
}

JumpIfNotZeroInstruction::JumpIfNotZeroInstruction(
    std::unique_ptr<Value> condition, std::string_view target)
    : condition(std::move(condition)), target(target) {
    if (!this->condition) {
        throw std::invalid_argument(
            "Creating JumpIfNotZeroInstruction with null "
            "condition in JumpIfNotZeroInstruction");
    }
}

Value *JumpIfNotZeroInstruction::getCondition() const {
    return condition.get();
}

const std::string &JumpIfNotZeroInstruction::getTarget() const {
    return target;
}

void JumpIfNotZeroInstruction::setCondition(
    std::unique_ptr<Value> newCondition) {
    if (!newCondition) {
        throw std::invalid_argument(
            "Setting condition to null in setCondition in "
            "JumpIfNotZeroInstruction");
    }
    condition = std::move(newCondition);
}

void JumpIfNotZeroInstruction::setTarget(std::string_view newTarget) {
    target = newTarget;
}

LabelInstruction::LabelInstruction(std::string_view label) : label(label) {}

const std::string &LabelInstruction::getLabel() const { return label; }

void LabelInstruction::setLabel(std::string_view newLabel) { label = newLabel; }

FunctionCallInstruction::FunctionCallInstruction(
    std::string_view functionIdentifier,
    std::unique_ptr<std::vector<std::unique_ptr<Value>>> args,
    std::unique_ptr<Value> dst)
    : functionIdentifier(functionIdentifier), args(std::move(args)),
      dst(std::move(dst)) {
    if (!this->args) {
        throw std::invalid_argument(
            "Creating FunctionCallInstruction with null "
            "args in FunctionCallInstruction");
    }
    if (!this->dst) {
        throw std::invalid_argument(
            "Creating FunctionCallInstruction with null dst "
            "in FunctionCallInstruction");
    }
}

const std::string &FunctionCallInstruction::getFunctionIdentifier() const {
    return functionIdentifier;
}

const std::vector<std::unique_ptr<Value>> &
FunctionCallInstruction::getArgs() const {
    return *args;
}

Value *FunctionCallInstruction::getDst() const { return dst.get(); }

void FunctionCallInstruction::setFunctionIdentifier(
    std::string_view newFunctionIdentifier) {
    functionIdentifier = newFunctionIdentifier;
}

void FunctionCallInstruction::setArgs(
    std::unique_ptr<std::vector<std::unique_ptr<Value>>> newArgs) {
    if (!newArgs) {
        throw std::invalid_argument(
            "Setting args to null in setArgs in FunctionCallInstruction");
    }
    args = std::move(newArgs);
}

void FunctionCallInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::invalid_argument(
            "Setting dst to null in setDst in FunctionCallInstruction");
    }
    dst = std::move(newDst);
}

FunctionDefinition::FunctionDefinition(
    std::string_view functionIdentifier, bool global,
    std::unique_ptr<std::vector<std::string>> parameters,
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> functionBody)
    : functionIdentifier(functionIdentifier), global(global),
      parameters(std::move(parameters)), functionBody(std::move(functionBody)) {
    if (!this->parameters) {
        throw std::invalid_argument("Creating FunctionDefinition with null "
                                    "parameters in FunctionDefinition");
    }
    if (!this->functionBody) {
        throw std::invalid_argument("Creating FunctionDefinition with null "
                                    "functionBody in FunctionDefinition");
    }
}

const std::string &FunctionDefinition::getFunctionIdentifier() const {
    return functionIdentifier;
}

bool FunctionDefinition::isGlobal() const { return global; }

const std::vector<std::string> &
FunctionDefinition::getParameterIdentifiers() const {
    return *parameters;
}

const std::vector<std::unique_ptr<Instruction>> &
FunctionDefinition::getFunctionBody() const {
    return *functionBody;
}

void FunctionDefinition::setFunctionBody(
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>>
        newFunctionBody) {
    if (!newFunctionBody) {
        throw std::invalid_argument("Setting functionBody to null in "
                                    "setFunctionBody in FunctionDefinition");
    }
    functionBody = std::move(newFunctionBody);
}

StaticVariable::StaticVariable(std::string_view identifier, bool global,
                               std::unique_ptr<AST::Type> type,
                               std::unique_ptr<AST::StaticInit> staticInit)
    : identifier(identifier), global(global), type(std::move(type)),
      staticInit(std::move(staticInit)) {
    if (!this->type) {
        throw std::invalid_argument(
            "Creating StaticVariable with null type in StaticVariable");
    }
    if (!this->staticInit) {
        throw std::invalid_argument(
            "Creating StaticVariable with null staticInit in StaticVariable");
    }
}

const std::string &StaticVariable::getIdentifier() const { return identifier; }

bool StaticVariable::isGlobal() const { return global; }

const AST::Type *StaticVariable::getType() const { return type.get(); }

const AST::StaticInit *StaticVariable::getStaticInit() const {
    return staticInit.get();
}

Program::Program(
    std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels)
    : topLevels(std::move(topLevels)) {
    if (!this->topLevels) {
        throw std::invalid_argument(
            "Creating Program with null topLevels in Program");
    }
}

const std::vector<std::unique_ptr<TopLevel>> &Program::getTopLevels() const {
    return *topLevels;
}

std::vector<std::unique_ptr<TopLevel>> &Program::getTopLevels() {
    return *topLevels;
}
} // namespace IR
