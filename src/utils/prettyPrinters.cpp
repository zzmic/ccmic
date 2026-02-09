#include "prettyPrinters.h"
#include "../backend/assembly.h"
#include "../frontend/constant.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../midend/ir.h"
#include "../utils/constants.h"
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

/*
 * Start: Functions to print the IR program to stdout.
 */
void PrettyPrinters::printIRProgram(
    const IR::Program &irProgram,
    const std::vector<std::unique_ptr<IR::StaticVariable>> &irStaticVariables) {
    for (const auto &topLevel : irProgram.getTopLevels()) {
        if (auto *functionDefinition =
                dynamic_cast<IR::FunctionDefinition *>(topLevel.get())) {
            printIRFunctionDefinition(*functionDefinition);
        }
        else {
            const auto &r = *topLevel;
            throw std::logic_error("Unsupported top-level element while "
                                   "printing IR program in printIRProgram: " +
                                   std::string(typeid(r).name()));
        }
    }

    for (const auto &irStaticVariable : irStaticVariables) {
        printIRStaticVariable(*irStaticVariable);
    }
}

void PrettyPrinters::printIRFunctionDefinition(
    const IR::FunctionDefinition &functionDefinition) {
    std::cout << functionDefinition.getFunctionIdentifier();
    std::cout << std::boolalpha;
    std::cout << "[isGlobal: " << functionDefinition.isGlobal() << "]";
    std::cout << "(";

    const auto &parameters = functionDefinition.getParameterIdentifiers();
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        const auto &parameter = *it;
        std::cout << parameter;
        const bool isLast = (std::next(it) == parameters.end());
        if (!isLast) {
            std::cout << ", ";
        }
    }

    std::cout << "):\n";
    for (const auto &instruction : functionDefinition.getFunctionBody()) {
        printIRInstruction(*instruction);
    }
}

void PrettyPrinters::printIRStaticVariable(
    const IR::StaticVariable &staticVariable) {
    const auto *staticInit = staticVariable.getStaticInit();
    std::cout << "[static] " << staticVariable.getIdentifier() << " = ";

    if (const auto *intInit = dynamic_cast<const AST::IntInit *>(staticInit)) {
        std::cout << std::get<int>(intInit->getValue());
    }
    else if (const auto *longInit =
                 dynamic_cast<const AST::LongInit *>(staticInit)) {
        std::cout << std::get<long>(longInit->getValue());
    }
    else if (const auto *uintInit =
                 dynamic_cast<const AST::UIntInit *>(staticInit)) {
        std::cout << std::get<unsigned int>(uintInit->getValue());
    }
    else if (const auto *ulongInit =
                 dynamic_cast<const AST::ULongInit *>(staticInit)) {
        std::cout << std::get<unsigned long>(ulongInit->getValue());
    }
    else {
        const auto &r = *staticInit;
        throw std::logic_error(
            "Unsupported static variable initializer type while printing IR "
            "static variable in printIRStaticVariable: " +
            std::string(typeid(r).name()));
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRInstruction(const IR::Instruction &instruction) {
    if (const auto *returnInstruction =
            dynamic_cast<const IR::ReturnInstruction *>(&instruction)) {
        printIRReturnInstruction(*returnInstruction);
    }
    else if (const auto *unaryInstruction =
                 dynamic_cast<const IR::UnaryInstruction *>(&instruction)) {
        printIRUnaryInstruction(*unaryInstruction);
    }
    else if (const auto *binaryInstruction =
                 dynamic_cast<const IR::BinaryInstruction *>(&instruction)) {
        printIRBinaryInstruction(*binaryInstruction);
    }
    else if (const auto *copyInstruction =
                 dynamic_cast<const IR::CopyInstruction *>(&instruction)) {
        printIRCopyInstruction(*copyInstruction);
    }
    else if (const auto *jumpInstruction =
                 dynamic_cast<const IR::JumpInstruction *>(&instruction)) {
        printIRJumpInstruction(*jumpInstruction);
    }
    else if (const auto *jumpIfZeroInstruction =
                 dynamic_cast<const IR::JumpIfZeroInstruction *>(
                     &instruction)) {
        printIRJumpIfZeroInstruction(*jumpIfZeroInstruction);
    }
    else if (const auto *jumpIfNotZeroInstruction =
                 dynamic_cast<const IR::JumpIfNotZeroInstruction *>(
                     &instruction)) {
        printIRJumpIfNotZeroInstruction(*jumpIfNotZeroInstruction);
    }
    else if (const auto *labelInstruction =
                 dynamic_cast<const IR::LabelInstruction *>(&instruction)) {
        printIRLabelInstruction(*labelInstruction);
    }
    else if (const auto *functionCallInstruction =
                 dynamic_cast<const IR::FunctionCallInstruction *>(
                     &instruction)) {
        printIRFunctionCallInstruction(*functionCallInstruction);
    }
    else if (const auto *signExtendInstruction =
                 dynamic_cast<const IR::SignExtendInstruction *>(
                     &instruction)) {
        printIRSignExtendInstruction(*signExtendInstruction);
    }
    else if (const auto *truncateInstruction =
                 dynamic_cast<const IR::TruncateInstruction *>(&instruction)) {
        printIRTruncateInstruction(*truncateInstruction);
    }
    else if (const auto *zeroExtendInstruction =
                 dynamic_cast<const IR::ZeroExtendInstruction *>(
                     &instruction)) {
        printIRZeroExtendInstruction(*zeroExtendInstruction);
    }
    else {
        const auto &r = *&instruction;
        throw std::logic_error("Unsupported instruction type while printing IR "
                               "instruction in printIRInstruction: " +
                               std::string(typeid(r).name()));
    }
}

void PrettyPrinters::printIRReturnInstruction(
    const IR::ReturnInstruction &returnInstruction) {
    std::cout << "    return ";

    if (const auto *constantValue = dynamic_cast<const IR::ConstantValue *>(
            returnInstruction.getReturnValue())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR return "
                "instruction in printIRReturnInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else if (const auto *variableValue =
                 dynamic_cast<const IR::VariableValue *>(
                     returnInstruction.getReturnValue())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *returnInstruction.getReturnValue();
        throw std::logic_error(
            "Unsupported return value type while printing IR return "
            "instruction in printIRReturnInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRSignExtendInstruction(
    const IR::SignExtendInstruction &signExtendInstruction) {
    std::cout << "    ";
    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            signExtendInstruction.getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *signExtendInstruction.getDst();
        throw std::logic_error(
            "Unsupported destination value type while printing IR sign extend "
            "instruction in printIRSignExtendInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << " = SignExtend(";

    if (const auto *constantValue = dynamic_cast<const IR::ConstantValue *>(
            signExtendInstruction.getSrc())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR sign extend "
                "instruction in printIRSignExtendInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else if (const auto *variableValue =
                 dynamic_cast<const IR::VariableValue *>(
                     signExtendInstruction.getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *signExtendInstruction.getSrc();
        throw std::logic_error(
            "Unsupported source value type while printing IR sign extend "
            "instruction in printIRSignExtendInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << ")\n";
}

void PrettyPrinters::printIRTruncateInstruction(
    const IR::TruncateInstruction &truncateInstruction) {
    std::cout << "    ";
    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            truncateInstruction.getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *truncateInstruction.getDst();
        throw std::logic_error(
            "Unsupported destination value type while printing IR truncate "
            "instruction in printIRTruncateInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << " = Truncate(";

    if (const auto *constantValue = dynamic_cast<const IR::ConstantValue *>(
            truncateInstruction.getSrc())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR truncate "
                "instruction in printIRTruncateInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else if (const auto *variableValue =
                 dynamic_cast<const IR::VariableValue *>(
                     truncateInstruction.getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *truncateInstruction.getSrc();
        throw std::logic_error(
            "Unsupported source value type while printing IR truncate "
            "instruction in printIRTruncateInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << ")\n";
}

void PrettyPrinters::printIRZeroExtendInstruction(
    const IR::ZeroExtendInstruction &zeroExtendInstruction) {
    std::cout << "    ";
    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            zeroExtendInstruction.getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *zeroExtendInstruction.getDst();
        throw std::logic_error(
            "Unsupported destination value type while printing IR zero extend "
            "instruction in printIRZeroExtendInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << " = ZeroExtend(";

    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            zeroExtendInstruction.getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (const auto *constantValue =
                 dynamic_cast<const IR::ConstantValue *>(
                     zeroExtendInstruction.getSrc())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR zero extend "
                "instruction in printIRZeroExtendInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else {
        const auto &r = *zeroExtendInstruction.getSrc();
        throw std::logic_error(
            "Unsupported source value type while printing IR zero extend "
            "instruction in printIRZeroExtendInstruction: " +
            std::string(typeid(r).name()));
    }
    std::cout << ")\n";
}

void PrettyPrinters::printIRUnaryInstruction(
    const IR::UnaryInstruction &unaryInstruction) {
    std::cout << "    ";

    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            unaryInstruction.getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *unaryInstruction.getDst();
        throw std::logic_error(
            "Unsupported destination value type while printing IR unary "
            "instruction in printIRUnaryInstruction: " +
            std::string(typeid(r).name()));
    }

    if (dynamic_cast<const IR::ComplementOperator *>(
            unaryInstruction.getUnaryOperator()) != nullptr) {
        std::cout << " = ~";
    }
    else if (dynamic_cast<const IR::NegateOperator *>(
                 unaryInstruction.getUnaryOperator()) != nullptr) {
        std::cout << " = -";
    }
    else if (dynamic_cast<const IR::NotOperator *>(
                 unaryInstruction.getUnaryOperator()) != nullptr) {
        std::cout << " = !";
    }
    else {
        const auto &r = *unaryInstruction.getUnaryOperator();
        throw std::logic_error(
            "Unsupported unary operator while printing IR unary instruction in "
            "printIRUnaryInstruction: " +
            std::string(typeid(r).name()));
    }

    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            unaryInstruction.getSrc())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (const auto *constantValue =
                 dynamic_cast<const IR::ConstantValue *>(
                     unaryInstruction.getSrc())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
            std::cout << "\n";
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
            std::cout << "\n";
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR unary instruction "
                "in printIRUnaryInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else {
        const auto &r = *unaryInstruction.getSrc();
        throw std::logic_error(
            "Unsupported source value type while printing IR unary instruction "
            "in printIRUnaryInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRBinaryInstruction(
    const IR::BinaryInstruction &binaryInstruction) {
    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            binaryInstruction.getDst())) {
        std::cout << "    " << variableValue->getIdentifier();
    }

    std::cout << " = ";

    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            binaryInstruction.getSrc1())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (const auto *constantValue =
                 dynamic_cast<const IR::ConstantValue *>(
                     binaryInstruction.getSrc1())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR binary "
                "instruction in printIRBinaryInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else {
        const auto &r = *binaryInstruction.getSrc1();
        throw std::logic_error(
            "Unsupported source value type while printing IR binary "
            "instruction in printIRBinaryInstruction: " +
            std::string(typeid(r).name()));
    }

    if (dynamic_cast<const IR::AddOperator *>(
            binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " + ";
    }
    else if (dynamic_cast<const IR::SubtractOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " - ";
    }
    else if (dynamic_cast<const IR::MultiplyOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " * ";
    }
    else if (dynamic_cast<const IR::DivideOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " / ";
    }
    else if (dynamic_cast<const IR::RemainderOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " % ";
    }
    else if (dynamic_cast<const IR::EqualOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " == ";
    }
    else if (dynamic_cast<const IR::NotEqualOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " != ";
    }
    else if (dynamic_cast<const IR::LessThanOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " < ";
    }
    else if (dynamic_cast<const IR::LessThanOrEqualOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " <= ";
    }
    else if (dynamic_cast<const IR::GreaterThanOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " > ";
    }
    else if (dynamic_cast<const IR::GreaterThanOrEqualOperator *>(
                 binaryInstruction.getBinaryOperator()) != nullptr) {
        std::cout << " >= ";
    }
    else {
        const auto &r = *binaryInstruction.getBinaryOperator();
        throw std::logic_error(
            "Unsupported binary operator while printing IR binary instruction "
            "in printIRBinaryInstruction: " +
            std::string(typeid(r).name()));
    }

    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            binaryInstruction.getSrc2())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (const auto *constantValue =
                 dynamic_cast<const IR::ConstantValue *>(
                     binaryInstruction.getSrc2())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR binary "
                "instruction in printIRBinaryInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else {
        const auto &r = *binaryInstruction.getSrc2();
        throw std::logic_error(
            "Unsupported source value type while printing IR binary "
            "instruction in printIRBinaryInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRCopyInstruction(
    const IR::CopyInstruction &copyInstruction) {
    std::cout << "    ";
    if (const auto *variableValue =
            dynamic_cast<const IR::VariableValue *>(copyInstruction.getDst())) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *copyInstruction.getDst();
        throw std::logic_error(
            "Unsupported destination value type while printing IR copy "
            "instruction in printIRCopyInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << " = ";

    auto *src = copyInstruction.getSrc();
    if (src == nullptr) {
        const auto &r = *src;
        throw std::logic_error("Source value is null while printing IR copy "
                               "instruction in printIRCopyInstruction: " +
                               std::string(typeid(r).name()));
    }

    if (const auto *constantValue =
            dynamic_cast<const IR::ConstantValue *>(src)) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR copy instruction "
                "in printIRCopyInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else if (const auto *variableValue =
                 dynamic_cast<const IR::VariableValue *>(src)) {
        std::cout << variableValue->getIdentifier();
    }
    else {
        const auto &r = *src;
        throw std::logic_error(
            "Unsupported source value type while printing IR copy instruction "
            "in printIRCopyInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << "\n";
}

void PrettyPrinters::printIRJumpInstruction(
    const IR::JumpInstruction &jumpInstruction) {
    std::cout << "    Jump(" << jumpInstruction.getTarget() << ")\n";
}

void PrettyPrinters::printIRJumpIfZeroInstruction(
    const IR::JumpIfZeroInstruction &jumpIfZeroInstruction) {
    std::cout << "    JumpIfZero(";

    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            jumpIfZeroInstruction.getCondition())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (const auto *constantValue =
                 dynamic_cast<const IR::ConstantValue *>(
                     jumpIfZeroInstruction.getCondition())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR jump if zero "
                "instruction in printIRJumpIfZeroInstruction: " +
                std::string(typeid(r).name()));
        }
    }

    std::cout << ", " << jumpIfZeroInstruction.getTarget() << ")\n";
}

void PrettyPrinters::printIRJumpIfNotZeroInstruction(
    const IR::JumpIfNotZeroInstruction &jumpIfNotZeroInstruction) {
    std::cout << "    JumpIfNotZero(";

    if (const auto *variableValue = dynamic_cast<const IR::VariableValue *>(
            jumpIfNotZeroInstruction.getCondition())) {
        std::cout << variableValue->getIdentifier();
    }
    else if (const auto *constantValue =
                 dynamic_cast<const IR::ConstantValue *>(
                     jumpIfNotZeroInstruction.getCondition())) {
        if (const auto *constantInt = dynamic_cast<const AST::ConstantInt *>(
                constantValue->getASTConstant())) {
            std::cout << constantInt->getValue();
        }
        else if (const auto *constantLong =
                     dynamic_cast<const AST::ConstantLong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantLong->getValue();
        }
        else if (const auto *constantUInt =
                     dynamic_cast<const AST::ConstantUInt *>(
                         constantValue->getASTConstant())) {
            std::cout << constantUInt->getValue();
        }
        else if (const auto *constantULong =
                     dynamic_cast<const AST::ConstantULong *>(
                         constantValue->getASTConstant())) {
            std::cout << constantULong->getValue();
        }
        else {
            const auto &r = *constantValue->getASTConstant();
            throw std::logic_error(
                "Unsupported constant type while printing IR jump if not zero "
                "instruction in printIRJumpIfNotZeroInstruction: " +
                std::string(typeid(r).name()));
        }
    }
    else {
        const auto &r = *jumpIfNotZeroInstruction.getCondition();
        throw std::logic_error(
            "Unsupported condition value type while printing IR jump if not "
            "zero instruction in printIRJumpIfNotZeroInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << ", " << jumpIfNotZeroInstruction.getTarget() << ")\n";
}

void PrettyPrinters::printIRLabelInstruction(
    const IR::LabelInstruction &labelInstruction) {
    std::cout << "    Label(" << labelInstruction.getLabel() << ")\n";
}

void PrettyPrinters::printIRFunctionCallInstruction(
    const IR::FunctionCallInstruction &functionCallInstruction) {
    auto *dst = functionCallInstruction.getDst();

    if (const auto *variableValue =
            dynamic_cast<const IR::VariableValue *>(dst)) {
        std::cout << "    " << variableValue->getIdentifier() << " = ";
    }
    else {
        throw std::logic_error(
            "Unsupported destination value type while printing IR function "
            "call instruction in printIRFunctionCallInstruction");
    }

    const auto &functionIdentifier =
        functionCallInstruction.getFunctionIdentifier();
    std::cout << functionIdentifier << "(";

    const auto &args = functionCallInstruction.getArgs();
    for (auto it = args.begin(); it != args.end(); ++it) {
        const auto &arg = *it;
        if (const auto *variableValue =
                dynamic_cast<const IR::VariableValue *>(arg.get())) {
            std::cout << variableValue->getIdentifier();
        }
        else if (const auto *constantValue =
                     dynamic_cast<const IR::ConstantValue *>(arg.get())) {
            if (const auto *constantInt =
                    dynamic_cast<const AST::ConstantInt *>(
                        constantValue->getASTConstant())) {
                std::cout << constantInt->getValue();
            }
            else if (const auto *constantLong =
                         dynamic_cast<const AST::ConstantLong *>(
                             constantValue->getASTConstant())) {
                std::cout << constantLong->getValue();
            }
            else if (const auto *constantUInt =
                         dynamic_cast<const AST::ConstantUInt *>(
                             constantValue->getASTConstant())) {
                std::cout << constantUInt->getValue();
            }
            else if (const auto *constantULong =
                         dynamic_cast<const AST::ConstantULong *>(
                             constantValue->getASTConstant())) {
                std::cout << constantULong->getValue();
            }
            else {
                const auto &r = *constantValue->getASTConstant();
                throw std::logic_error(
                    "Unsupported constant type while printing IR function call "
                    "argument in printIRFunctionCallArgument: " +
                    std::string(typeid(r).name()));
            }
        }
        const bool isLast = (std::next(it) == args.end());
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
    const Assembly::Program &assemblyProgram) {
    const auto &topLevels = assemblyProgram.getTopLevels();

    for (const auto &topLevel : topLevels) {
        if (auto *functionDefinition =
                dynamic_cast<Assembly::FunctionDefinition *>(topLevel.get())) {
            printAssyFunctionDefinition(*functionDefinition);
        }
        else if (auto *staticVariable =
                     dynamic_cast<Assembly::StaticVariable *>(topLevel.get())) {
            printAssyStaticVariable(*staticVariable);
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
    const Assembly::FunctionDefinition &functionDefinition) {
    std::string functionName = functionDefinition.getFunctionIdentifier();
    prependUnderscoreToIdentifierIfMacOS(functionName);
    auto isGlobal = functionDefinition.isGlobal();
    auto globalDirective = "    .globl " + functionName + "\n";
    if (!isGlobal) {
        globalDirective = "";
    }

    // Print the function prologue (before printing the function body).
    std::cout << "\n" << globalDirective;
    std::cout << "    .text\n";
    std::cout << functionName << ":\n";
    std::cout << "    pushq %rbp\n";
    std::cout << "    movq %rsp, %rbp\n";

    for (const auto &instruction : functionDefinition.getFunctionBody()) {
        printAssyInstruction(*instruction);
    }
}

void PrettyPrinters::printAssyStaticVariable(
    const Assembly::StaticVariable &staticVariable) {
    auto alignment = staticVariable.getAlignment();
    auto alignmentInStr = std::to_string(alignment);
    auto alignDirective = ".align " + alignmentInStr;
// If the underlying OS is macOS, use the `.balign 4` directive instead of the
// `.align 4` directive.
#ifdef __APPLE__
    alignDirective = ".balign " + alignmentInStr;
#endif

    auto variableIdentifier = staticVariable.getIdentifier();
    prependUnderscoreToIdentifierIfMacOS(variableIdentifier);

    auto isGlobal = staticVariable.isGlobal();
    auto globalDirective = ".globl " + variableIdentifier + "\n";
    if (!isGlobal) {
        globalDirective = "";
    }

    const auto *staticInit = staticVariable.getStaticInit();
    bool isZeroInit = false;
    if (const auto *intInit = dynamic_cast<const AST::IntInit *>(staticInit)) {
        isZeroInit = std::get<int>(intInit->getValue()) == 0;
    }
    else if (const auto *longInit =
                 dynamic_cast<const AST::LongInit *>(staticInit)) {
        isZeroInit = std::get<long>(longInit->getValue()) == 0L;
    }
    else if (const auto *uintInit =
                 dynamic_cast<const AST::UIntInit *>(staticInit)) {
        isZeroInit = std::get<unsigned int>(uintInit->getValue()) == 0U;
    }
    else if (const auto *ulongInit =
                 dynamic_cast<const AST::ULongInit *>(staticInit)) {
        isZeroInit = std::get<unsigned long>(ulongInit->getValue()) == 0UL;
    }
    else {
        const auto &r = *staticInit;
        throw std::logic_error(
            "Unsupported static init type while printing assembly static "
            "variable in printAssyStaticVariable: " +
            std::string(typeid(r).name()));
    }

    std::cout << "\n";
    if (!isZeroInit) {
        std::cout << globalDirective;
        std::cout << "    .data\n";
        std::cout << "    " << alignDirective << "\n";
        std::cout << variableIdentifier << ":\n";
        if (const auto *intInit =
                dynamic_cast<const AST::IntInit *>(staticInit)) {
            std::cout << "    .long " << std::get<int>(intInit->getValue())
                      << "\n";
        }
        else if (const auto *longInit =
                     dynamic_cast<const AST::LongInit *>(staticInit)) {
            std::cout << "    .quad " << std::get<long>(longInit->getValue())
                      << "\n";
        }
        else if (const auto *uintInit =
                     dynamic_cast<const AST::UIntInit *>(staticInit)) {
            std::cout << "    .long "
                      << std::get<unsigned int>(uintInit->getValue()) << "\n";
        }
        else if (const auto *ulongInit =
                     dynamic_cast<const AST::ULongInit *>(staticInit)) {
            std::cout << "    .quad "
                      << std::get<unsigned long>(ulongInit->getValue()) << "\n";
        }
    }
    else if (isZeroInit) {
        std::cout << globalDirective;
        std::cout << "    .bss\n";
        std::cout << "    " << alignDirective << "\n";
        std::cout << variableIdentifier << ":\n";
        if ((dynamic_cast<const AST::IntInit *>(staticInit) != nullptr) ||
            (dynamic_cast<const AST::UIntInit *>(staticInit) != nullptr)) {
            std::cout << "    .zero 4\n";
        }
        else if ((dynamic_cast<const AST::LongInit *>(staticInit) != nullptr) ||
                 (dynamic_cast<const AST::ULongInit *>(staticInit) !=
                  nullptr)) {
            std::cout << "    .zero 8\n";
        }
    }
}

void PrettyPrinters::printAssyInstruction(
    const Assembly::Instruction &instruction) {
    if (const auto *movInstruction =
            dynamic_cast<const Assembly::MovInstruction *>(&instruction)) {
        printAssyMovInstruction(*movInstruction);
    }
    else if (const auto *movsxInstruction =
                 dynamic_cast<const Assembly::MovsxInstruction *>(
                     &instruction)) {
        printAssyMovsxInstruction(*movsxInstruction);
    }
    else if (const auto *retInstruction =
                 dynamic_cast<const Assembly::RetInstruction *>(&instruction)) {
        printAssyRetInstruction(*retInstruction);
    }
    else if (const auto *pushInstruction =
                 dynamic_cast<const Assembly::PushInstruction *>(
                     &instruction)) {
        printAssyPushInstruction(*pushInstruction);
    }
    else if (const auto *callInstruction =
                 dynamic_cast<const Assembly::CallInstruction *>(
                     &instruction)) {
        printAssyCallInstruction(*callInstruction);
    }
    else if (const auto *unaryInstruction =
                 dynamic_cast<const Assembly::UnaryInstruction *>(
                     &instruction)) {
        printAssyUnaryInstruction(*unaryInstruction);
    }
    else if (const auto *binaryInstruction =
                 dynamic_cast<const Assembly::BinaryInstruction *>(
                     &instruction)) {
        printAssyBinaryInstruction(*binaryInstruction);
    }
    else if (const auto *cmpInstruction =
                 dynamic_cast<const Assembly::CmpInstruction *>(&instruction)) {
        printAssyCmpInstruction(*cmpInstruction);
    }
    else if (const auto *idivInstruction =
                 dynamic_cast<const Assembly::IdivInstruction *>(
                     &instruction)) {
        printAssyIdivInstruction(*idivInstruction);
    }
    else if (const auto *divInstruction =
                 dynamic_cast<const Assembly::DivInstruction *>(&instruction)) {
        printAssyDivInstruction(*divInstruction);
    }
    else if (const auto *cdqInstruction =
                 dynamic_cast<const Assembly::CdqInstruction *>(&instruction)) {
        printAssyCdqInstruction(*cdqInstruction);
    }
    else if (const auto *jmpInstruction =
                 dynamic_cast<const Assembly::JmpInstruction *>(&instruction)) {
        printAssyJmpInstruction(*jmpInstruction);
    }
    else if (const auto *jmpCCInstruction =
                 dynamic_cast<const Assembly::JmpCCInstruction *>(
                     &instruction)) {
        printAssyJmpCCInstruction(*jmpCCInstruction);
    }
    else if (const auto *setCCInstruction =
                 dynamic_cast<const Assembly::SetCCInstruction *>(
                     &instruction)) {
        printAssySetCCInstruction(*setCCInstruction);
    }
    else if (const auto *labelInstruction =
                 dynamic_cast<const Assembly::LabelInstruction *>(
                     &instruction)) {
        printAssyLabelInstruction(*labelInstruction);
    }
    else {
        const auto &r = *&instruction;
        throw std::logic_error(
            "Unsupported instruction type while printing assembly instruction "
            "in printAssyInstruction: " +
            std::string(typeid(r).name()));
    }
}

void PrettyPrinters::printAssyMovInstruction(
    const Assembly::MovInstruction &movInstruction) {
    const auto *type = movInstruction.getType();

    std::string instructionName;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        instructionName = "movl";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        instructionName = "movq";
        registerSize = QUADWORD_SIZE;
    }
    else {
        const auto &r = *type;
        throw std::logic_error("Unsupported type while printing assembly mov "
                               "instruction in printAssyMovInstruction: " +
                               std::string(typeid(r).name()));
    }

    const auto *src = movInstruction.getSrc();
    std::string srcStr;
    if (const auto *srcReg =
            dynamic_cast<const Assembly::RegisterOperand *>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(registerSize);
    }
    else if (const auto *srcImm =
                 dynamic_cast<const Assembly::ImmediateOperand *>(src)) {
        srcStr =
            "$" + std::to_string(static_cast<long>(srcImm->getImmediate()));
    }
    else if (const auto *srcStack =
                 dynamic_cast<const Assembly::StackOperand *>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *srcData =
                 dynamic_cast<const Assembly::DataOperand *>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        const auto &r = *src;
        throw std::logic_error(
            "Unsupported source operand type while printing assembly mov "
            "instruction in printAssyMovInstruction: " +
            std::string(typeid(r).name()));
    }

    const auto *dst = movInstruction.getDst();
    std::string dstStr;
    if (const auto *dstReg =
            dynamic_cast<const Assembly::RegisterOperand *>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(registerSize);
    }
    else if (const auto *dstStack =
                 dynamic_cast<const Assembly::StackOperand *>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *dstData =
                 dynamic_cast<const Assembly::DataOperand *>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        const auto &r = *dst;
        throw std::logic_error(
            "Unsupported destination operand type while printing assembly mov "
            "instruction in printAssyMovInstruction: " +
            std::string(typeid(r).name()));
    }

    std::cout << "    " << instructionName << " " << srcStr << ", " << dstStr
              << "\n";
}

void PrettyPrinters::printAssyMovsxInstruction(
    const Assembly::MovsxInstruction &movsxInstruction) {
    const auto *src = movsxInstruction.getSrc();
    std::string srcStr;
    if (const auto *srcReg =
            dynamic_cast<const Assembly::RegisterOperand *>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(LONGWORD_SIZE);
    }
    else if (const auto *srcImm =
                 dynamic_cast<const Assembly::ImmediateOperand *>(src)) {
        srcStr =
            "$" + std::to_string(static_cast<long>(srcImm->getImmediate()));
    }
    else if (const auto *srcStack =
                 dynamic_cast<const Assembly::StackOperand *>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *srcData =
                 dynamic_cast<const Assembly::DataOperand *>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Unsupported source type while printing assembly movsx instruction "
            "in printAssyMovsxInstruction");
    }

    const auto *dst = movsxInstruction.getDst();
    std::string dstStr;
    if (const auto *dstReg =
            dynamic_cast<const Assembly::RegisterOperand *>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(QUADWORD_SIZE);
    }
    else if (const auto *dstStack =
                 dynamic_cast<const Assembly::StackOperand *>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *dstData =
                 dynamic_cast<const Assembly::DataOperand *>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Unsupported destination operand type while printing assembly "
            "movsx instruction in printAssyMovsxInstruction");
    }

    std::cout << "    movslq " << srcStr << ", " << dstStr << "\n";
}

void PrettyPrinters::printAssyRetInstruction(
    [[maybe_unused]] const Assembly::RetInstruction &retInstruction) {
    // Print the function epilogue before printing the return
    // instruction.
    std::cout << "    movq %rbp, %rsp\n";
    std::cout << "    popq %rbp\n";
    std::cout << "    ret\n";
}

void PrettyPrinters::printAssyPushInstruction(
    const Assembly::PushInstruction &pushInstruction) {
    const auto *operand = pushInstruction.getOperand();
    if (const auto *stackOperand =
            dynamic_cast<const Assembly::StackOperand *>(operand)) {
        std::cout << "    pushq" << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *regOperand =
                 dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        std::cout << "    pushq" << " "
                  << regOperand->getRegisterInBytesInStr(QUADWORD_SIZE) << "\n";
    }
    else if (const auto *immOperand =
                 dynamic_cast<const Assembly::ImmediateOperand *>(operand)) {
        std::cout << "    pushq" << " $"
                  << static_cast<long>(immOperand->getImmediate()) << "\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << "    pushq" << " " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Unsupported operand type while printing assembly push instruction "
            "in printAssyPushInstruction");
    }
}

void PrettyPrinters::printAssyCallInstruction(
    const Assembly::CallInstruction &callInstruction) {
    std::string functionName = callInstruction.getFunctionIdentifier();
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
    const Assembly::UnaryInstruction &unaryInstruction) {
    const auto *unaryOperator = unaryInstruction.getUnaryOperator();
    const auto *type = unaryInstruction.getType();

    std::string instructionName;
    if (dynamic_cast<const Assembly::NegateOperator *>(unaryOperator) !=
        nullptr) {
        instructionName = "neg";
    }
    else if ((dynamic_cast<const Assembly::ComplementOperator *>(
                  unaryOperator) != nullptr) ||
             (dynamic_cast<const Assembly::NotOperator *>(unaryOperator) !=
              nullptr)) {
        instructionName = "not";
    }
    else {
        throw std::logic_error(
            "Unsupported unary operator while printing assembly unary "
            "instruction in printAssyUnaryInstruction");
    }

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error("Unsupported type while printing assembly unary "
                               "instruction in printAssyUnaryInstruction");
    }

    std::cout << "    " << instructionName << typeSuffix;

    const auto *operand = unaryInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        std::cout << " " << regOperand->getRegisterInBytesInStr(registerSize)
                  << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly unary "
            "instruction in printAssyUnaryInstruction: " +
            std::string(typeid(r).name()));
    }
}

void PrettyPrinters::printAssyBinaryInstruction(
    const Assembly::BinaryInstruction &binaryInstruction) {
    const auto *binaryOperator = binaryInstruction.getBinaryOperator();
    const auto *type = binaryInstruction.getType();

    std::string instructionName;
    if (dynamic_cast<const Assembly::AddOperator *>(binaryOperator) !=
        nullptr) {
        instructionName = "add";
    }
    else if (dynamic_cast<const Assembly::SubtractOperator *>(binaryOperator) !=
             nullptr) {
        instructionName = "sub";
    }
    else if (dynamic_cast<const Assembly::MultiplyOperator *>(binaryOperator) !=
             nullptr) {
        instructionName = "imul";
    }
    else {
        throw std::logic_error(
            "Unsupported binary operator while printing assembly binary "
            "instruction in printAssyBinaryInstruction");
    }

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly binary instruction in "
            "printAssyBinaryInstruction");
    }

    std::cout << "    " << instructionName << typeSuffix;

    const auto *operand1 = binaryInstruction.getOperand1();
    if (const auto *operand1Imm =
            dynamic_cast<const Assembly::ImmediateOperand *>(operand1)) {
        std::cout << " $" << static_cast<long>(operand1Imm->getImmediate())
                  << ",";
    }
    else if (const auto *operand1Reg =
                 dynamic_cast<const Assembly::RegisterOperand *>(operand1)) {
        std::cout << " " << operand1Reg->getRegisterInBytesInStr(registerSize)
                  << ",";
    }
    else if (const auto *operand1Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand1)) {
        std::cout << " " << operand1Stack->getOffset() << "("
                  << operand1Stack->getReservedRegisterInStr() << "),";
    }
    else if (const auto *operand1Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip),";
    }

    const auto *operand2 = binaryInstruction.getOperand2();
    if (const auto *operand2Reg =
            dynamic_cast<const Assembly::RegisterOperand *>(operand2)) {
        std::cout << " " << operand2Reg->getRegisterInBytesInStr(registerSize)
                  << "\n";
    }
    else if (const auto *operand2Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand2)) {
        std::cout << " " << operand2Stack->getOffset() << "("
                  << operand2Stack->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *operand2Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand2;
        throw std::logic_error(
            "Unsupported operand type while printing assembly binary "
            "instruction in printAssyBinaryInstruction: " +
            std::string(typeid(r).name()));
    }
}

void PrettyPrinters::printAssyCmpInstruction(
    const Assembly::CmpInstruction &cmpInstruction) {
    const auto *type = cmpInstruction.getType();

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error("Unsupported type while printing assembly cmp "
                               "instruction in printAssyCmpInstruction");
    }

    std::cout << "    cmp" << typeSuffix;

    const auto *operand1 = cmpInstruction.getOperand1();
    if (const auto *operand1Imm =
            dynamic_cast<const Assembly::ImmediateOperand *>(operand1)) {
        std::cout << " $" << static_cast<long>(operand1Imm->getImmediate());
    }
    else if (const auto *operand1Reg =
                 dynamic_cast<const Assembly::RegisterOperand *>(operand1)) {
        std::cout << " " << operand1Reg->getRegisterInBytesInStr(registerSize);
    }
    else if (const auto *operand1Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand1)) {
        std::cout << " " << operand1Stack->getOffset() << "("
                  << operand1Stack->getReservedRegisterInStr() << ")";
    }
    else if (const auto *operand1Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)";
    }

    std::cout << ",";

    const auto *operand2 = cmpInstruction.getOperand2();
    if (const auto *operand2Reg =
            dynamic_cast<const Assembly::RegisterOperand *>(operand2)) {
        std::cout << " " << operand2Reg->getRegisterInBytesInStr(registerSize)
                  << "\n";
    }
    else if (const auto *operand2Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand2)) {
        std::cout << " " << operand2Stack->getOffset() << "("
                  << operand2Stack->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *operand2Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
}

void PrettyPrinters::printAssyIdivInstruction(
    const Assembly::IdivInstruction &idivInstruction) {
    const auto *type = idivInstruction.getType();

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error("Unsupported type while printing assembly idiv "
                               "instruction in printAssyIdivInstruction");
    }

    std::cout << "    idiv" << typeSuffix;

    const auto *operand = idivInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        std::cout << " " << regOperand->getRegisterInBytesInStr(registerSize)
                  << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly idiv instruction "
            "in printAssyIdivInstruction: " +
            std::string(typeid(r).name()));
    }
}

void PrettyPrinters::printAssyDivInstruction(
    const Assembly::DivInstruction &divInstruction) {
    const auto *type = divInstruction.getType();

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error("Unsupported type while printing assembly div "
                               "instruction in printAssyDivInstruction");
    }

    std::cout << "    div" << typeSuffix;

    const auto *operand = divInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        std::cout << " " << regOperand->getRegisterInBytesInStr(registerSize)
                  << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly div instruction "
            "in printAssyDivInstruction: " +
            std::string(typeid(r).name()));
    }
}

void PrettyPrinters::printAssyCdqInstruction(
    const Assembly::CdqInstruction &cdqInstruction) {
    const auto *type = cdqInstruction.getType();

    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        std::cout << "    cdq\n";
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        std::cout << "    cqo\n";
    }
    else {
        throw std::logic_error("Unsupported type while printing assembly cdq "
                               "instruction in printAssyCdqInstruction");
    }
}

void PrettyPrinters::printAssyJmpInstruction(
    const Assembly::JmpInstruction &jmpInstruction) {
    auto label = jmpInstruction.getLabel();
    std::cout << "    jmp .L" << label << "\n";
}

void PrettyPrinters::printAssyJmpCCInstruction(
    const Assembly::JmpCCInstruction &jmpCCInstruction) {
    const auto *condCode = jmpCCInstruction.getCondCode();
    if (dynamic_cast<const Assembly::E *>(condCode) != nullptr) {
        std::cout << "    je";
    }
    else if (dynamic_cast<const Assembly::NE *>(condCode) != nullptr) {
        std::cout << "    jne";
    }
    else if (dynamic_cast<const Assembly::G *>(condCode) != nullptr) {
        std::cout << "    jg";
    }
    else if (dynamic_cast<const Assembly::GE *>(condCode) != nullptr) {
        std::cout << "    jge";
    }
    else if (dynamic_cast<const Assembly::L *>(condCode) != nullptr) {
        std::cout << "    jl";
    }
    else if (dynamic_cast<const Assembly::LE *>(condCode) != nullptr) {
        std::cout << "    jle";
    }
    else if (dynamic_cast<const Assembly::A *>(condCode) != nullptr) {
        std::cout << "    ja";
    }
    else if (dynamic_cast<const Assembly::AE *>(condCode) != nullptr) {
        std::cout << "    jae";
    }
    else if (dynamic_cast<const Assembly::B *>(condCode) != nullptr) {
        std::cout << "    jb";
    }
    else if (dynamic_cast<const Assembly::BE *>(condCode) != nullptr) {
        std::cout << "    jbe";
    }
    else {
        throw std::logic_error(
            "Unsupported conditional code while printing "
            "assembly jmpcc instruction in printAssyJmpCCInstruction");
    }

    auto label = jmpCCInstruction.getLabel();
    std::cout << " .L" << label << "\n";
}

void PrettyPrinters::printAssySetCCInstruction(
    const Assembly::SetCCInstruction &setCCInstruction) {
    const auto *condCode = setCCInstruction.getCondCode();
    if (dynamic_cast<const Assembly::E *>(condCode) != nullptr) {
        std::cout << "    sete";
    }
    else if (dynamic_cast<const Assembly::NE *>(condCode) != nullptr) {
        std::cout << "    setne";
    }
    else if (dynamic_cast<const Assembly::G *>(condCode) != nullptr) {
        std::cout << "    setg";
    }
    else if (dynamic_cast<const Assembly::GE *>(condCode) != nullptr) {
        std::cout << "    setge";
    }
    else if (dynamic_cast<const Assembly::L *>(condCode) != nullptr) {
        std::cout << "    setl";
    }
    else if (dynamic_cast<const Assembly::LE *>(condCode) != nullptr) {
        std::cout << "    setle";
    }
    else if (dynamic_cast<const Assembly::A *>(condCode) != nullptr) {
        std::cout << "    seta";
    }
    else if (dynamic_cast<const Assembly::AE *>(condCode) != nullptr) {
        std::cout << "    setae";
    }
    else if (dynamic_cast<const Assembly::B *>(condCode) != nullptr) {
        std::cout << "    setb";
    }
    else if (dynamic_cast<const Assembly::BE *>(condCode) != nullptr) {
        std::cout << "    setbe";
    }
    else {
        throw std::logic_error(
            "Unsupported conditional code while printing "
            "assembly setcc instruction in printAssySetCCInstruction");
    }

    const auto *operand = setCCInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        std::cout << " " << regOperand->getRegisterInBytesInStr(1) << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        std::cout << " " << stackOperand->getOffset() << "("
                  << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        std::cout << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly setcc "
            "instruction in printAssySetCCInstruction: " +
            std::string(typeid(r).name()));
    }
}

void PrettyPrinters::printAssyLabelInstruction(
    const Assembly::LabelInstruction &labelInstruction) {
    auto label = labelInstruction.getLabel();
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
