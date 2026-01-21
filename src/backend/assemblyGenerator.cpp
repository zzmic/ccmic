#include "assemblyGenerator.h"
#include "../frontend/frontendSymbolTable.h"
#include "assembly.h"
#include <algorithm>
#include <limits>

namespace Assembly {
AssemblyGenerator::AssemblyGenerator(
    const std::vector<std::unique_ptr<IR::StaticVariable>> &irStaticVariables,
    const AST::FrontendSymbolTable &frontendSymbolTable)
    : irStaticVariables(irStaticVariables),
      frontendSymbolTable(frontendSymbolTable) {}

std::shared_ptr<Assembly::Program>
AssemblyGenerator::generateAssembly(const IR::Program &irProgram) {
    auto &irTopLevels = irProgram.getTopLevels();
    auto assyTopLevels =
        std::make_shared<std::vector<std::shared_ptr<TopLevel>>>();

    // Generate assembly instructions for each IR top-level function definition.
    for (const auto &irTopLevel : irTopLevels) {
        if (auto *irFunctionDefinition =
                dynamic_cast<IR::FunctionDefinition *>(irTopLevel.get())) {
            auto instructions = std::make_shared<
                std::vector<std::shared_ptr<Assembly::Instruction>>>();
            auto assyFunctionDefinition = convertIRFunctionDefinitionToAssy(
                *irFunctionDefinition, instructions);
            assyTopLevels->emplace_back(assyFunctionDefinition);
        }
        else {
            throw std::logic_error("Unsupported top-level element");
        }
    }

    // Generate assembly instructions for each IR (either top-level or local)
    // static variable.
    for (const auto &irStaticVariable : irStaticVariables) {
        auto assyStaticVariable =
            convertIRStaticVariableToAssy(*irStaticVariable);
        assyTopLevels->emplace_back(assyStaticVariable);
    }

    return std::make_shared<Program>(assyTopLevels);
}

std::shared_ptr<Assembly::FunctionDefinition>
AssemblyGenerator::convertIRFunctionDefinitionToAssy(
    const IR::FunctionDefinition &irFunctionDefinition,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    auto functionIdentifier = irFunctionDefinition.getFunctionIdentifier();
    auto functionGlobal = irFunctionDefinition.isGlobal();
    auto &functionBody = irFunctionDefinition.getFunctionBody();

    // Generate instructions to move parameters from registers to the
    // stack.
    auto &irParameters = irFunctionDefinition.getParameterIdentifiers();
    if (!irParameters.empty()) {
        std::vector<std::string> argRegistersInStr = {"DI", "SI", "DX",
                                                      "CX", "R8", "R9"};
        size_t registerIndex = 0;
        for (size_t i = 0; i < irParameters.size(); ++i) {
            auto irParam = irParameters.at(i);
            auto irParamOperand =
                std::make_shared<Assembly::PseudoRegisterOperand>(irParam);
            if (i < 6) { // First six parameters from registers.
                auto registerOperand =
                    std::make_shared<Assembly::RegisterOperand>(
                        argRegistersInStr[registerIndex]);
                // Determine assembly type based on the parameter type from the
                // symbol table.
                IR::VariableValue irParamValue(irParam);
                auto assemblyType = determineAssemblyType(&irParamValue);
                // Generate a `Mov` instruction to move the
                // parameter from the register to the stack.
                instructions->emplace_back(
                    std::make_shared<Assembly::MovInstruction>(
                        assemblyType, registerOperand, irParamOperand));
                registerIndex++;
            }
            else { // Remaining parameters from the stack.
                // Calculate the offset from the base pointer.
                // `(%rbp)` stores the base pointer.
                // `(%rbp + 8)` stores the return address.
                // `(%rbp + 16)` stores the first stack parameter (if any).
                // ...
                auto stackOffset = 8 * (i - 6 + 2);
                auto stackOperand = std::make_shared<Assembly::StackOperand>(
                    stackOffset, std::make_shared<Assembly::BP>());
                // Determine assembly type based on the parameter type from the
                // symbol table.
                IR::VariableValue irParamValue(irParam);
                auto assemblyType = determineAssemblyType(&irParamValue);
                // Generate a `Mov` instruction to move the
                // parameter from the stack to the register.
                instructions->emplace_back(
                    std::make_shared<Assembly::MovInstruction>(
                        assemblyType, stackOperand, irParamOperand));
            }
        }
    }

    // Generate function definition that is corresponding to the IR function
    // definition (after conversion).
    auto assyFunctionDefinition = std::make_shared<FunctionDefinition>(
        functionIdentifier, functionGlobal, instructions, 0);

    // Generate assembly instructions for the function body.
    for (const auto &irInstruction : functionBody) {
        convertIRInstructionToAssy(*irInstruction,
                                   assyFunctionDefinition->getFunctionBody());
    }

    return assyFunctionDefinition;
}

std::shared_ptr<Assembly::StaticVariable>
AssemblyGenerator::convertIRStaticVariableToAssy(
    const IR::StaticVariable &irStaticVariable) {
    auto identifier = irStaticVariable.getIdentifier();
    auto global = irStaticVariable.isGlobal();
    auto staticInit = irStaticVariable.getStaticInit();

    // Convert the static initializer to an assembly static variable.
    if (auto constInt = dynamic_cast<const AST::IntInit *>(staticInit)) {
        auto assyInit =
            std::make_shared<AST::IntInit>(std::get<int>(constInt->getValue()));
        return std::make_shared<StaticVariable>(identifier, global, 4,
                                                assyInit);
    }
    else if (auto constLong = dynamic_cast<const AST::LongInit *>(staticInit)) {
        auto assyInit = std::make_shared<AST::LongInit>(
            std::get<long>(constLong->getValue()));
        return std::make_shared<StaticVariable>(identifier, global, 8,
                                                assyInit);
    }
    else {
        throw std::logic_error("Unsupported static initializer type");
    }
}

void AssemblyGenerator::convertIRInstructionToAssy(
    const IR::Instruction &irInstruction,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    if (auto returnInstr =
            dynamic_cast<const IR::ReturnInstruction *>(&irInstruction)) {
        convertIRReturnInstructionToAssy(*returnInstr, instructions);
    }
    else if (auto unaryInstr =
                 dynamic_cast<const IR::UnaryInstruction *>(&irInstruction)) {
        convertIRUnaryInstructionToAssy(*unaryInstr, instructions);
    }
    else if (auto binaryInstr =
                 dynamic_cast<const IR::BinaryInstruction *>(&irInstruction)) {
        convertIRBinaryInstructionToAssy(*binaryInstr, instructions);
    }
    else if (auto copyInstr =
                 dynamic_cast<const IR::CopyInstruction *>(&irInstruction)) {
        convertIRCopyInstructionToAssy(*copyInstr, instructions);
    }
    else if (auto jumpInstr =
                 dynamic_cast<const IR::JumpInstruction *>(&irInstruction)) {
        convertIRJumpInstructionToAssy(*jumpInstr, instructions);
    }
    else if (auto jumpIfZeroInstr =
                 dynamic_cast<const IR::JumpIfZeroInstruction *>(
                     &irInstruction)) {
        convertIRJumpIfZeroInstructionToAssy(*jumpIfZeroInstr, instructions);
    }
    else if (auto jumpIfNotZeroInstr =
                 dynamic_cast<const IR::JumpIfNotZeroInstruction *>(
                     &irInstruction)) {
        convertIRJumpIfNotZeroInstructionToAssy(*jumpIfNotZeroInstr,
                                                instructions);
    }
    else if (auto labelInstr =
                 dynamic_cast<const IR::LabelInstruction *>(&irInstruction)) {
        convertIRLabelInstructionToAssy(*labelInstr, instructions);
    }
    else if (auto functionCallInstr =
                 dynamic_cast<const IR::FunctionCallInstruction *>(
                     &irInstruction)) {
        convertIRFunctionCallInstructionToAssy(*functionCallInstr,
                                               instructions);
    }
    else if (auto signExtendInstr =
                 dynamic_cast<const IR::SignExtendInstruction *>(
                     &irInstruction)) {
        convertIRSignExtendInstructionToAssy(*signExtendInstr, instructions);
    }
    else if (auto truncateInstr = dynamic_cast<const IR::TruncateInstruction *>(
                 &irInstruction)) {
        convertIRTruncateInstructionToAssy(*truncateInstr, instructions);
    }
    else {
        throw std::logic_error("Unsupported IR instruction type");
    }
}

void AssemblyGenerator::convertIRReturnInstructionToAssy(
    const IR::ReturnInstruction &returnInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    auto returnValue = convertValue(returnInstr.getReturnValue());

    // Determine the assembly type based on the return value.
    auto assemblyType = determineAssemblyType(returnInstr.getReturnValue());

    // Move the return value into the `AX` register.
    auto axReg = std::make_shared<Assembly::AX>();
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        assemblyType, returnValue,
        std::make_shared<Assembly::RegisterOperand>(axReg)));

    // Generate a `Ret` instruction to return from the function.
    instructions->emplace_back(std::make_shared<Assembly::RetInstruction>());
}

void AssemblyGenerator::convertIRUnaryInstructionToAssy(
    const IR::UnaryInstruction &unaryInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(unaryInstr.getSrc());
    auto dstOperand = convertValue(unaryInstr.getDst());

    // Determine the assembly type of the source operand and the destination
    // operand based on the source and destination operands.
    auto srcAssemblyType = determineAssemblyType(unaryInstr.getSrc());
    auto dstAssemblyType = determineAssemblyType(unaryInstr.getDst());

    // Get the unary operator from the IR unary instruction.
    auto unaryIROperator = unaryInstr.getUnaryOperator();

    // Generate the assembly instructions based on the unary operator.
    if (dynamic_cast<const IR::NotOperator *>(unaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            srcAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            srcOperand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::E>(), dstOperand));
    }
    else {
        // Move the source operand to the destination operand.
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            srcAssemblyType, srcOperand, dstOperand));
        // Generate the assembly instructions based on the unary operator.
        if (dynamic_cast<const IR::NegateOperator *>(unaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::UnaryInstruction>(
                    std::make_shared<Assembly::NegateOperator>(),
                    srcAssemblyType, dstOperand));
        }
        else if (dynamic_cast<const IR::ComplementOperator *>(
                     unaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::UnaryInstruction>(
                    std::make_shared<Assembly::ComplementOperator>(),
                    srcAssemblyType, dstOperand));
        }
    }
}

void AssemblyGenerator::convertIRBinaryInstructionToAssy(
    const IR::BinaryInstruction &binaryInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source operands to assembly operands.
    auto src1Operand = convertValue(binaryInstr.getSrc1());
    auto src2Operand = convertValue(binaryInstr.getSrc2());
    auto dstOperand = convertValue(binaryInstr.getDst());

    auto src1AssemblyType = determineAssemblyType(binaryInstr.getSrc1());
    auto dstAssemblyType = determineAssemblyType(binaryInstr.getDst());

    // Get the binary operator from the IR binary instruction.
    auto binaryIROperator = binaryInstr.getBinaryOperator();
    // Generate the assembly instructions based on the IR binary operator.
    if (dynamic_cast<const IR::AddOperator *>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, src1Operand, dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::AddOperator>(), src1AssemblyType,
                src2Operand, dstOperand));
    }
    else if (dynamic_cast<const IR::SubtractOperator *>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, src1Operand, dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::SubtractOperator>(),
                src1AssemblyType, src2Operand, dstOperand));
    }
    else if (dynamic_cast<const IR::MultiplyOperator *>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, src1Operand, dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::MultiplyOperator>(),
                src1AssemblyType, src2Operand, dstOperand));
    }
    else if (dynamic_cast<const IR::DivideOperator *>(binaryIROperator)) {
        auto axReg = std::make_shared<Assembly::AX>();
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, src1Operand,
            std::make_shared<Assembly::RegisterOperand>(axReg)));
        instructions->emplace_back(
            std::make_shared<Assembly::CdqInstruction>(src1AssemblyType));
        instructions->emplace_back(std::make_shared<Assembly::IdivInstruction>(
            src1AssemblyType, src2Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType,
            std::make_shared<Assembly::RegisterOperand>(axReg), dstOperand));
    }
    else if (dynamic_cast<const IR::RemainderOperator *>(binaryIROperator)) {
        auto axReg = std::make_shared<Assembly::AX>();
        auto dxReg = std::make_shared<Assembly::DX>();
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, src1Operand,
            std::make_shared<Assembly::RegisterOperand>(axReg)));
        instructions->emplace_back(
            std::make_shared<Assembly::CdqInstruction>(src1AssemblyType));
        instructions->emplace_back(std::make_shared<Assembly::IdivInstruction>(
            src1AssemblyType, src2Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType,
            std::make_shared<Assembly::RegisterOperand>(dxReg), dstOperand));
    }
    else if (dynamic_cast<const IR::EqualOperator *>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, src2Operand, src1Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::E>(), dstOperand));
    }
    else if (dynamic_cast<const IR::NotEqualOperator *>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, src2Operand, src1Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::NE>(), dstOperand));
    }
    else if (dynamic_cast<const IR::LessThanOperator *>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, src2Operand, src1Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::L>(), dstOperand));
    }
    else if (dynamic_cast<const IR::LessThanOrEqualOperator *>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, src2Operand, src1Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::LE>(), dstOperand));
    }
    else if (dynamic_cast<const IR::GreaterThanOperator *>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, src2Operand, src1Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::G>(), dstOperand));
    }
    else if (dynamic_cast<const IR::GreaterThanOrEqualOperator *>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, src2Operand, src1Operand));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::GE>(), dstOperand));
    }
    else {
        throw std::logic_error("Unsupported IR binary operator type");
    }
}

void AssemblyGenerator::convertIRJumpInstructionToAssy(
    const IR::JumpInstruction &jumpInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Jmp` instruction to jump to the target (label).
    instructions->emplace_back(
        std::make_shared<Assembly::JmpInstruction>(jumpInstr.getTarget()));
}

void AssemblyGenerator::convertIRJumpIfZeroInstructionToAssy(
    const IR::JumpIfZeroInstruction &jumpIfZeroInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Determine the assembly type based on the condition operand.
    auto assemblyType = determineAssemblyType(jumpIfZeroInstr.getCondition());

    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
        assemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfZeroInstr.getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
        std::make_shared<Assembly::E>(), jumpIfZeroInstr.getTarget()));
}

void AssemblyGenerator::convertIRJumpIfNotZeroInstructionToAssy(
    const IR::JumpIfNotZeroInstruction &jumpIfNotZeroInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Determine the assembly type based on the condition operand.
    auto assemblyType =
        determineAssemblyType(jumpIfNotZeroInstr.getCondition());

    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
        assemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfNotZeroInstr.getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
        std::make_shared<Assembly::NE>(), jumpIfNotZeroInstr.getTarget()));
}

void AssemblyGenerator::convertIRCopyInstructionToAssy(
    const IR::CopyInstruction &copyInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(copyInstr.getSrc());
    auto dstOperand = convertValue(copyInstr.getDst());

    // Determine the assembly type of the `Mov` instruction.
    auto assemblyType = determineMovType(
        srcOperand, dstOperand, copyInstr.getSrc(), copyInstr.getDst());

    // Generate a `Mov` instruction to copy the source operand to the
    // destination operand.
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        assemblyType, srcOperand, dstOperand));
}

void AssemblyGenerator::convertIRLabelInstructionToAssy(
    const IR::LabelInstruction &labelInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Label` instruction with the label name.
    instructions->emplace_back(
        std::make_shared<Assembly::LabelInstruction>(labelInstr.getLabel()));
}

void AssemblyGenerator::convertIRFunctionCallInstructionToAssy(
    const IR::FunctionCallInstruction &functionCallInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    std::vector<std::string> argRegistersInStr = {"DI", "SI", "DX",
                                                  "CX", "R8", "R9"};
    auto axReg = std::make_shared<Assembly::AX>();

    auto &irArgs = functionCallInstr.getArgs();
    std::vector<const IR::Value *> irRegisterArgs;
    std::vector<const IR::Value *> irStackArgs;
    if (irArgs.size() < 6) {
        for (const auto &arg : irArgs) {
            irRegisterArgs.emplace_back(arg.get());
        }
    }
    else {
        for (size_t i = 0; i < irArgs.size(); ++i) {
            if (i < 6) {
                irRegisterArgs.emplace_back(irArgs.at(i).get());
            }
            else {
                irStackArgs.emplace_back(irArgs.at(i).get());
            }
        }
    }
    // Adjust the stack alignment (if the number of arguments on the stack is
    // odd).
    auto stackPadding = irStackArgs.size() % 2 != 0 ? 8 : 0;
    if (stackPadding != 0) {
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::SubtractOperator>(),
                std::make_shared<Assembly::Quadword>(),
                std::make_shared<Assembly::ImmediateOperand>(stackPadding),
                std::make_shared<Assembly::RegisterOperand>("RSP")));
    }

    // Pass the arguments in registers.
    size_t registerIndex = 0;
    for (auto irRegisterArg : irRegisterArgs) {
        auto registerOperand = std::make_shared<Assembly::RegisterOperand>(
            argRegistersInStr[registerIndex]);
        auto assyRegisterArg = convertValue(irRegisterArg);
        auto assemblyType = determineAssemblyType(irRegisterArg);
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            assemblyType, assyRegisterArg, registerOperand));
        registerIndex++;
    }

    // Pass the arguments on the stack.
    // Reverse the order of the stack arguments since
    // they should be pushed in the reverse order.
    std::reverse(irStackArgs.begin(), irStackArgs.end());
    for (auto irStackArg : irStackArgs) {
        auto assyStackArg = convertValue(irStackArg);
        auto assemblyType = determineAssemblyType(irStackArg);

        // Check if the operand is a register or needs to be moved to a register
        // first.
        if (std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                assyStackArg)) {
            // If it's already a register, push it directly.
            instructions->emplace_back(
                std::make_shared<Assembly::PushInstruction>(assyStackArg));
        }
        else {
            // For immediates, stack operands, or data operands, move to a
            // register first. Use R10 for quadword values, AX for longword
            // values.
            std::shared_ptr<Assembly::Register> tempReg;
            if (std::dynamic_pointer_cast<Assembly::Quadword>(assemblyType)) {
                tempReg = std::make_shared<Assembly::R10>();
            }
            else {
                tempReg = std::make_shared<Assembly::AX>();
            }

            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    assemblyType, assyStackArg,
                    std::make_shared<Assembly::RegisterOperand>(tempReg)));
            instructions->emplace_back(
                std::make_shared<Assembly::PushInstruction>(
                    std::make_shared<Assembly::RegisterOperand>(tempReg)));
        }
    }

    // Emit a `Call` instruction (to call the function).
    instructions->emplace_back(std::make_shared<Assembly::CallInstruction>(
        functionCallInstr.getFunctionIdentifier()));

    // Adjust the stack pointer (after the function call).
    auto bytesToPop =
        8 * irStackArgs.size() + static_cast<size_t>(stackPadding);
    if (bytesToPop != 0) {
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::AddOperator>(),
                std::make_shared<Assembly::Quadword>(),
                std::make_shared<Assembly::ImmediateOperand>(
                    static_cast<long>(bytesToPop)),
                std::make_shared<Assembly::RegisterOperand>("RSP")));
    }

    // Retrieve the return value.
    auto assyDst = convertValue(functionCallInstr.getDst());
    // Determine the assembly type of the destination operand.
    auto assemblyType = determineAssemblyType(functionCallInstr.getDst());
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        assemblyType, std::make_shared<Assembly::RegisterOperand>(axReg),
        assyDst));
}

std::shared_ptr<Assembly::Operand>
AssemblyGenerator::convertValue(const IR::Value *irValue) {
    if (auto constantVal = dynamic_cast<const IR::ConstantValue *>(irValue)) {
        if (auto constInt = dynamic_cast<const AST::ConstantInt *>(
                constantVal->getASTConstant())) {
            return std::make_shared<Assembly::ImmediateOperand>(
                constInt->getValue());
        }
        else if (auto constLong = dynamic_cast<const AST::ConstantLong *>(
                     constantVal->getASTConstant())) {
            // For long constants, we need to handle the full 64-bit value.
            return std::make_shared<Assembly::ImmediateOperand>(
                constLong->getValue());
        }
        else {
            throw std::logic_error("Unsupported constant type");
        }
    }
    else if (auto varVal = dynamic_cast<const IR::VariableValue *>(irValue)) {
        return std::make_shared<Assembly::PseudoRegisterOperand>(
            varVal->getIdentifier());
    }
    else {
        throw std::logic_error("Unsupported IR value type");
    }
}

void AssemblyGenerator::convertIRSignExtendInstructionToAssy(
    const IR::SignExtendInstruction &signExtendInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(signExtendInstr.getSrc());
    auto dstOperand = convertValue(signExtendInstr.getDst());

    // Generate a `Movsx` instruction to sign extend from int to long.
    instructions->emplace_back(
        std::make_shared<Assembly::MovsxInstruction>(srcOperand, dstOperand));
}

void AssemblyGenerator::convertIRTruncateInstructionToAssy(
    const IR::TruncateInstruction &truncateInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(truncateInstr.getSrc());
    auto dstOperand = convertValue(truncateInstr.getDst());

    // Generate a `Mov` instruction with Longword type to truncate from long to
    // int. This moves the lowest 4 bytes of the source to the destination.
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        std::make_shared<Assembly::Longword>(), srcOperand, dstOperand));
}

std::shared_ptr<Assembly::AssemblyType>
AssemblyGenerator::determineAssemblyType(const IR::Value *irValue) {
    if (auto constantVal = dynamic_cast<const IR::ConstantValue *>(irValue)) {
        // For constants, determine type based on the AST constant type.
        if (auto constInt = dynamic_cast<const AST::ConstantInt *>(
                constantVal->getASTConstant())) {
            return std::make_shared<Assembly::Longword>();
        }
        else if (auto constLong = dynamic_cast<const AST::ConstantLong *>(
                     constantVal->getASTConstant())) {
            return std::make_shared<Assembly::Quadword>();
        }
        else {
            throw std::logic_error(
                "Unsupported constant type for assembly type determination");
        }
    }
    else if (auto varVal = dynamic_cast<const IR::VariableValue *>(irValue)) {
        // For variables, look up the type in the (frontend) symbol table.
        auto symbolIt = frontendSymbolTable.find(varVal->getIdentifier());
        if (symbolIt != frontendSymbolTable.end()) {
            auto varType = symbolIt->second.first.get();
            return AssemblyGenerator::convertASTTypeToAssemblyType(varType);
        }
        // If not found in (frontend) symbol table, throw an error.
        throw std::logic_error(
            "Identifier not found in frontend symbol table: " +
            varVal->getIdentifier());
    }
    else {
        throw std::logic_error(
            "Unsupported IR value type for assembly value type conversion");
    }
}

std::shared_ptr<Assembly::AssemblyType>
AssemblyGenerator::convertASTTypeToAssemblyType(const AST::Type *astType) {
    if (!astType) {
        throw std::logic_error("AST type is null");
    }
    if (*astType == AST::IntType()) {
        return std::make_shared<Assembly::Longword>();
    }
    else if (*astType == AST::LongType()) {
        return std::make_shared<Assembly::Quadword>();
    }
    else {
        throw std::logic_error(
            "Unsupported AST type for assembly value type conversion");
    }
}

std::shared_ptr<Assembly::AssemblyType>
AssemblyGenerator::determineMovType(std::shared_ptr<Assembly::Operand> src,
                                    std::shared_ptr<Assembly::Operand> dst,
                                    const IR::Value *irSrc,
                                    const IR::Value *irDst) {
    // Determine the assembly type based on the IR operands.
    auto srcType = determineAssemblyType(irSrc);
    auto dstType = determineAssemblyType(irDst);

    // Use the larger type as base.
    std::shared_ptr<Assembly::AssemblyType> baseType;
    if (std::dynamic_pointer_cast<Assembly::Quadword>(srcType) ||
        std::dynamic_pointer_cast<Assembly::Quadword>(dstType)) {
        baseType = std::make_shared<Assembly::Quadword>();
    }
    else {
        baseType = std::make_shared<Assembly::Longword>();
    }

    // Check if we can optimize to `movl` when using `quadword` type.
    // This is only safe when the destination is a register and the immediate
    // is non-negative, as `movl` zero-extends to 64 bits and does not write
    // the upper 4 bytes of memory destinations.
    if (std::dynamic_pointer_cast<Assembly::Quadword>(baseType)) {
        auto dstReg = std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst);
        if (dstReg) {
            if (auto srcImm =
                    std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                        src)) {
                long value = srcImm->getImmediateLong();
                if (value >= 0 && value <= std::numeric_limits<int>::max()) {
                    return std::make_shared<Assembly::Longword>();
                }
            }
        }
    }

    return baseType;
}
} // namespace Assembly
