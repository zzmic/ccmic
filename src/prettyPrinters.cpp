#include "prettyPrinters.h"

/*
 * Start: Functions to print the IR program onto the stdout.
 */
void PrettyPrinters::printIRProgram(std::shared_ptr<IR::Program> irProgram) {
    for (auto function : *irProgram->getFunctionDefinition()) {
        printIRFunctionDefinition(function);
    }
}

void PrettyPrinters::printIRFunctionDefinition(
    std::shared_ptr<IR::FunctionDefinition> functionDefinition) {
    std::cout << functionDefinition->getFunctionIdentifier() << ":\n";
    for (auto instruction : *functionDefinition->getFunctionBody()) {
        printIRInstruction(instruction);
    }
}

void PrettyPrinters::printIRInstruction(
    std::shared_ptr<IR::Instruction> instruction) {
    if (auto returnInstruction =
            std::dynamic_pointer_cast<IR::ReturnInstruction>(instruction)) {
        printIRReturnInstruction(returnInstruction);
    }
    else if (auto unaryInstruction =
                 std::dynamic_pointer_cast<IR::UnaryInstruction>(instruction)) {
        printIRUnaryInstruction(unaryInstruction);
    }
    else if (auto binaryInstruction =
                 std::dynamic_pointer_cast<IR::BinaryInstruction>(
                     instruction)) {
        printIRBinaryInstruction(binaryInstruction);
    }
    else if (auto copyInstruction =
                 std::dynamic_pointer_cast<IR::CopyInstruction>(instruction)) {
        printIRCopyInstruction(copyInstruction);
    }
    else if (auto jumpInstruction =
                 std::dynamic_pointer_cast<IR::JumpInstruction>(instruction)) {
        printIRJumpInstruction(jumpInstruction);
    }
    else if (auto jumpIfZeroInstruction =
                 std::dynamic_pointer_cast<IR::JumpIfZeroInstruction>(
                     instruction)) {
        printIRJumpIfZeroInstruction(jumpIfZeroInstruction);
    }
    else if (auto jumpIfNotZeroInstruction =
                 std::dynamic_pointer_cast<IR::JumpIfNotZeroInstruction>(
                     instruction)) {
        printIRJumpIfNotZeroInstruction(jumpIfNotZeroInstruction);
    }
    else if (auto labelInstruction =
                 std::dynamic_pointer_cast<IR::LabelInstruction>(instruction)) {
        printIRLabelInstruction(labelInstruction);
    }
    else if (auto functionCallInstruction =
                 std::dynamic_pointer_cast<IR::FunctionCallInstruction>(
                     instruction)) {
        printIRFunctionCallInstruction(functionCallInstruction);
    }
}

void PrettyPrinters::printIRReturnInstruction(
    std::shared_ptr<IR::ReturnInstruction> returnInstruction) {
    std::cout << "    return ";

    if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
            returnInstruction->getReturnValue())) {
        std::cout << constantValue->getValue();
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 returnInstruction->getReturnValue())) {
        std::cout << variableValue->getIdentifier();
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRUnaryInstruction(
    std::shared_ptr<IR::UnaryInstruction> unaryInstruction) {
    std::cout << "    ";

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            unaryInstruction->getDst())) {
        std::cout << variableValue->getIdentifier();
    }

    if (auto complementOperator =
            std::dynamic_pointer_cast<IR::ComplementOperator>(
                unaryInstruction->getUnaryOperator())) {
        std::cout << " = ~";
    }
    else if (auto negateOperator =
                 std::dynamic_pointer_cast<IR::NegateOperator>(
                     unaryInstruction->getUnaryOperator())) {
        std::cout << " = -";
    }
    else if (auto notOperator = std::dynamic_pointer_cast<IR::NotOperator>(
                 unaryInstruction->getUnaryOperator())) {
        std::cout << " = !";
    }

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            unaryInstruction->getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 unaryInstruction->getSrc())) {
        std::cout << constantValue->getValue();
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRBinaryInstruction(
    std::shared_ptr<IR::BinaryInstruction> binaryInstruction) {
    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            binaryInstruction->getDst())) {
        std::cout << "    " << variableValue->getIdentifier();
    }

    std::cout << " = ";

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            binaryInstruction->getSrc1())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 binaryInstruction->getSrc1())) {
        std::cout << constantValue->getValue();
    }

    if (auto binaryOperator = std::dynamic_pointer_cast<IR::AddOperator>(
            binaryInstruction->getBinaryOperator())) {
        std::cout << " + ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::SubtractOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " - ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::MultiplyOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " * ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::DivideOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " / ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::RemainderOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " % ";
    }
    else if (auto binaryOperator = std::dynamic_pointer_cast<IR::EqualOperator>(
                 binaryInstruction->getBinaryOperator())) {
        std::cout << " == ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::NotEqualOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " != ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::LessThanOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " < ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::LessThanOrEqualOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " <= ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::GreaterThanOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " > ";
    }
    else if (auto binaryOperator =
                 std::dynamic_pointer_cast<IR::GreaterThanOrEqualOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " >= ";
    }

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            binaryInstruction->getSrc2())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 binaryInstruction->getSrc2())) {
        std::cout << constantValue->getValue();
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRCopyInstruction(
    std::shared_ptr<IR::CopyInstruction> copyInstruction) {
    std::cout << "    ";
    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            copyInstruction->getDst())) {
        std::cout << variableValue->getIdentifier();
    }

    std::cout << " = ";

    if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
            copyInstruction->getSrc())) {
        std::cout << constantValue->getValue();
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 copyInstruction->getSrc())) {
        std::cout << variableValue->getIdentifier();
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRJumpInstruction(
    std::shared_ptr<IR::JumpInstruction> jumpInstruction) {
    std::cout << "    Jump(" << jumpInstruction->getTarget() << ")\n";
}

void PrettyPrinters::printIRJumpIfZeroInstruction(
    std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstruction) {
    std::cout << "    JumpIfZero(";

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            jumpIfZeroInstruction->getCondition())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 jumpIfZeroInstruction->getCondition())) {
        std::cout << constantValue->getValue();
    }

    std::cout << ", " << jumpIfZeroInstruction->getTarget() << ")\n";
}

void PrettyPrinters::printIRJumpIfNotZeroInstruction(
    std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstruction) {
    std::cout << "    JumpIfNotZero(";

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            jumpIfNotZeroInstruction->getCondition())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 jumpIfNotZeroInstruction->getCondition())) {
        std::cout << constantValue->getValue();
    }

    std::cout << ", " << jumpIfNotZeroInstruction->getTarget() << ")\n";
}

void PrettyPrinters::printIRLabelInstruction(
    std::shared_ptr<IR::LabelInstruction> labelInstruction) {
    std::cout << "    Label(" << labelInstruction->getLabel() << ")\n";
}

void PrettyPrinters::printIRFunctionCallInstruction(
    std::shared_ptr<IR::FunctionCallInstruction> functionCallInstruction) {
    auto dst = functionCallInstruction->getDst();
    if (auto variableValue =
            std::dynamic_pointer_cast<IR::VariableValue>(dst)) {
        std::cout << "    " << variableValue->getIdentifier() << " = ";
    }

    auto functionIdentifier = functionCallInstruction->getFunctionIdentifier();
    std::cout << functionIdentifier << "(";
    for (auto arg : *functionCallInstruction->getArgs()) {
        if (auto variableValue =
                std::dynamic_pointer_cast<IR::VariableValue>(arg)) {
            std::cout << variableValue->getIdentifier();
        }
        else if (auto constantValue =
                     std::dynamic_pointer_cast<IR::ConstantValue>(arg)) {
            std::cout << constantValue->getValue();
        }
        std::cout << ", ";
    }
    std::cout << ")\n";
}
/*
 * End: Functions to print the IR program onto the stdout.
 */

/*
 * Start: Functions to print the assembly program onto the stdout.
 */
// Function to print the assembly code onto the stdout.
void PrettyPrinters::printAssemblyProgram(
    std::shared_ptr<Assembly::Program> assemblyProgram) {
    for (auto function : *assemblyProgram->getFunctionDefinition()) {
        printAssyFunctionDefinition(function);
    }
}

void PrettyPrinters::printAssyFunctionDefinition(
    std::shared_ptr<Assembly::FunctionDefinition> functionDefinition) {
    std::string functionName = functionDefinition->getFunctionIdentifier();

    // Print the function prologue (before printing the function body).
    std::cout << "\n"
              << "    .globl " << functionName << "\n";
    std::cout << functionDefinition->getFunctionIdentifier() << ":\n";
    std::cout << "    pushq %rbp\n";
    std::cout << "    movq %rsp, %rbp\n";

    for (auto instruction : *functionDefinition->getFunctionBody()) {
        printAssyInstruction(instruction);
    }
}

void PrettyPrinters::printAssyInstruction(
    std::shared_ptr<Assembly::Instruction> instruction) {
    if (auto movInstruction =
            std::dynamic_pointer_cast<Assembly::MovInstruction>(instruction)) {
        printAssyMovInstruction(movInstruction);
    }
    else if (auto retInstruction =
                 std::dynamic_pointer_cast<Assembly::RetInstruction>(
                     instruction)) {
        printAssyRetInstruction();
    }
    else if (auto allocateStackInstruction =
                 std::dynamic_pointer_cast<Assembly::AllocateStackInstruction>(
                     instruction)) {
        printAssyAllocateStackInstruction(allocateStackInstruction);
    }
    else if (auto unaryInstruction =
                 std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                     instruction)) {
        printAssyUnaryInstruction(unaryInstruction);
    }
    else if (auto binaryInstruction =
                 std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                     instruction)) {
        printAssyBinaryInstruction(binaryInstruction);
    }
    else if (auto cmpInstruction =
                 std::dynamic_pointer_cast<Assembly::CmpInstruction>(
                     instruction)) {
        printAssyCmpInstruction(cmpInstruction);
    }
    else if (auto idivInstruction =
                 std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                     instruction)) {
        printAssyIdivInstruction(idivInstruction);
    }
    else if (auto cdqInstruction =
                 std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                     instruction)) {
        printAssyCdqInstruction();
    }
    else if (auto jmpInstruction =
                 std::dynamic_pointer_cast<Assembly::JmpInstruction>(
                     instruction)) {
        printAssyJmpInstruction(jmpInstruction);
    }
    else if (auto jmpCCInstruction =
                 std::dynamic_pointer_cast<Assembly::JmpCCInstruction>(
                     instruction)) {
        printAssyJmpCCInstruction(jmpCCInstruction);
    }
    else if (auto setCCInstruction =
                 std::dynamic_pointer_cast<Assembly::SetCCInstruction>(
                     instruction)) {
        printAssySetCCInstruction(setCCInstruction);
    }
    else if (auto labelInstruction =
                 std::dynamic_pointer_cast<Assembly::LabelInstruction>(
                     instruction)) {
        printAssyLabelInstruction(labelInstruction);
    }
}

void PrettyPrinters::printAssyMovInstruction(
    std::shared_ptr<Assembly::MovInstruction> movInstruction) {
    auto src = movInstruction->getSrc();
    if (auto srcReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
        std::cout << "    movl %" << srcReg->getRegister();
    }
    else if (auto srcImm =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src)) {
        std::cout << "    movl $" << srcImm->getImmediate();
    }
    else if (auto srcStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(src)) {
        std::cout << "    movl " << srcStack->getOffset() << "(%rsp)";
    }

    auto dst = movInstruction->getDst();
    if (auto dstReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
        std::cout << ", %" << dstReg->getRegister() << "\n";
    }
    else if (auto dstStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(dst)) {
        std::cout << ", " << dstStack->getOffset() << "(%rsp)\n";
    }
}

void PrettyPrinters::printAssyRetInstruction() {
    // Print the function epilogue before printing the return
    // instruction.
    std::cout << "    movq %rbp, %rsp\n";
    std::cout << "    popq %rbp\n";
    std::cout << "    ret\n";
}

void PrettyPrinters::printAssyAllocateStackInstruction(
    std::shared_ptr<Assembly::AllocateStackInstruction>
        allocateStackInstruction) {
    std::cout << "    subq $"
              << allocateStackInstruction->getAddressGivenOffsetFromRBP()
              << ", %rsp\n";
}

void PrettyPrinters::printAssyUnaryInstruction(
    std::shared_ptr<Assembly::UnaryInstruction> unaryInstruction) {
    auto unaryOperator = unaryInstruction->getUnaryOperator();
    if (auto negateOperator =
            std::dynamic_pointer_cast<Assembly::NegateOperator>(
                unaryOperator)) {
        std::cout << "    negl";
    }
    else if (auto complementOperator =
                 std::dynamic_pointer_cast<Assembly::ComplementOperator>(
                     unaryOperator)) {
        std::cout << "    notl";
    }
    else if (auto notOperator =
                 std::dynamic_pointer_cast<Assembly::NotOperator>(
                     unaryOperator)) {
        std::cout << "    notl";
    }

    auto operand = unaryInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        std::cout << " %" << regOperand->getRegister() << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "(%rsp)\n";
    }
}

void PrettyPrinters::printAssyBinaryInstruction(
    std::shared_ptr<Assembly::BinaryInstruction> binaryInstruction) {
    auto binaryOperator = binaryInstruction->getBinaryOperator();
    if (auto addOperator =
            std::dynamic_pointer_cast<Assembly::AddOperator>(binaryOperator)) {
        std::cout << "    addl";
    }
    else if (auto subtractOperator =
                 std::dynamic_pointer_cast<Assembly::SubtractOperator>(
                     binaryOperator)) {
        std::cout << "    subl";
    }
    else if (auto multiplyOperator =
                 std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                     binaryOperator)) {
        std::cout << "    imull";
    }

    auto operand1 = binaryInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        std::cout << " $" << operand1Imm->getImmediate() << ",";
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        std::cout << " %" << operand1Reg->getRegister() << ",";
    }
    else if (auto operand1Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand1)) {
        std::cout << " " << operand1Stack->getOffset() << "(%rsp),";
    }

    auto operand2 = binaryInstruction->getOperand2();
    if (auto operand2Reg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand2)) {
        std::cout << " %" << operand2Reg->getRegister() << "\n";
    }
    else if (auto operand2Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand2)) {
        std::cout << " " << operand2Stack->getOffset() << "(%rsp)\n";
    }
}

void PrettyPrinters::printAssyCmpInstruction(
    std::shared_ptr<Assembly::CmpInstruction> cmpInstruction) {
    std::cout << "    cmpl";

    auto operand1 = cmpInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        std::cout << " $" << operand1Imm->getImmediate();
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        std::cout << " %" << operand1Reg->getRegister();
    }
    else if (auto operand1Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand1)) {
        std::cout << " " << operand1Stack->getOffset() << "(%rsp)";
    }

    std::cout << ",";

    auto operand2 = cmpInstruction->getOperand2();
    if (auto operand2Reg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand2)) {
        std::cout << " %" << operand2Reg->getRegister() << "\n";
    }
    else if (auto operand2Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand2)) {
        std::cout << " " << operand2Stack->getOffset() << "(%rsp)\n";
    }
}

void PrettyPrinters::printAssyIdivInstruction(
    std::shared_ptr<Assembly::IdivInstruction> idivInstruction) {
    std::cout << "    idivl";

    auto operand = idivInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        std::cout << " %" << regOperand->getRegister() << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "(%rsp)\n";
    }
}

void PrettyPrinters::printAssyCdqInstruction() { std::cout << "    cdq\n"; }

void PrettyPrinters::printAssyJmpInstruction(
    std::shared_ptr<Assembly::JmpInstruction> jmpInstruction) {
    auto label = jmpInstruction->getLabel();
    std::cout << "    jmp .L" << label << "\n";
}

void PrettyPrinters::printAssyJmpCCInstruction(
    std::shared_ptr<Assembly::JmpCCInstruction> jmpCCInstruction) {
    auto condCode = jmpCCInstruction->getCondCode();
    if (auto e = std::dynamic_pointer_cast<Assembly::E>(condCode)) {
        std::cout << "    je";
    }
    else if (auto ne = std::dynamic_pointer_cast<Assembly::NE>(condCode)) {
        std::cout << "    jne";
    }
    else if (auto g = std::dynamic_pointer_cast<Assembly::G>(condCode)) {
        std::cout << "    jg";
    }
    else if (auto ge = std::dynamic_pointer_cast<Assembly::GE>(condCode)) {
        std::cout << "    jge";
    }
    else if (auto l = std::dynamic_pointer_cast<Assembly::L>(condCode)) {
        std::cout << "    jl";
    }
    else if (auto le = std::dynamic_pointer_cast<Assembly::LE>(condCode)) {
        std::cout << "    jle";
    }

    auto label = jmpCCInstruction->getLabel();
    std::cout << " .L" << label << "\n";
}

void PrettyPrinters::printAssySetCCInstruction(
    std::shared_ptr<Assembly::SetCCInstruction> setCCInstruction) {
    auto condCode = setCCInstruction->getCondCode();
    if (auto e = std::dynamic_pointer_cast<Assembly::E>(condCode)) {
        std::cout << "    sete";
    }
    else if (auto ne = std::dynamic_pointer_cast<Assembly::NE>(condCode)) {
        std::cout << "    setne";
    }
    else if (auto g = std::dynamic_pointer_cast<Assembly::G>(condCode)) {
        std::cout << "    setg";
    }
    else if (auto ge = std::dynamic_pointer_cast<Assembly::GE>(condCode)) {
        std::cout << "    setge";
    }
    else if (auto l = std::dynamic_pointer_cast<Assembly::L>(condCode)) {
        std::cout << "    setl";
    }
    else if (auto le = std::dynamic_pointer_cast<Assembly::LE>(condCode)) {
        std::cout << "    setle";
    }

    auto operand = setCCInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        if (regOperand->getRegister() == "%eax") {
            std::cout << " %al\n";
        }
        else if (regOperand->getRegister() == "%edx") {
            std::cout << " %dl\n";
        }
        else if (regOperand->getRegister() == "%r10d") {
            std::cout << " %r10b\n";
        }
        else if (regOperand->getRegister() == "%r11d") {
            std::cout << " %r11b\n";
        }
        else {
            throw std::runtime_error(
                "Unsupported register conversion for SetCC instruction");
        }
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "(%rsp)\n";
    }
}

void PrettyPrinters::printAssyLabelInstruction(
    std::shared_ptr<Assembly::LabelInstruction> labelInstruction) {
    auto label = labelInstruction->getLabel();
    std::cout << ".L" << label << ":\n";
}
/*
 * End: Functions to print the assembly program onto the stdout.
 */
