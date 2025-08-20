#include "ir.h"
#include <stdexcept>

namespace IR {
ConstantValue::ConstantValue(std::unique_ptr<AST::Constant> astConstant)
    : astConstant(std::move(astConstant)) {
    if (!astConstant) {
        throw std::logic_error("Creating ConstantValue with null astConstant");
    }
}

ConstantValue::~ConstantValue() = default;

std::unique_ptr<AST::Constant> &ConstantValue::getASTConstant() {
    return astConstant;
}

void ConstantValue::setASTConstant(
    std::unique_ptr<AST::Constant> newAstConstant) {
    if (!newAstConstant) {
        throw std::logic_error("Setting ConstantValue astConstant to null");
    }
    this->astConstant = std::move(newAstConstant);
}

VariableValue::VariableValue(std::string_view identifier)
    : identifier(identifier) {}

VariableValue::~VariableValue() = default;

const std::string &VariableValue::getIdentifier() const { return identifier; }

void VariableValue::setIdentifier(std::string_view newIdentifier) {
    this->identifier = newIdentifier;
}

ReturnInstruction::ReturnInstruction(std::unique_ptr<Value> returnValue)
    : returnValue(std::move(returnValue)) {
    if (!returnValue) {
        throw std::logic_error(
            "Creating ReturnInstruction with null returnValue");
    }
}

std::unique_ptr<Value> &ReturnInstruction::getReturnValue() {
    return returnValue;
}

void ReturnInstruction::setReturnValue(std::unique_ptr<Value> newReturnValue) {
    if (!newReturnValue) {
        throw std::logic_error("Setting ReturnInstruction returnValue to null");
    }
    this->returnValue = std::move(newReturnValue);
}

SignExtendInstruction::SignExtendInstruction(std::unique_ptr<Value> src,
                                             std::unique_ptr<Value> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!src) {
        throw std::logic_error("Creating SignExtendInstruction with null src");
    }
    if (!dst) {
        throw std::logic_error("Creating SignExtendInstruction with null dst");
    }
}

std::unique_ptr<Value> &SignExtendInstruction::getSrc() { return src; }

std::unique_ptr<Value> &SignExtendInstruction::getDst() { return dst; }

void SignExtendInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::logic_error("Setting SignExtendInstruction src to null");
    }
    this->src = std::move(newSrc);
}

void SignExtendInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::logic_error("Setting SignExtendInstruction dst to null");
    }
    this->dst = std::move(newDst);
}

TruncateInstruction::TruncateInstruction(std::unique_ptr<Value> src,
                                         std::unique_ptr<Value> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!src) {
        throw std::logic_error("Creating TruncateInstruction with null src");
    }
    if (!dst) {
        throw std::logic_error("Creating TruncateInstruction with null dst");
    }
}

std::unique_ptr<Value> &TruncateInstruction::getSrc() { return src; }

std::unique_ptr<Value> &TruncateInstruction::getDst() { return dst; }

void TruncateInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::logic_error("Setting TruncateInstruction src to null");
    }
    this->src = std::move(newSrc);
}

void TruncateInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::logic_error("Setting TruncateInstruction dst to null");
    }
    this->dst = std::move(newDst);
}

UnaryInstruction::UnaryInstruction(std::unique_ptr<UnaryOperator> unaryOperator,
                                   std::unique_ptr<Value> src,
                                   std::unique_ptr<Value> dst)
    : unaryOperator(std::move(unaryOperator)), src(std::move(src)),
      dst(std::move(dst)) {
    if (!unaryOperator) {
        throw std::logic_error(
            "Creating UnaryInstruction with null unaryOperator");
    }
    if (!src) {
        throw std::logic_error("Creating UnaryInstruction with null src");
    }
    if (!dst) {
        throw std::logic_error("Creating UnaryInstruction with null dst");
    }
}

std::unique_ptr<UnaryOperator> &UnaryInstruction::getUnaryOperator() {
    return unaryOperator;
}

std::unique_ptr<Value> &UnaryInstruction::getSrc() { return src; }

std::unique_ptr<Value> &UnaryInstruction::getDst() { return dst; }

void UnaryInstruction::setUnaryOperator(
    std::unique_ptr<UnaryOperator> newUnaryOperator) {
    if (!newUnaryOperator) {
        throw std::logic_error(
            "Setting UnaryInstruction unaryOperator to null");
    }
    this->unaryOperator = std::move(newUnaryOperator);
}

void UnaryInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::logic_error("Setting UnaryInstruction src to null");
    }
    this->src = std::move(newSrc);
}

void UnaryInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::logic_error("Setting UnaryInstruction dst to null");
    }
    this->dst = std::move(newDst);
}

BinaryInstruction::BinaryInstruction(
    std::unique_ptr<BinaryOperator> binaryOperator, std::unique_ptr<Value> src1,
    std::unique_ptr<Value> src2, std::unique_ptr<Value> dst)
    : binaryOperator(std::move(binaryOperator)), src1(std::move(src1)),
      src2(std::move(src2)), dst(std::move(dst)) {
    if (!binaryOperator) {
        throw std::logic_error(
            "Creating BinaryInstruction with null binaryOperator");
    }
    if (!src1) {
        throw std::logic_error("Creating BinaryInstruction with null src1");
    }
    if (!src2) {
        throw std::logic_error("Creating BinaryInstruction with null src2");
    }
    if (!dst) {
        throw std::logic_error("Creating BinaryInstruction with null dst");
    }
}

std::unique_ptr<BinaryOperator> &BinaryInstruction::getBinaryOperator() {
    return binaryOperator;
}

std::unique_ptr<Value> &BinaryInstruction::getSrc1() { return src1; }

std::unique_ptr<Value> &BinaryInstruction::getSrc2() { return src2; }

std::unique_ptr<Value> &BinaryInstruction::getDst() { return dst; }

void BinaryInstruction::setBinaryOperator(
    std::unique_ptr<BinaryOperator> newBinaryOperator) {
    if (!newBinaryOperator) {
        throw std::logic_error(
            "Setting BinaryInstruction binaryOperator to null");
    }
    this->binaryOperator = std::move(newBinaryOperator);
}

void BinaryInstruction::setSrc1(std::unique_ptr<Value> newSrc1) {
    if (!newSrc1) {
        throw std::logic_error("Setting BinaryInstruction src1 to null");
    }
    this->src1 = std::move(newSrc1);
}

void BinaryInstruction::setSrc2(std::unique_ptr<Value> newSrc2) {
    if (!newSrc2) {
        throw std::logic_error("Setting BinaryInstruction src2 to null");
    }
    this->src2 = std::move(newSrc2);
}

void BinaryInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::logic_error("Setting BinaryInstruction dst to null");
    }
    this->dst = std::move(newDst);
}

CopyInstruction::CopyInstruction(std::unique_ptr<Value> src,
                                 std::unique_ptr<Value> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!src) {
        throw std::logic_error("Creating CopyInstruction with null src");
    }
    if (!dst) {
        throw std::logic_error("Creating CopyInstruction with null dst");
    }
}

std::unique_ptr<Value> &CopyInstruction::getSrc() { return src; }

std::unique_ptr<Value> &CopyInstruction::getDst() { return dst; }

void CopyInstruction::setSrc(std::unique_ptr<Value> newSrc) {
    if (!newSrc) {
        throw std::logic_error("Setting CopyInstruction src to null");
    }
    this->src = std::move(newSrc);
}

void CopyInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::logic_error("Setting CopyInstruction dst to null");
    }
    this->dst = std::move(newDst);
}

JumpInstruction::JumpInstruction(std::string_view target) : target(target) {}

const std::string &JumpInstruction::getTarget() const { return target; }

void JumpInstruction::setTarget(std::string_view newTarget) {
    this->target = newTarget;
}

JumpIfZeroInstruction::JumpIfZeroInstruction(std::unique_ptr<Value> condition,
                                             std::string_view target)
    : condition(std::move(condition)), target(target) {
    if (!condition) {
        throw std::logic_error(
            "Creating JumpIfZeroInstruction with null condition");
    }
}

std::unique_ptr<Value> &JumpIfZeroInstruction::getCondition() {
    return condition;
}

const std::string &JumpIfZeroInstruction::getTarget() const { return target; }

void JumpIfZeroInstruction::setCondition(std::unique_ptr<Value> newCondition) {
    if (!newCondition) {
        throw std::logic_error(
            "Setting JumpIfZeroInstruction condition to null");
    }
    this->condition = std::move(newCondition);
}

void JumpIfZeroInstruction::setTarget(std::string_view newTarget) {
    this->target = newTarget;
}

JumpIfNotZeroInstruction::JumpIfNotZeroInstruction(
    std::unique_ptr<Value> condition, std::string_view target)
    : condition(std::move(condition)), target(target) {
    if (!condition) {
        throw std::logic_error(
            "Creating JumpIfNotZeroInstruction with null condition");
    }
}

std::unique_ptr<Value> &JumpIfNotZeroInstruction::getCondition() {
    return condition;
}

const std::string &JumpIfNotZeroInstruction::getTarget() const {
    return target;
}

void JumpIfNotZeroInstruction::setCondition(
    std::unique_ptr<Value> newCondition) {
    if (!newCondition) {
        throw std::logic_error(
            "Setting JumpIfNotZeroInstruction condition to null");
    }
    this->condition = std::move(newCondition);
}

void JumpIfNotZeroInstruction::setTarget(std::string_view newTarget) {
    this->target = newTarget;
}

LabelInstruction::LabelInstruction(std::string_view label) : label(label) {}

const std::string &LabelInstruction::getLabel() const { return label; }

void LabelInstruction::setLabel(std::string_view newLabel) {
    this->label = newLabel;
}

FunctionCallInstruction::FunctionCallInstruction(
    std::string_view functionIdentifier,
    std::unique_ptr<std::vector<std::unique_ptr<Value>>> args,
    std::unique_ptr<Value> dst)
    : functionIdentifier(functionIdentifier), args(std::move(args)),
      dst(std::move(dst)) {
    if (!args) {
        throw std::logic_error(
            "Creating FunctionCallInstruction with null args");
    }
    if (!dst) {
        throw std::logic_error(
            "Creating FunctionCallInstruction with null dst");
    }
}

const std::string &FunctionCallInstruction::getFunctionIdentifier() const {
    return functionIdentifier;
}

const std::unique_ptr<std::vector<std::unique_ptr<Value>>> &
FunctionCallInstruction::getArgs() const {
    return args;
}

std::unique_ptr<Value> &FunctionCallInstruction::getDst() { return dst; }

void FunctionCallInstruction::setFunctionIdentifier(
    std::string_view newFunctionIdentifier) {
    this->functionIdentifier = newFunctionIdentifier;
}

void FunctionCallInstruction::setArgs(
    std::unique_ptr<std::vector<std::unique_ptr<Value>>> newArgs) {
    if (!newArgs) {
        throw std::logic_error("Setting FunctionCallInstruction args to null");
    }
    this->args = std::move(newArgs);
}

void FunctionCallInstruction::setDst(std::unique_ptr<Value> newDst) {
    if (!newDst) {
        throw std::logic_error("Setting FunctionCallInstruction dst to null");
    }
    this->dst = std::move(newDst);
}

FunctionDefinition::FunctionDefinition(
    std::string_view functionIdentifier, bool global,
    std::unique_ptr<std::vector<std::string>> parameters,
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> functionBody)
    : functionIdentifier(functionIdentifier), global(global),
      parameters(std::move(parameters)), functionBody(std::move(functionBody)) {
    if (!parameters) {
        throw std::logic_error(
            "Creating FunctionDefinition with null parameters");
    }
    if (!functionBody) {
        throw std::logic_error(
            "Creating FunctionDefinition with null functionBody");
    }
}

const std::string &FunctionDefinition::getFunctionIdentifier() const {
    return functionIdentifier;
}

bool FunctionDefinition::isGlobal() const { return global; }

const std::unique_ptr<std::vector<std::string>> &
FunctionDefinition::getParameterIdentifiers() const {
    return parameters;
}

const std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> &
FunctionDefinition::getFunctionBody() const {
    return functionBody;
}

void FunctionDefinition::setFunctionBody(
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>>
        newFunctionBody) {
    if (!newFunctionBody) {
        throw std::logic_error(
            "Setting FunctionDefinition functionBody to null");
    }
    this->functionBody = std::move(newFunctionBody);
}

StaticVariable::StaticVariable(std::string_view identifier, bool global,
                               std::unique_ptr<AST::Type> type,
                               std::unique_ptr<AST::StaticInit> staticInit)
    : identifier(identifier), global(global), type(std::move(type)),
      staticInit(std::move(staticInit)) {
    if (!type) {
        throw std::logic_error("Creating StaticVariable with null type");
    }
    if (!staticInit) {
        throw std::logic_error("Creating StaticVariable with null staticInit");
    }
}

const std::string &StaticVariable::getIdentifier() const { return identifier; }

bool StaticVariable::isGlobal() const { return global; }

std::unique_ptr<AST::Type> &StaticVariable::getType() { return type; }

std::unique_ptr<AST::StaticInit> &StaticVariable::getStaticInit() {
    return staticInit;
}

Program::Program(
    std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels)
    : topLevels(std::move(topLevels)) {
    if (!topLevels) {
        throw std::logic_error("Creating Program with null topLevels");
    }
}

std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> &
Program::getTopLevels() {
    return topLevels;
}
} // namespace IR
