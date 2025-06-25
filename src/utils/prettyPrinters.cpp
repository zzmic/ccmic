#include "prettyPrinters.h"

/*
 * Start: Functions to print the IR program onto the stdout.
 */
void PrettyPrinters::printIRProgram(
    const std::shared_ptr<IR::Program> &irProgram,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
        &irStaticVariables) {
    auto topLevels = irProgram->getTopLevels();

    for (auto topLevel : *topLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<IR::FunctionDefinition>(topLevel)) {
            printIRFunctionDefinition(functionDefinition);
        }
        else {
            throw std::runtime_error("Unsupported top-level element");
        }
    }

    for (auto irStaticVariable : *irStaticVariables) {
        printIRStaticVariable(irStaticVariable);
    }
}

void PrettyPrinters::printIRFunctionDefinition(
    const std::shared_ptr<IR::FunctionDefinition> &functionDefinition) {
    std::cout << functionDefinition->getFunctionIdentifier();
    std::cout << std::boolalpha;
    std::cout << "[isGlobal: " << functionDefinition->isGlobal() << "]";
    std::cout << "(";

    auto &parameters = *functionDefinition->getParameterIdentifiers();
    for (auto it = parameters.begin(); it != parameters.end(); it++) {
        auto &parameter = *it;
        std::cout << parameter;
        bool isLast = (std::next(it) == parameters.end());
        if (!isLast) {
            std::cout << ", ";
        }
    }

    std::cout << "):\n";
    for (auto instruction : *functionDefinition->getFunctionBody()) {
        printIRInstruction(instruction);
    }
}

void PrettyPrinters::printIRStaticVariable(
    const std::shared_ptr<IR::StaticVariable> &staticVariable) {
    auto staticInit = staticVariable->getStaticInit();
    std::cout << "[static] " << staticVariable->getIdentifier() << " = ";

    if (auto intInit = std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
        std::cout << std::get<int>(intInit->getValue());
    }
    else if (auto longInit =
                 std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
        std::cout << std::get<long>(longInit->getValue());
    }
    else {
        throw std::runtime_error("Unknown static variable initializer type");
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRInstruction(
    const std::shared_ptr<IR::Instruction> &instruction) {
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
    else if (auto signExtendInstruction =
                 std::dynamic_pointer_cast<IR::SignExtendInstruction>(
                     instruction)) {
        printIRSignExtendInstruction(signExtendInstruction);
    }
    else if (auto truncateInstruction =
                 std::dynamic_pointer_cast<IR::TruncateInstruction>(
                     instruction)) {
        printIRTruncateInstruction(truncateInstruction);
    }
    else {
        throw std::runtime_error("Unknown instruction type in IR program");
    }
}

void PrettyPrinters::printIRReturnInstruction(
    const std::shared_ptr<IR::ReturnInstruction> &returnInstruction) {
    std::cout << "    return ";

    if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
            returnInstruction->getReturnValue())) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in return instruction");
        }
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 returnInstruction->getReturnValue())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown return value type in return instruction");
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRSignExtendInstruction(
    const std::shared_ptr<IR::SignExtendInstruction> &signExtendInstruction) {
    std::cout << "    ";
    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            signExtendInstruction->getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown destination value type in sign extend instruction");
    }

    std::cout << " = sign_extend(";

    if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
            signExtendInstruction->getSrc())) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in sign extend instruction");
        }
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 signExtendInstruction->getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown source value type in sign extend instruction");
    }

    std::cout << ")\n";
}

void PrettyPrinters::printIRTruncateInstruction(
    const std::shared_ptr<IR::TruncateInstruction> &truncateInstruction) {
    std::cout << "    ";
    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            truncateInstruction->getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown destination value type in truncate instruction");
    }

    std::cout << " = truncate(";

    if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
            truncateInstruction->getSrc())) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in truncate instruction");
        }
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 truncateInstruction->getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown source value type in truncate instruction");
    }

    std::cout << ")\n";
}

void PrettyPrinters::printIRUnaryInstruction(
    const std::shared_ptr<IR::UnaryInstruction> &unaryInstruction) {
    std::cout << "    ";

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            unaryInstruction->getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown destination value type in unary instruction");
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
    else {
        throw std::runtime_error("Unknown unary operator in instruction");
    }

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            unaryInstruction->getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 unaryInstruction->getSrc())) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
            std::cout << "\n";
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
            std::cout << "\n";
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in unary instruction");
        }
    }
    else {
        throw std::runtime_error(
            "Unknown source value type in unary instruction");
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRBinaryInstruction(
    const std::shared_ptr<IR::BinaryInstruction> &binaryInstruction) {
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
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in binary instruction");
        }
    }
    else {
        throw std::runtime_error(
            "Unknown source value type in binary instruction");
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
    else {
        throw std::runtime_error("Unknown binary operator in instruction");
    }

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            binaryInstruction->getSrc2())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 binaryInstruction->getSrc2())) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in binary instruction");
        }
    }
    else {
        throw std::runtime_error(
            "Unknown source value type in binary instruction");
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRCopyInstruction(
    const std::shared_ptr<IR::CopyInstruction> &copyInstruction) {
    std::cout << "    ";
    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            copyInstruction->getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown/unsupported destination value type in copy instruction");
    }

    std::cout << " = ";

    auto src = copyInstruction->getSrc();
    if (src == nullptr) {
        throw std::runtime_error("Source value is null in copy instruction");
    }

    if (auto constantValue =
            std::dynamic_pointer_cast<IR::ConstantValue>(src)) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in copy instruction");
        }
    }
    else if (auto variableValue =
                 std::dynamic_pointer_cast<IR::VariableValue>(src)) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::runtime_error(
            "Unknown source value type in copy instruction");
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRJumpInstruction(
    const std::shared_ptr<IR::JumpInstruction> &jumpInstruction) {
    std::cout << "    Jump(" << jumpInstruction->getTarget() << ")\n";
}

void PrettyPrinters::printIRJumpIfZeroInstruction(
    const std::shared_ptr<IR::JumpIfZeroInstruction> &jumpIfZeroInstruction) {
    std::cout << "    JumpIfZero(";

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            jumpIfZeroInstruction->getCondition())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 jumpIfZeroInstruction->getCondition())) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in jump if zero instruction");
        }
    }

    std::cout << ", " << jumpIfZeroInstruction->getTarget() << ")\n";
}

void PrettyPrinters::printIRJumpIfNotZeroInstruction(
    const std::shared_ptr<IR::JumpIfNotZeroInstruction>
        &jumpIfNotZeroInstruction) {
    std::cout << "    JumpIfNotZero(";

    if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
            jumpIfNotZeroInstruction->getCondition())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (auto constantValue = std::dynamic_pointer_cast<IR::ConstantValue>(
                 jumpIfNotZeroInstruction->getCondition())) {
        if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (auto constantLong =
                     std::dynamic_pointer_cast<AST::ConstantLong>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else {
            throw std::runtime_error(
                "Unknown constant type in jump if not zero instruction");
        }
    }
    else {
        throw std::runtime_error(
            "Unknown condition value type in jump if not zero instruction");
    }

    std::cout << ", " << jumpIfNotZeroInstruction->getTarget() << ")\n";
}

void PrettyPrinters::printIRLabelInstruction(
    const std::shared_ptr<IR::LabelInstruction> &labelInstruction) {
    std::cout << "    Label(" << labelInstruction->getLabel() << ")\n";
}

void PrettyPrinters::printIRFunctionCallInstruction(
    const std::shared_ptr<IR::FunctionCallInstruction>
        &functionCallInstruction) {
    auto dst = functionCallInstruction->getDst();

    if (auto variableValue =
            std::dynamic_pointer_cast<IR::VariableValue>(dst)) {
        std::cout << "    " << variableValue->getIdentifier() << " = ";
    }
    else {
        throw std::runtime_error(
            "Unknown destination value type in function call instruction");
    }

    auto functionIdentifier = functionCallInstruction->getFunctionIdentifier();
    std::cout << functionIdentifier << "(";

    auto &args = *functionCallInstruction->getArgs();
    for (auto it = args.begin(); it != args.end(); it++) {
        auto &arg = *it;
        if (auto variableValue =
                std::dynamic_pointer_cast<IR::VariableValue>(arg)) {
            std::cout << variableValue->getIdentifier();
        }
        else if (auto constantValue =
                     std::dynamic_pointer_cast<IR::ConstantValue>(arg)) {
            if (auto constantInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                    constantValue->getASTConstant())) {
                std::cout << constantInt->getValue();
            }
            else if (auto constantLong =
                         std::dynamic_pointer_cast<AST::ConstantLong>(
                             constantValue->getASTConstant())) {
                std::cout << constantLong->getValue();
            }
            else {
                throw std::runtime_error(
                    "Unknown constant type in function call argument");
            }
        }
        bool isLast = (std::next(it) == args.end());
        if (!isLast) {
            std::cout << ", ";
        }
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
    const std::shared_ptr<Assembly::Program> &assemblyProgram) {
    auto topLevels = assemblyProgram->getTopLevels();

    for (auto topLevel : *topLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<Assembly::FunctionDefinition>(
                    topLevel)) {
            printAssyFunctionDefinition(functionDefinition);
        }
        else if (auto staticVariable =
                     std::dynamic_pointer_cast<Assembly::StaticVariable>(
                         topLevel)) {
            printAssyStaticVariable(staticVariable);
        }
    }

// If the underlying OS is Linux, add the following to enable an important
// security hardening measure: it indicates that the code does not require an
// executable stack.
#ifdef __linux__
    std::cout << ".section .note.GNU-stack,\"\",@progbits\n";
#endif
}

void PrettyPrinters::printAssyFunctionDefinition(
    const std::shared_ptr<Assembly::FunctionDefinition> &functionDefinition) {
    std::string functionName = functionDefinition->getFunctionIdentifier();
    prependUnderscoreToIdentifierIfMacOS(functionName);
    auto global = functionDefinition->isGlobal();
    auto globalDirective = "    .globl " + functionName + "\n";
    if (!global) {
        globalDirective = "";
    }

    // Print the function prologue (before printing the function body).
    std::cout << "\n" << globalDirective;
    std::cout << "    .text\n";
    std::cout << functionName << ":\n";
    std::cout << "    pushq %rbp\n";
    std::cout << "    movq %rsp, %rbp\n";

    for (auto instruction : *functionDefinition->getFunctionBody()) {
        printAssyInstruction(instruction);
    }
}

void PrettyPrinters::printAssyStaticVariable(
    const std::shared_ptr<Assembly::StaticVariable> &staticVariable) {
    auto alignDirective = ".align 4";
// If the underlying OS is macOS, use the `.balign 4` directive instead of the
// `.align 4` directive.
#ifdef __APPLE__
    alignDirective = ".balign 4";
#endif
    auto variableIdentifier = staticVariable->getIdentifier();
    prependUnderscoreToIdentifierIfMacOS(variableIdentifier);
    auto global = staticVariable->isGlobal();
    auto globalDirective = ".globl " + variableIdentifier + "\n";
    if (!global) {
        globalDirective = "";
    }
    auto initialValue = staticVariable->getInitialValue();

    std::cout << "\n";
    if (initialValue != 0) {
        std::cout << globalDirective;
        std::cout << "    .data\n";
        std::cout << "    " << alignDirective << "\n";
        std::cout << variableIdentifier << ":\n";
        std::cout << "    .long " << initialValue << "\n";
    }
    else if (initialValue == 0) {
        std::cout << globalDirective;
        std::cout << "    .bss\n";
        std::cout << "    " << alignDirective << "\n";
        std::cout << variableIdentifier << ":\n";
        std::cout << "    .zero 4\n";
    }
}

void PrettyPrinters::printAssyInstruction(
    const std::shared_ptr<Assembly::Instruction> &instruction) {
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
    else if (auto deallocateStackInstruction = std::dynamic_pointer_cast<
                 Assembly::DeallocateStackInstruction>(instruction)) {
        printAssyDeallocateStackInstruction(deallocateStackInstruction);
    }
    else if (auto pushInstruction =
                 std::dynamic_pointer_cast<Assembly::PushInstruction>(
                     instruction)) {
        printAssyPushInstruction(pushInstruction);
    }
    else if (auto callInstruction =
                 std::dynamic_pointer_cast<Assembly::CallInstruction>(
                     instruction)) {
        printAssyCallInstruction(callInstruction);
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
    const std::shared_ptr<Assembly::MovInstruction> &movInstruction) {
    auto src = movInstruction->getSrc();
    if (auto srcReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
        std::cout << "    movl " << srcReg->getRegisterInBytesInStr(4);
    }
    else if (auto srcImm =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src)) {
        std::cout << "    movl $" << srcImm->getImmediate();
    }
    else if (auto srcStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(src)) {
        std::cout << "    movl " << srcStack->getOffset() << "("
                  << srcStack->getReservedRegisterInStr() << ")";
    }
    else if (auto srcData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << "    movl " << identifier << "(%rip)";
    }

    auto dst = movInstruction->getDst();
    if (auto dstReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
        std::cout << ", " << dstReg->getRegisterInBytesInStr(4) << "\n";
    }
    else if (auto dstStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(dst)) {
        std::cout << ", " << dstStack->getOffset() << "("
                  << dstStack->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dstData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << ", " << identifier << "(%rip)\n";
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
    const std::shared_ptr<Assembly::AllocateStackInstruction>
        &allocateStackInstruction) {
    std::cout << "    subq $"
              << allocateStackInstruction->getAddressGivenOffsetFromRBP()
              << ", %rsp\n";
}

void PrettyPrinters::printAssyDeallocateStackInstruction(
    const std::shared_ptr<Assembly::DeallocateStackInstruction>
        &deallocateStackInstruction) {
    std::cout << "    addq $"
              << deallocateStackInstruction->getAddressGivenOffsetFromRBP()
              << ", %rsp\n";
}

void PrettyPrinters::printAssyPushInstruction(
    const std::shared_ptr<Assembly::PushInstruction> &pushInstruction) {
    auto operand = pushInstruction->getOperand();
    if (auto stackOperand =
            std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        std::cout << "    pushq" << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto regOperand =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand)) {
        std::cout << "    pushq" << " "
                  << regOperand->getRegisterInBytesInStr(8) << "\n";
    }
    else if (auto immOperand =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                     operand)) {
        std::cout << "    pushq" << " $" << immOperand->getImmediate() << "\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << "    pushq" << " " << identifier << "(%rip)\n";
    }
}

void PrettyPrinters::printAssyCallInstruction(
    const std::shared_ptr<Assembly::CallInstruction> &callInstruction) {
    std::string functionName = callInstruction->getFunctionIdentifier();
    prependUnderscoreToIdentifierIfMacOS(functionName);
    std::cout << "    call " << functionName;
// If the underlying OS is Linux, add the `@PLT` suffix (PLT modifier) to the
// operand.
#ifdef __linux__
    std::cout << "@PLT";
#endif
    std::cout << "\n";
}

void PrettyPrinters::printAssyUnaryInstruction(
    const std::shared_ptr<Assembly::UnaryInstruction> &unaryInstruction) {
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
        std::cout << " " << regOperand->getRegisterInBytesInStr(4) << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
}

void PrettyPrinters::printAssyBinaryInstruction(
    const std::shared_ptr<Assembly::BinaryInstruction> &binaryInstruction) {
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
        std::cout << " " << operand1Reg->getRegisterInBytesInStr(4) << ",";
    }
    else if (auto operand1Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand1)) {
        std::cout << " " << operand1Stack->getOffset() << "("
                  << operand1Stack->getReservedRegisterInStr() << "),";
    }
    else if (auto operand1Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip),";
    }

    auto operand2 = binaryInstruction->getOperand2();
    if (auto operand2Reg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand2)) {
        std::cout << " " << operand2Reg->getRegisterInBytesInStr(4) << "\n";
    }
    else if (auto operand2Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand2)) {
        std::cout << " " << operand2Stack->getOffset() << "("
                  << operand2Stack->getReservedRegisterInStr() << ")\n";
    }
    else if (auto operand2Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
}

void PrettyPrinters::printAssyCmpInstruction(
    const std::shared_ptr<Assembly::CmpInstruction> &cmpInstruction) {
    std::cout << "    cmpl";

    auto operand1 = cmpInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        std::cout << " $" << operand1Imm->getImmediate();
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        std::cout << " " << operand1Reg->getRegisterInBytesInStr(4);
    }
    else if (auto operand1Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand1)) {
        std::cout << " " << operand1Stack->getOffset() << "("
                  << operand1Stack->getReservedRegisterInStr() << ")";
    }
    else if (auto operand1Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)";
    }

    std::cout << ",";

    auto operand2 = cmpInstruction->getOperand2();
    if (auto operand2Reg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand2)) {
        std::cout << " " << operand2Reg->getRegisterInBytesInStr(4) << "\n";
    }
    else if (auto operand2Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand2)) {
        std::cout << " " << operand2Stack->getOffset() << "("
                  << operand2Stack->getReservedRegisterInStr() << ")\n";
    }
    else if (auto operand2Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
}

void PrettyPrinters::printAssyIdivInstruction(
    const std::shared_ptr<Assembly::IdivInstruction> &idivInstruction) {
    std::cout << "    idivl";

    auto operand = idivInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        std::cout << " " << regOperand->getRegisterInBytesInStr(4) << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
}

void PrettyPrinters::printAssyCdqInstruction() { std::cout << "    cdq\n"; }

void PrettyPrinters::printAssyJmpInstruction(
    const std::shared_ptr<Assembly::JmpInstruction> &jmpInstruction) {
    auto label = jmpInstruction->getLabel();
    std::cout << "    jmp .L" << label << "\n";
}

void PrettyPrinters::printAssyJmpCCInstruction(
    const std::shared_ptr<Assembly::JmpCCInstruction> &jmpCCInstruction) {
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
    const std::shared_ptr<Assembly::SetCCInstruction> &setCCInstruction) {
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
        std::cout << " " << regOperand->getRegisterInBytesInStr(1) << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
}

void PrettyPrinters::printAssyLabelInstruction(
    const std::shared_ptr<Assembly::LabelInstruction> &labelInstruction) {
    auto label = labelInstruction->getLabel();
    std::cout << ".L" << label << ":\n";
}

void PrettyPrinters::prependUnderscoreToIdentifierIfMacOS(
    [[maybe_unused]] std::string &identifier) {
// If the underlying OS is macOS, prepend an underscore to the function name.
// Otherwise, leave the function name as is.
#ifdef __APPLE__
    identifier = "_" + std::move(identifier);
#endif
}
/*
 * End: Functions to print the assembly program onto the stdout.
 */
