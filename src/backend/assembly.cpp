#include "assembly.h"

namespace Assembly {
int Operand::getImmediate() const {
    throw std::logic_error("Operand is not an immediate");
}

std::shared_ptr<Register> Operand::getRegister() const {
    throw std::logic_error("Operand is not a register");
}

std::shared_ptr<ReservedRegister> Operand::getReservedRegister() const {
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

ImmediateOperand::ImmediateOperand(int imm) : imm(static_cast<long>(imm)) {}

ImmediateOperand::ImmediateOperand(long imm) : imm(imm) {}

int ImmediateOperand::getImmediate() const { return static_cast<int>(imm); }

long ImmediateOperand::getImmediateLong() const { return imm; }

RegisterOperand::RegisterOperand(std::shared_ptr<Register> reg) : reg(reg) {
    if (!reg) {
        throw std::logic_error("Creating RegisterOperand with null reg");
    }
}

RegisterOperand::RegisterOperand(std::string regInStr) {
    if (regInStr == "AX") {
        reg = std::make_shared<AX>();
    }
    else if (regInStr == "CX") {
        reg = std::make_shared<CX>();
    }
    else if (regInStr == "DX") {
        reg = std::make_shared<DX>();
    }
    else if (regInStr == "DI") {
        reg = std::make_shared<DI>();
    }
    else if (regInStr == "SI") {
        reg = std::make_shared<SI>();
    }
    else if (regInStr == "R8") {
        reg = std::make_shared<R8>();
    }
    else if (regInStr == "R9") {
        reg = std::make_shared<R9>();
    }
    else if (regInStr == "R10") {
        reg = std::make_shared<R10>();
    }
    else if (regInStr == "R11") {
        reg = std::make_shared<R11>();
    }
    else if (regInStr == "RSP") {
        reg = std::make_shared<SP>();
    }
    else {
        throw std::logic_error("Unsupported register while creating "
                               "RegisterOperand: " +
                               regInStr);
    }
}

std::shared_ptr<Register> RegisterOperand::getRegister() const { return reg; }

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
                           std::shared_ptr<ReservedRegister> reservedReg)
    : offset(offset), reservedReg(reservedReg) {
    if (!reservedReg) {
        throw std::logic_error("Creating StackOperand with null reservedReg");
    }
}

int StackOperand::getOffset() const { return offset; }

std::shared_ptr<ReservedRegister> StackOperand::getReservedRegister() const {
    return reservedReg;
}

std::string StackOperand::getReservedRegisterInStr() const {
    if (auto sp = std::dynamic_pointer_cast<SP>(reservedReg)) {
        return "%rsp";
    }
    else if (auto rsp = std::dynamic_pointer_cast<SP>(reservedReg)) {
        return "%rsp";
    }
    else if (auto bp = std::dynamic_pointer_cast<BP>(reservedReg)) {
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

MovInstruction::MovInstruction(std::shared_ptr<AssemblyType> type,
                               std::shared_ptr<Operand> src,
                               std::shared_ptr<Operand> dst)
    : type(type), src(src), dst(dst) {
    if (!type) {
        throw std::logic_error("Creating MovInstruction with null type");
    }
    if (!src) {
        throw std::logic_error("Creating MovInstruction with null src");
    }
    if (!dst) {
        throw std::logic_error("Creating MovInstruction with null dst");
    }
}

std::shared_ptr<AssemblyType> MovInstruction::getType() { return type; }

std::shared_ptr<Operand> MovInstruction::getSrc() { return src; }

std::shared_ptr<Operand> MovInstruction::getDst() { return dst; }

void MovInstruction::setType(std::shared_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in MovInstruction");
    }
    this->type = newType;
}

void MovInstruction::setSrc(std::shared_ptr<Operand> newSrc) {
    if (!newSrc) {
        throw std::logic_error("Setting null source operand in MovInstruction");
    }
    this->src = newSrc;
}

void MovInstruction::setDst(std::shared_ptr<Operand> newDst) {
    if (!newDst) {
        throw std::logic_error(
            "Setting null destination operand in MovInstruction");
    }
    this->dst = newDst;
}

MovsxInstruction::MovsxInstruction(std::shared_ptr<Operand> src,
                                   std::shared_ptr<Operand> dst)
    : src(src), dst(dst) {
    if (!src) {
        throw std::logic_error("Creating MovsxInstruction with null src");
    }
    if (!dst) {
        throw std::logic_error("Creating MovsxInstruction with null dst");
    }
}

std::shared_ptr<Operand> MovsxInstruction::getSrc() { return src; }

std::shared_ptr<Operand> MovsxInstruction::getDst() { return dst; }

void MovsxInstruction::setSrc(std::shared_ptr<Operand> newSrc) {
    if (!newSrc) {
        throw std::logic_error(
            "Setting null source operand in MovsxInstruction");
    }
    this->src = newSrc;
}

void MovsxInstruction::setDst(std::shared_ptr<Operand> newDst) {
    if (!newDst) {
        throw std::logic_error(
            "Setting null destination operand in MovsxInstruction");
    }
    this->dst = newDst;
}

UnaryInstruction::UnaryInstruction(std::shared_ptr<UnaryOperator> unaryOperator,
                                   std::shared_ptr<AssemblyType> type,
                                   std::shared_ptr<Operand> operand)
    : unaryOperator(unaryOperator), type(type), operand(operand) {
    if (!unaryOperator) {
        throw std::logic_error(
            "Creating UnaryInstruction with null unaryOperator");
    }
    if (!type) {
        throw std::logic_error("Creating UnaryInstruction with null type");
    }
    if (!operand) {
        throw std::logic_error("Creating UnaryInstruction with null operand");
    }
}

std::shared_ptr<UnaryOperator> UnaryInstruction::getUnaryOperator() {
    return unaryOperator;
}

std::shared_ptr<AssemblyType> UnaryInstruction::getType() { return type; }

std::shared_ptr<Operand> UnaryInstruction::getOperand() { return operand; }

void UnaryInstruction::setUnaryOperator(
    std::shared_ptr<UnaryOperator> newUnaryOperator) {
    if (!newUnaryOperator) {
        throw std::logic_error(
            "Setting null unary operator in UnaryInstruction");
    }
    this->unaryOperator = newUnaryOperator;
}

void UnaryInstruction::setType(std::shared_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in UnaryInstruction");
    }
    this->type = newType;
}

void UnaryInstruction::setOperand(std::shared_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in UnaryInstruction");
    }
    this->operand = newOperand;
}

BinaryInstruction::BinaryInstruction(
    std::shared_ptr<BinaryOperator> binaryOperator,
    std::shared_ptr<AssemblyType> type, std::shared_ptr<Operand> operand1,
    std::shared_ptr<Operand> operand2)
    : binaryOperator(binaryOperator), type(type), operand1(operand1),
      operand2(operand2) {
    if (!binaryOperator) {
        throw std::logic_error(
            "Creating BinaryInstruction with null binaryOperator");
    }
    if (!type) {
        throw std::logic_error("Creating BinaryInstruction with null type");
    }
    if (!operand1) {
        throw std::logic_error("Creating BinaryInstruction with null operand1");
    }
    if (!operand2) {
        throw std::logic_error("Creating BinaryInstruction with null operand2");
    }
}

std::shared_ptr<BinaryOperator> BinaryInstruction::getBinaryOperator() {
    return binaryOperator;
}

std::shared_ptr<AssemblyType> BinaryInstruction::getType() { return type; }

std::shared_ptr<Operand> BinaryInstruction::getOperand1() { return operand1; }

std::shared_ptr<Operand> BinaryInstruction::getOperand2() { return operand2; }

void BinaryInstruction::setBinaryOperator(
    std::shared_ptr<BinaryOperator> newBinaryOperator) {
    if (!newBinaryOperator) {
        throw std::logic_error(
            "Setting null binary operator in BinaryInstruction");
    }
    this->binaryOperator = newBinaryOperator;
}

void BinaryInstruction::setType(std::shared_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in BinaryInstruction");
    }
    this->type = newType;
}

void BinaryInstruction::setOperand1(std::shared_ptr<Operand> newOperand1) {
    if (!newOperand1) {
        throw std::logic_error("Setting null operand1 in BinaryInstruction");
    }
    this->operand1 = newOperand1;
}

void BinaryInstruction::setOperand2(std::shared_ptr<Operand> newOperand2) {
    if (!newOperand2) {
        throw std::logic_error("Setting null operand2 in BinaryInstruction");
    }
    this->operand2 = newOperand2;
}

CmpInstruction::CmpInstruction(std::shared_ptr<AssemblyType> type,
                               std::shared_ptr<Operand> operand1,
                               std::shared_ptr<Operand> operand2)
    : type(type), operand1(operand1), operand2(operand2) {
    if (!type) {
        throw std::logic_error("Creating CmpInstruction with null type");
    }
    if (!operand1) {
        throw std::logic_error("Creating CmpInstruction with null operand1");
    }
    if (!operand2) {
        throw std::logic_error("Creating CmpInstruction with null operand2");
    }
}

std::shared_ptr<AssemblyType> CmpInstruction::getType() { return type; }

std::shared_ptr<Operand> CmpInstruction::getOperand1() { return operand1; }

std::shared_ptr<Operand> CmpInstruction::getOperand2() { return operand2; }

void CmpInstruction::setType(std::shared_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in CmpInstruction");
    }
    this->type = newType;
}

void CmpInstruction::setOperand1(std::shared_ptr<Operand> newOperand1) {
    if (!newOperand1) {
        throw std::logic_error("Setting null operand1 in CmpInstruction");
    }
    this->operand1 = newOperand1;
}

void CmpInstruction::setOperand2(std::shared_ptr<Operand> newOperand2) {
    if (!newOperand2) {
        throw std::logic_error("Setting null operand2 in CmpInstruction");
    }
    this->operand2 = newOperand2;
}

IdivInstruction::IdivInstruction(std::shared_ptr<AssemblyType> type,
                                 std::shared_ptr<Operand> operand)
    : type(type), operand(operand) {
    if (!type) {
        throw std::logic_error("Creating IdivInstruction with null type");
    }
    if (!operand) {
        throw std::logic_error("Creating IdivInstruction with null operand");
    }
}

std::shared_ptr<AssemblyType> IdivInstruction::getType() { return type; }

std::shared_ptr<Operand> IdivInstruction::getOperand() { return operand; }

void IdivInstruction::setType(std::shared_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in IdivInstruction");
    }
    this->type = newType;
}

void IdivInstruction::setOperand(std::shared_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in IdivInstruction");
    }
    this->operand = newOperand;
}

CdqInstruction::CdqInstruction(std::shared_ptr<AssemblyType> type)
    : type(type) {}

std::shared_ptr<AssemblyType> CdqInstruction::getType() { return type; }

void CdqInstruction::setType(std::shared_ptr<AssemblyType> newType) {
    if (!newType) {
        throw std::logic_error("Setting null type in CdqInstruction");
    }
    this->type = newType;
}

JmpInstruction::JmpInstruction(std::string label) : label(label) {}

std::string JmpInstruction::getLabel() { return label; }

void JmpInstruction::setLabel(std::string newLabel) { this->label = newLabel; }

JmpCCInstruction::JmpCCInstruction(std::shared_ptr<CondCode> condCode,
                                   std::string label)
    : condCode(condCode), label(label) {
    if (!condCode) {
        throw std::logic_error("Creating JmpCCInstruction with null condCode");
    }
}

std::shared_ptr<CondCode> JmpCCInstruction::getCondCode() { return condCode; }

std::string JmpCCInstruction::getLabel() { return label; }

void JmpCCInstruction::setCondCode(std::shared_ptr<CondCode> newCondCode) {
    if (!newCondCode) {
        throw std::logic_error(
            "Setting null condition code in JmpCCInstruction");
    }
    this->condCode = newCondCode;
}

void JmpCCInstruction::setLabel(std::string newLabel) {
    this->label = newLabel;
}

SetCCInstruction::SetCCInstruction(std::shared_ptr<CondCode> condCode,
                                   std::shared_ptr<Operand> operand)
    : condCode(condCode), operand(operand) {
    if (!condCode) {
        throw std::logic_error("Creating SetCCInstruction with null condCode");
    }
    if (!operand) {
        throw std::logic_error("Creating SetCCInstruction with null operand");
    }
}

std::shared_ptr<CondCode> SetCCInstruction::getCondCode() { return condCode; }

std::shared_ptr<Operand> SetCCInstruction::getOperand() { return operand; }

void SetCCInstruction::setCondCode(std::shared_ptr<CondCode> newCondCode) {
    if (!newCondCode) {
        throw std::logic_error(
            "Setting null condition code in SetCCInstruction");
    }
    this->condCode = newCondCode;
}

void SetCCInstruction::setOperand(std::shared_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in SetCCInstruction");
    }
    this->operand = newOperand;
}

LabelInstruction::LabelInstruction(std::string label) : label(label) {}

std::string LabelInstruction::getLabel() { return label; }

void LabelInstruction::setLabel(std::string newLabel) {
    this->label = newLabel;
}

PushInstruction::PushInstruction(std::shared_ptr<Operand> operand)
    : operand(operand) {
    if (!operand) {
        throw std::logic_error("Creating PushInstruction with null operand");
    }
}

std::shared_ptr<Operand> PushInstruction::getOperand() { return operand; }

void PushInstruction::setOperand(std::shared_ptr<Operand> newOperand) {
    if (!newOperand) {
        throw std::logic_error("Setting null operand in PushInstruction");
    }
    this->operand = newOperand;
}

CallInstruction::CallInstruction(std::string functionIdentifier)
    : functionIdentifier(functionIdentifier) {}

std::string CallInstruction::getFunctionIdentifier() {
    return functionIdentifier;
}

FunctionDefinition::FunctionDefinition(
    std::string functionIdentifier, bool global,
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody,
    size_t stackSize)
    : functionIdentifier(functionIdentifier), global(global),
      functionBody(functionBody), stackSize(stackSize) {
    if (!functionBody) {
        throw std::logic_error(
            "Creating FunctionDefinition with null functionBody");
    }
}

std::string FunctionDefinition::getFunctionIdentifier() {
    return functionIdentifier;
}

bool FunctionDefinition::isGlobal() { return global; }

std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
FunctionDefinition::getFunctionBody() {
    return functionBody;
}

void FunctionDefinition::setFunctionBody(
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
        newFunctionBody) {
    if (!newFunctionBody) {
        throw std::logic_error(
            "Setting null function body in FunctionDefinition");
    }
    this->functionBody = newFunctionBody;
}

size_t FunctionDefinition::getStackSize() { return stackSize; }

void FunctionDefinition::setStackSize(size_t newStackSize) {
    this->stackSize = newStackSize;
}

StaticVariable::StaticVariable(std::string identifier, bool global,
                               int alignment,
                               std::shared_ptr<AST::StaticInit> staticInit)
    : identifier(identifier), global(global), alignment(alignment),
      staticInit(staticInit) {
    if (!staticInit) {
        throw std::logic_error("Creating StaticVariable with null staticInit");
    }
}

std::string StaticVariable::getIdentifier() { return identifier; }

bool StaticVariable::isGlobal() { return global; }

int StaticVariable::getAlignment() { return alignment; }

void StaticVariable::setAlignment(int newAlignment) {
    this->alignment = newAlignment;
}

std::shared_ptr<AST::StaticInit> StaticVariable::getStaticInit() {
    return staticInit;
}

void StaticVariable::setStaticInit(
    std::shared_ptr<AST::StaticInit> newStaticInit) {
    if (!newStaticInit) {
        throw std::logic_error("Setting null static init in StaticVariable");
    }
    this->staticInit = newStaticInit;
}

Program::Program(
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels)
    : topLevels(topLevels) {}

std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>>
Program::getTopLevels() {
    return topLevels;
}

void Program::setTopLevels(
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> newTopLevels) {
    if (!newTopLevels) {
        throw std::logic_error("Setting null top levels in Program");
    }
    this->topLevels = newTopLevels;
}

} // namespace Assembly
