#include "assembly.h"

namespace Assembly {
unsigned long Operand::getImmediate() const {
    throw std::logic_error("Operand is not an immediate");
}

Register *Operand::getRegister() const {
    throw std::logic_error("Operand is not a register");
}

ReservedRegister *Operand::getReservedRegister() const {
    throw std::logic_error("Operand is not a reserved register");
}

std::string Operand::getPseudoRegister() const {
    throw std::logic_error("Operand is not a pseudo register");
}

int Operand::getOffset() const {
    throw std::logic_error("Operand is not a stack (operand)");
}

std::string Operand::getIdentifier() const {
    throw std::logic_error("Operand is not a data (operand)");
}

ImmediateOperand::ImmediateOperand(int imm)
    : imm(static_cast<unsigned long>(static_cast<long>(imm))) {}

ImmediateOperand::ImmediateOperand(long imm)
    : imm(static_cast<unsigned long>(imm)) {}

ImmediateOperand::ImmediateOperand(unsigned int imm) : imm(imm) {}

ImmediateOperand::ImmediateOperand(unsigned long imm) : imm(imm) {}

unsigned long ImmediateOperand::getImmediate() const { return imm; }

RegisterOperand::RegisterOperand(std::unique_ptr<Register> reg)
    : reg(std::move(reg)) {
    if (!this->reg) {
        throw std::logic_error("Creating RegisterOperand with null reg");
    }
}

RegisterOperand::RegisterOperand(std::string regInStr) {
    if (regInStr == "AX") {
        reg = std::make_unique<AX>();
    }
    else if (regInStr == "CX") {
        reg = std::make_unique<CX>();
    }
    else if (regInStr == "DX") {
        reg = std::make_unique<DX>();
    }
    else if (regInStr == "DI") {
        reg = std::make_unique<DI>();
    }
    else if (regInStr == "SI") {
        reg = std::make_unique<SI>();
    }
    else if (regInStr == "R8") {
        reg = std::make_unique<R8>();
    }
    else if (regInStr == "R9") {
        reg = std::make_unique<R9>();
    }
    else if (regInStr == "R10") {
        reg = std::make_unique<R10>();
    }
    else if (regInStr == "R11") {
        reg = std::make_unique<R11>();
    }
    else if (regInStr == "RSP") {
        reg = std::make_unique<SP>();
    }
    else {
        throw std::logic_error("Unsupported register while creating "
                               "RegisterOperand: " +
                               regInStr);
    }
}

Register *RegisterOperand::getRegister() const { return reg.get(); }

std::string RegisterOperand::getRegisterInBytesInStr(int size) const {
    auto sizeIt = regMappings.find(size);
    if (sizeIt == regMappings.end()) {
        throw std::logic_error("Unsupported register size while getting "
                               "register in bytes in string: " +
                               std::to_string(size));
    }
    const auto &sizeMappings = sizeIt->second;
    auto &r = *reg.get();
    auto regIt = sizeMappings.find(typeid(r));
    if (regIt == sizeMappings.end()) {
        throw std::logic_error("Unsupported register type while getting "
                               "register in bytes in string: " +
                               std::string(typeid(r).name()));
    }
    return regIt->second;
}

PseudoRegisterOperand::PseudoRegisterOperand(std::string pseudoReg)
    : pseudoReg(pseudoReg) {}

std::string PseudoRegisterOperand::getPseudoRegister() const {
    return pseudoReg;
}

StackOperand::StackOperand(int offset,
                           std::unique_ptr<ReservedRegister> reservedReg)
    : offset(offset), reservedReg(std::move(reservedReg)) {
    if (!this->reservedReg) {
        throw std::logic_error("Creating StackOperand with null reservedReg");
    }
}

int StackOperand::getOffset() const { return offset; }

ReservedRegister *StackOperand::getReservedRegister() const {
    return reservedReg.get();
}

std::string StackOperand::getReservedRegisterInStr() const {
    if (dynamic_cast<SP *>(reservedReg.get())) {
        return "%rsp";
    }
    else if (dynamic_cast<BP *>(reservedReg.get())) {
        return "%rbp";
    }
    else {
        auto reg = reservedReg.get();
        throw std::logic_error("Unsupported reserved register while "
                               "getting reserved register in string: " +
                               std::string(typeid(*reg).name()));
    }
}

DataOperand::DataOperand(std::string identifier) : identifier(identifier) {}

std::string DataOperand::getIdentifier() const { return identifier; }

MovInstruction::MovInstruction(std::unique_ptr<AssemblyType> type,
                               std::unique_ptr<Operand> src,
                               std::unique_ptr<Operand> dst)
    : type(std::move(type)), src(std::move(src)), dst(std::move(dst)) {
    if (!this->type) {
        throw std::logic_error("Creating MovInstruction with null type");
    }
    if (!this->src) {
        throw std::logic_error("Creating MovInstruction with null src");
    }
    if (!this->dst) {
        throw std::logic_error("Creating MovInstruction with null dst");
    }
}

const AssemblyType *MovInstruction::getType() const { return type.get(); }

const Operand *MovInstruction::getSrc() const { return src.get(); }

const Operand *MovInstruction::getDst() const { return dst.get(); }

void MovInstruction::setType(std::unique_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in MovInstruction");
    }
    this->type = std::move(newType);
}

void MovInstruction::setSrc(std::unique_ptr<Operand> newSrc) {
    if (!newSrc) {
        throw std::logic_error("Setting null source operand in MovInstruction");
    }
    this->src = std::move(newSrc);
}

void MovInstruction::setDst(std::unique_ptr<Operand> newDst) {
    if (!newDst) {
        throw std::logic_error(
            "Setting null destination operand in MovInstruction");
    }
    this->dst = std::move(newDst);
}

MovsxInstruction::MovsxInstruction(std::unique_ptr<Operand> src,
                                   std::unique_ptr<Operand> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!this->src) {
        throw std::logic_error("Creating MovsxInstruction with null src");
    }
    if (!this->dst) {
        throw std::logic_error("Creating MovsxInstruction with null dst");
    }
}

const Operand *MovsxInstruction::getSrc() const { return src.get(); }

const Operand *MovsxInstruction::getDst() const { return dst.get(); }

void MovsxInstruction::setSrc(std::unique_ptr<Operand> newSrc) {
    if (!newSrc) {
        throw std::logic_error(
            "Setting null source operand in MovsxInstruction");
    }
    this->src = std::move(newSrc);
}

void MovsxInstruction::setDst(std::unique_ptr<Operand> newDst) {
    if (!newDst) {
        throw std::logic_error(
            "Setting null destination operand in MovsxInstruction");
    }
    this->dst = std::move(newDst);
}

MovZeroExtendInstruction::MovZeroExtendInstruction(std::unique_ptr<Operand> src,
                                                   std::unique_ptr<Operand> dst)
    : src(std::move(src)), dst(std::move(dst)) {
    if (!this->src) {
        throw std::logic_error(
            "Creating MovZeroExtendInstruction with null src");
    }
    if (!this->dst) {
        throw std::logic_error(
            "Creating MovZeroExtendInstruction with null dst");
    }
}

const Operand *MovZeroExtendInstruction::getSrc() const { return src.get(); }

const Operand *MovZeroExtendInstruction::getDst() const { return dst.get(); }

void MovZeroExtendInstruction::setSrc(std::unique_ptr<Operand> newSrc) {
    if (!newSrc) {
        throw std::logic_error(
            "Setting null source operand in MovZeroExtendInstruction");
    }
    this->src = std::move(newSrc);
}

void MovZeroExtendInstruction::setDst(std::unique_ptr<Operand> newDst) {
    if (!newDst) {
        throw std::logic_error(
            "Setting null destination operand in MovZeroExtendInstruction");
    }
    this->dst = std::move(newDst);
}

UnaryInstruction::UnaryInstruction(std::unique_ptr<UnaryOperator> unaryOperator,
                                   std::unique_ptr<AssemblyType> type,
                                   std::unique_ptr<Operand> operand)
    : unaryOperator(std::move(unaryOperator)), type(std::move(type)),
      operand(std::move(operand)) {
    if (!this->unaryOperator) {
        throw std::logic_error(
            "Creating UnaryInstruction with null unaryOperator");
    }
    if (!this->type) {
        throw std::logic_error("Creating UnaryInstruction with null type");
    }
    if (!this->operand) {
        throw std::logic_error("Creating UnaryInstruction with null operand");
    }
}

const UnaryOperator *UnaryInstruction::getUnaryOperator() const {
    return unaryOperator.get();
}

const AssemblyType *UnaryInstruction::getType() const { return type.get(); }

const Operand *UnaryInstruction::getOperand() const { return operand.get(); }

void UnaryInstruction::setUnaryOperator(
    std::unique_ptr<UnaryOperator> newUnaryOperator) {
    if (!newUnaryOperator) {
        throw std::logic_error(
            "Setting null unary operator in UnaryInstruction");
    }
    this->unaryOperator = std::move(newUnaryOperator);
}

void UnaryInstruction::setType(std::unique_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in UnaryInstruction");
    }
    this->type = std::move(newType);
}

void UnaryInstruction::setOperand(std::unique_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in UnaryInstruction");
    }
    this->operand = std::move(newOperand);
}

BinaryInstruction::BinaryInstruction(
    std::unique_ptr<BinaryOperator> binaryOperator,
    std::unique_ptr<AssemblyType> type, std::unique_ptr<Operand> operand1,
    std::unique_ptr<Operand> operand2)
    : binaryOperator(std::move(binaryOperator)), type(std::move(type)),
      operand1(std::move(operand1)), operand2(std::move(operand2)) {
    if (!this->binaryOperator) {
        throw std::logic_error(
            "Creating BinaryInstruction with null binaryOperator");
    }
    if (!this->type) {
        throw std::logic_error("Creating BinaryInstruction with null type");
    }
    if (!this->operand1) {
        throw std::logic_error("Creating BinaryInstruction with null operand1");
    }
    if (!this->operand2) {
        throw std::logic_error("Creating BinaryInstruction with null operand2");
    }
}

const BinaryOperator *BinaryInstruction::getBinaryOperator() const {
    return binaryOperator.get();
}

const AssemblyType *BinaryInstruction::getType() const { return type.get(); }

const Operand *BinaryInstruction::getOperand1() const { return operand1.get(); }

const Operand *BinaryInstruction::getOperand2() const { return operand2.get(); }

void BinaryInstruction::setBinaryOperator(
    std::unique_ptr<BinaryOperator> newBinaryOperator) {
    if (!newBinaryOperator) {
        throw std::logic_error(
            "Setting null binary operator in BinaryInstruction");
    }
    this->binaryOperator = std::move(newBinaryOperator);
}

void BinaryInstruction::setType(std::unique_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in BinaryInstruction");
    }
    this->type = std::move(newType);
}

void BinaryInstruction::setOperand1(std::unique_ptr<Operand> newOperand1) {
    if (!newOperand1) {
        throw std::logic_error("Setting null operand1 in BinaryInstruction");
    }
    this->operand1 = std::move(newOperand1);
}

void BinaryInstruction::setOperand2(std::unique_ptr<Operand> newOperand2) {
    if (!newOperand2) {
        throw std::logic_error("Setting null operand2 in BinaryInstruction");
    }
    this->operand2 = std::move(newOperand2);
}

CmpInstruction::CmpInstruction(std::unique_ptr<AssemblyType> type,
                               std::unique_ptr<Operand> operand1,
                               std::unique_ptr<Operand> operand2)
    : type(std::move(type)), operand1(std::move(operand1)),
      operand2(std::move(operand2)) {
    if (!this->type) {
        throw std::logic_error("Creating CmpInstruction with null type");
    }
    if (!this->operand1) {
        throw std::logic_error("Creating CmpInstruction with null operand1");
    }
    if (!this->operand2) {
        throw std::logic_error("Creating CmpInstruction with null operand2");
    }
}

const AssemblyType *CmpInstruction::getType() const { return type.get(); }

const Operand *CmpInstruction::getOperand1() const { return operand1.get(); }

const Operand *CmpInstruction::getOperand2() const { return operand2.get(); }

void CmpInstruction::setType(std::unique_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in CmpInstruction");
    }
    this->type = std::move(newType);
}

void CmpInstruction::setOperand1(std::unique_ptr<Operand> newOperand1) {
    if (!newOperand1) {
        throw std::logic_error("Setting null operand1 in CmpInstruction");
    }
    this->operand1 = std::move(newOperand1);
}

void CmpInstruction::setOperand2(std::unique_ptr<Operand> newOperand2) {
    if (!newOperand2) {
        throw std::logic_error("Setting null operand2 in CmpInstruction");
    }
    this->operand2 = std::move(newOperand2);
}

IdivInstruction::IdivInstruction(std::unique_ptr<AssemblyType> type,
                                 std::unique_ptr<Operand> operand)
    : type(std::move(type)), operand(std::move(operand)) {
    if (!this->type) {
        throw std::logic_error("Creating IdivInstruction with null type");
    }
    if (!this->operand) {
        throw std::logic_error("Creating IdivInstruction with null operand");
    }
}

const AssemblyType *IdivInstruction::getType() const { return type.get(); }

const Operand *IdivInstruction::getOperand() const { return operand.get(); }

void IdivInstruction::setType(std::unique_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in IdivInstruction");
    }
    this->type = std::move(newType);
}

void IdivInstruction::setOperand(std::unique_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in IdivInstruction");
    }
    this->operand = std::move(newOperand);
}

DivInstruction::DivInstruction(std::unique_ptr<AssemblyType> type,
                               std::unique_ptr<Operand> operand)
    : type(std::move(type)), operand(std::move(operand)) {
    if (!this->type) {
        throw std::logic_error("Creating DivInstruction with null type");
    }
    if (!this->operand) {
        throw std::logic_error("Creating DivInstruction with null operand");
    }
}

const AssemblyType *DivInstruction::getType() const { return type.get(); }

const Operand *DivInstruction::getOperand() const { return operand.get(); }

void DivInstruction::setType(std::unique_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in DivInstruction");
    }
    this->type = std::move(newType);
}

void DivInstruction::setOperand(std::unique_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in DivInstruction");
    }
    this->operand = std::move(newOperand);
}

CdqInstruction::CdqInstruction(std::unique_ptr<AssemblyType> type)
    : type(std::move(type)) {}

const AssemblyType *CdqInstruction::getType() const { return type.get(); }

void CdqInstruction::setType(std::unique_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in CdqInstruction");
    }
    this->type = std::move(newType);
}

JmpInstruction::JmpInstruction(std::string label) : label(label) {}

std::string JmpInstruction::getLabel() const { return label; }

void JmpInstruction::setLabel(std::string newLabel) { this->label = newLabel; }

JmpCCInstruction::JmpCCInstruction(std::unique_ptr<CondCode> condCode,
                                   std::string label)
    : condCode(std::move(condCode)), label(label) {
    if (!this->condCode) {
        throw std::logic_error("Creating JmpCCInstruction with null condCode");
    }
}

const CondCode *JmpCCInstruction::getCondCode() const { return condCode.get(); }

std::string JmpCCInstruction::getLabel() const { return label; }

void JmpCCInstruction::setCondCode(std::unique_ptr<CondCode> newCondCode) {
    if (!newCondCode) {
        throw std::logic_error(
            "Setting null condition code in JmpCCInstruction");
    }
    this->condCode = std::move(newCondCode);
}

void JmpCCInstruction::setLabel(std::string newLabel) {
    this->label = newLabel;
}

SetCCInstruction::SetCCInstruction(std::unique_ptr<CondCode> condCode,
                                   std::unique_ptr<Operand> operand)
    : condCode(std::move(condCode)), operand(std::move(operand)) {
    if (!this->condCode) {
        throw std::logic_error("Creating SetCCInstruction with null condCode");
    }
    if (!this->operand) {
        throw std::logic_error("Creating SetCCInstruction with null operand");
    }
}

const CondCode *SetCCInstruction::getCondCode() const { return condCode.get(); }

const Operand *SetCCInstruction::getOperand() const { return operand.get(); }

void SetCCInstruction::setCondCode(std::unique_ptr<CondCode> newCondCode) {
    if (!newCondCode) {
        throw std::logic_error(
            "Setting null condition code in SetCCInstruction");
    }
    this->condCode = std::move(newCondCode);
}

void SetCCInstruction::setOperand(std::unique_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in SetCCInstruction");
    }
    this->operand = std::move(newOperand);
}

LabelInstruction::LabelInstruction(std::string label) : label(label) {}

std::string LabelInstruction::getLabel() const { return label; }

void LabelInstruction::setLabel(std::string newLabel) {
    this->label = newLabel;
}

PushInstruction::PushInstruction(std::unique_ptr<Operand> operand)
    : operand(std::move(operand)) {
    if (!this->operand) {
        throw std::logic_error("Creating PushInstruction with null operand");
    }
}

const Operand *PushInstruction::getOperand() const { return operand.get(); }

void PushInstruction::setOperand(std::unique_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in PushInstruction");
    }
    this->operand = std::move(newOperand);
}

CallInstruction::CallInstruction(std::string functionIdentifier)
    : functionIdentifier(functionIdentifier) {}

std::string CallInstruction::getFunctionIdentifier() const {
    return functionIdentifier;
}

FunctionDefinition::FunctionDefinition(
    std::string functionIdentifier, bool global,
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> functionBody,
    size_t stackSize)
    : functionIdentifier(functionIdentifier), global(global),
      functionBody(std::move(functionBody)), stackSize(stackSize) {
    if (!this->functionBody) {
        throw std::logic_error(
            "Creating FunctionDefinition with null functionBody");
    }
}

std::string FunctionDefinition::getFunctionIdentifier() const {
    return functionIdentifier;
}

bool FunctionDefinition::isGlobal() const { return global; }

const std::vector<std::unique_ptr<Instruction>> &
FunctionDefinition::getFunctionBody() const {
    return *functionBody;
}

std::vector<std::unique_ptr<Instruction>> &
FunctionDefinition::getFunctionBody() {
    return *functionBody;
}

void FunctionDefinition::setFunctionBody(
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>>
        newFunctionBody) {
    if (!newFunctionBody) {
        throw std::logic_error(
            "Setting null function body in FunctionDefinition");
    }
    this->functionBody = std::move(newFunctionBody);
}

size_t FunctionDefinition::getStackSize() const { return stackSize; }

void FunctionDefinition::setStackSize(size_t newStackSize) {
    this->stackSize = newStackSize;
}

StaticVariable::StaticVariable(std::string identifier, bool global,
                               int alignment,
                               std::unique_ptr<AST::StaticInit> staticInit)
    : identifier(identifier), global(global), alignment(alignment),
      staticInit(std::move(staticInit)) {
    if (!this->staticInit) {
        throw std::logic_error("Creating StaticVariable with null staticInit");
    }
}

std::string StaticVariable::getIdentifier() const { return identifier; }

bool StaticVariable::isGlobal() const { return global; }

int StaticVariable::getAlignment() const { return alignment; }

void StaticVariable::setAlignment(int newAlignment) {
    this->alignment = newAlignment;
}

const AST::StaticInit *StaticVariable::getStaticInit() const {
    return staticInit.get();
}

void StaticVariable::setStaticInit(
    std::unique_ptr<AST::StaticInit> newStaticInit) {
    if (!newStaticInit) {
        throw std::logic_error("Setting null static init in StaticVariable");
    }
    this->staticInit = std::move(newStaticInit);
}

Program::Program(
    std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels)
    : topLevels(std::move(topLevels)) {}

const std::vector<std::unique_ptr<TopLevel>> &Program::getTopLevels() const {
    return *topLevels;
}

std::vector<std::unique_ptr<TopLevel>> &Program::getTopLevels() {
    return *topLevels;
}

void Program::setTopLevels(
    std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> newTopLevels) {
    if (!newTopLevels) {
        throw std::logic_error("Setting null top levels in Program");
    }
    this->topLevels = std::move(newTopLevels);
}

} // namespace Assembly
