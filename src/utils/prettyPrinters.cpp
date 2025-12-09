#include "prettyPrinters.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

/*
 * Start: Functions to print the IR program to stdout.
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
            throw std::logic_error(
                "Unsupported top-level element while printing IR program");
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
        throw std::logic_error("Unknown static variable initializer type while "
                               "printing IR static variable");
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
        throw std::logic_error(
            "Unknown instruction type while printing IR instruction");
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
            throw std::logic_error(
                "Unknown constant type while printing IR return instruction");
        }
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 returnInstruction->getReturnValue())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::logic_error(
            "Unknown return value type while printing IR return instruction");
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
        throw std::logic_error("Unknown destination value type while printing "
                               "IR sign extend instruction");
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
            throw std::logic_error("Unknown constant type while printing IR "
                                   "sign extend instruction");
        }
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 signExtendInstruction->getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::logic_error("Unknown source value type while printing IR "
                               "sign extend instruction");
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
        throw std::logic_error("Unknown destination value type while printing "
                               "IR truncate instruction");
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
            throw std::logic_error(
                "Unknown constant type while printing IR truncate instruction");
        }
    }
    else if (auto variableValue = std::dynamic_pointer_cast<IR::VariableValue>(
                 truncateInstruction->getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::logic_error(
            "Unknown source value type while printing IR truncate instruction");
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
        throw std::logic_error("Unknown destination value type while printing "
                               "IR unary instruction");
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
        throw std::logic_error(
            "Unknown unary operator while printing IR unary instruction");
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
            throw std::logic_error(
                "Unknown constant type while printing IR unary instruction");
        }
    }
    else {
        throw std::logic_error(
            "Unknown source value type while printing IR unary instruction");
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
            throw std::logic_error(
                "Unknown constant type while printing IR binary instruction");
        }
    }
    else {
        throw std::logic_error(
            "Unknown source value type while printing IR binary instruction");
    }

    if (auto binaryOperator = std::dynamic_pointer_cast<IR::AddOperator>(
            binaryInstruction->getBinaryOperator())) {
        std::cout << " + ";
    }
    else if (auto binaryOperator1 =
                 std::dynamic_pointer_cast<IR::SubtractOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " - ";
    }
    else if (auto binaryOperator2 =
                 std::dynamic_pointer_cast<IR::MultiplyOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " * ";
    }
    else if (auto binaryOperator3 =
                 std::dynamic_pointer_cast<IR::DivideOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " / ";
    }
    else if (auto binaryOperator4 =
                 std::dynamic_pointer_cast<IR::RemainderOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " % ";
    }
    else if (auto binaryOperator5 =
                 std::dynamic_pointer_cast<IR::EqualOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " == ";
    }
    else if (auto binaryOperator6 =
                 std::dynamic_pointer_cast<IR::NotEqualOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " != ";
    }
    else if (auto binaryOperator7 =
                 std::dynamic_pointer_cast<IR::LessThanOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " < ";
    }
    else if (auto binaryOperator8 =
                 std::dynamic_pointer_cast<IR::LessThanOrEqualOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " <= ";
    }
    else if (auto binaryOperator9 =
                 std::dynamic_pointer_cast<IR::GreaterThanOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " > ";
    }
    else if (auto binaryOperator10 =
                 std::dynamic_pointer_cast<IR::GreaterThanOrEqualOperator>(
                     binaryInstruction->getBinaryOperator())) {
        std::cout << " >= ";
    }
    else {
        throw std::logic_error(
            "Unknown binary operator while printing IR binary instruction");
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
            throw std::logic_error(
                "Unknown constant type while printing IR binary instruction");
        }
    }
    else {
        throw std::logic_error(
            "Unknown source value type while printing IR binary instruction");
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
        throw std::logic_error("Unknown/unsupported destination value type "
                               "while printing IR copy instruction");
    }

    std::cout << " = ";

    auto src = copyInstruction->getSrc();
    if (src == nullptr) {
        throw std::logic_error(
            "Source value is null while printing IR copy instruction");
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
            throw std::logic_error(
                "Unknown constant type while printing IR copy instruction");
        }
    }
    else if (auto variableValue =
                 std::dynamic_pointer_cast<IR::VariableValue>(src)) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        throw std::logic_error(
            "Unknown source value type while printing IR copy instruction");
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
            throw std::logic_error("Unknown constant type while printing IR "
                                   "jump if zero instruction");
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
            throw std::logic_error("Unknown constant type while printing IR "
                                   "jump if not zero instruction");
        }
    }
    else {
        throw std::logic_error("Unknown condition value type while printing IR "
                               "jump if not zero instruction");
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
        throw std::logic_error("Unknown destination value type while printing "
                               "IR function call instruction");
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
                throw std::logic_error("Unknown constant type while printing "
                                       "IR function call argument");
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
 * End: Functions to print the IR program to stdout.
 */

/*
 * Start: Functions to print the assembly program to stdout.
 */
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
    auto alignment = staticVariable->getAlignment();
    auto alignmentInStr = std::to_string(alignment);
    auto alignDirective = ".align " + alignmentInStr;
// If the underlying OS is macOS, use the `.balign 4` directive instead of the
// `.align 4` directive.
#ifdef __APPLE__
    alignDirective = ".balign " + alignmentInStr;
#endif

    auto variableIdentifier = staticVariable->getIdentifier();
    prependUnderscoreToIdentifierIfMacOS(variableIdentifier);

    auto global = staticVariable->isGlobal();
    auto globalDirective = ".globl " + variableIdentifier + "\n";
    if (!global) {
        globalDirective = "";
    }

    auto staticInit = staticVariable->getStaticInit();
    int initialValue;
    if (auto intInit = std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
        initialValue = std::get<int>(intInit->getValue());
    }
    else if (auto longInit =
                 std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
        initialValue = static_cast<int>(std::get<long>(longInit->getValue()));
    }
    else {
        throw std::logic_error(
            "Unknown static init type while printing IR static variable");
    }

    std::cout << "\n";
    if (initialValue != 0) {
        std::cout << globalDirective;
        std::cout << "    .data\n";
        std::cout << "    " << alignDirective << "\n";
        std::cout << variableIdentifier << ":\n";
        if (auto intInit =
                std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
            if (std::get<int>(intInit->getValue()) == 0) {
                std::cout << "    .zero 4\n";
            }
            else {
                std::cout << "    .long " << std::get<int>(intInit->getValue())
                          << "\n";
            }
        }
        else if (auto longInit =
                     std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
            if (std::get<long>(longInit->getValue()) == 0) {
                std::cout << "    .zero 8\n";
            }
            else {
                std::cout << "    .quad "
                          << std::get<long>(longInit->getValue()) << "\n";
            }
        }
    }
    else if (initialValue == 0) {
        std::cout << globalDirective;
        std::cout << "    .bss\n";
        std::cout << "    " << alignDirective << "\n";
        std::cout << variableIdentifier << ":\n";
        if (auto intInit =
                std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
            std::cout << "    .zero 4\n";
        }
        else if (auto longInit =
                     std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
            std::cout << "    .zero 8\n";
        }
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
        printAssyRetInstruction(retInstruction);
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
    else if (auto movsxInstruction =
                 std::dynamic_pointer_cast<Assembly::MovsxInstruction>(
                     instruction)) {
        printAssyMovsxInstruction(movsxInstruction);
    }
    else if (auto cdqInstruction =
                 std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                     instruction)) {
        printAssyCdqInstruction(cdqInstruction);
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
    else {
        throw std::logic_error(
            "Invalid instruction type while printing assembly instruction");
    }
}

void PrettyPrinters::printAssyMovInstruction(
    const std::shared_ptr<Assembly::MovInstruction> &movInstruction) {
    auto type = movInstruction->getType();

    std::string instructionName;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        instructionName = "movl";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        instructionName = "movq";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly mov instruction");
    }

    auto src = movInstruction->getSrc();
    std::string srcStr;
    if (auto srcReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(registerSize);
    }
    else if (auto srcImm =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src)) {
        // Use long values for quadword instructions and int values for longword
        // instructions in order to avoid (potential) overflow issues.
        if (auto quadword =
                std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
            srcStr = "$" + std::to_string(srcImm->getImmediateLong());
        }
        else {
            srcStr = "$" + std::to_string(srcImm->getImmediate());
        }
    }
    else if (auto srcStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (auto srcData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Invalid source type while printing assembly mov instruction");
    }

    auto dst = movInstruction->getDst();
    std::string dstStr;
    if (auto dstReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(registerSize);
    }
    else if (auto dstStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (auto dstData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Invalid destination type while printing assembly mov instruction");
    }

    std::cout << "    " << instructionName << " " << srcStr << ", " << dstStr
              << "\n";
}

void PrettyPrinters::printAssyMovsxInstruction(
    const std::shared_ptr<Assembly::MovsxInstruction> &movsxInstruction) {
    auto src = movsxInstruction->getSrc();
    std::string srcStr;
    if (auto srcReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(4);
    }
    else if (auto srcImm =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src)) {
        srcStr = "$" + std::to_string(srcImm->getImmediateLong());
    }
    else if (auto srcStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (auto srcData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        std::stringstream msg;
        msg << "Invalid source type while printing assembly movsx instruction";
        if (src == nullptr) {
            msg << ": Source operand is nullptr";
        }
        else {
            msg << ": Source operand is not nullptr but unknown type";
        }
        throw std::logic_error(msg.str());
    }

    auto dst = movsxInstruction->getDst();
    std::string dstStr;
    if (auto dstReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(8);
    }
    else if (auto dstStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (auto dstData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error("Invalid destination type while printing "
                               "assembly movsx instruction");
    }

    std::cout << "    movslq " << srcStr << ", " << dstStr << "\n";
}

void PrettyPrinters::printAssyRetInstruction(
    [[maybe_unused]] const std::shared_ptr<Assembly::RetInstruction>
        &retInstruction) {
    // Print the function epilogue before printing the return
    // instruction.
    std::cout << "    movq %rbp, %rsp\n";
    std::cout << "    popq %rbp\n";
    std::cout << "    ret\n";
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
        std::cout << "    pushq" << " $" << immOperand->getImmediateLong()
                  << "\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << "    pushq" << " " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly push instruction");
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
    auto type = unaryInstruction->getType();

    std::string instructionName;
    if (auto negateOperator =
            std::dynamic_pointer_cast<Assembly::NegateOperator>(
                unaryOperator)) {
        instructionName = "neg";
    }
    else if (auto complementOperator =
                 std::dynamic_pointer_cast<Assembly::ComplementOperator>(
                     unaryOperator)) {
        instructionName = "not";
    }
    else if (auto notOperator =
                 std::dynamic_pointer_cast<Assembly::NotOperator>(
                     unaryOperator)) {
        instructionName = "not";
    }
    else {
        throw std::logic_error(
            "Invalid unary operator while printing assembly unary instruction");
    }

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly unary instruction");
    }

    std::cout << "    " << instructionName << typeSuffix;

    auto operand = unaryInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        std::cout << " " << regOperand->getRegisterInBytesInStr(registerSize)
                  << "\n";
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
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly unary instruction");
    }
}

void PrettyPrinters::printAssyBinaryInstruction(
    const std::shared_ptr<Assembly::BinaryInstruction> &binaryInstruction) {
    auto binaryOperator = binaryInstruction->getBinaryOperator();
    auto type = binaryInstruction->getType();

    std::string instructionName;
    if (auto addOperator =
            std::dynamic_pointer_cast<Assembly::AddOperator>(binaryOperator)) {
        instructionName = "add";
    }
    else if (auto subtractOperator =
                 std::dynamic_pointer_cast<Assembly::SubtractOperator>(
                     binaryOperator)) {
        instructionName = "sub";
    }
    else if (auto multiplyOperator =
                 std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                     binaryOperator)) {
        instructionName = "imul";
    }
    else {
        throw std::logic_error("Invalid binary operator while printing "
                               "assembly binary instruction");
    }

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly binary instruction");
    }

    std::cout << "    " << instructionName << typeSuffix;

    auto operand1 = binaryInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        if (auto quadword =
                std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
            std::cout << " $" << operand1Imm->getImmediateLong() << ",";
        }
        else {
            std::cout << " $" << operand1Imm->getImmediate() << ",";
        }
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        std::cout << " " << operand1Reg->getRegisterInBytesInStr(registerSize)
                  << ",";
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
        std::cout << " " << operand2Reg->getRegisterInBytesInStr(registerSize)
                  << "\n";
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
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly binary instruction");
    }
}

void PrettyPrinters::printAssyCmpInstruction(
    const std::shared_ptr<Assembly::CmpInstruction> &cmpInstruction) {
    auto type = cmpInstruction->getType();

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly cmp instruction");
    }

    std::cout << "    cmp" << typeSuffix;

    auto operand1 = cmpInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        if (auto quadword =
                std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
            std::cout << " $" << operand1Imm->getImmediateLong();
        }
        else {
            std::cout << " $" << operand1Imm->getImmediate();
        }
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        std::cout << " " << operand1Reg->getRegisterInBytesInStr(registerSize);
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
        std::cout << " " << operand2Reg->getRegisterInBytesInStr(registerSize)
                  << "\n";
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
    auto type = idivInstruction->getType();

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly idiv instruction");
    }

    std::cout << "    idiv" << typeSuffix;

    auto operand = idivInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        std::cout << " " << regOperand->getRegisterInBytesInStr(registerSize)
                  << "\n";
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
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly idiv instruction");
    }
}

void PrettyPrinters::printAssyCdqInstruction(
    const std::shared_ptr<Assembly::CdqInstruction> &cdqInstruction) {
    auto type = cdqInstruction->getType();

    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        std::cout << "    cdq\n";
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        std::cout << "    cqo\n";
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly cdq instruction");
    }
}

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
    else {
        throw std::logic_error("Invalid conditional code while printing "
                               "assembly jmpcc instruction");
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
    else {
        throw std::logic_error("Invalid conditional code while printing "
                               "assembly setcc instruction");
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
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly setcc instruction");
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
    identifier = "_" + identifier;
#endif
}
/*
 * End: Functions to print the assembly program to stdout.
 */
