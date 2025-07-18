#include "assemblyGenerator.h"
#include "assembly.h"
#include "backendSymbolTable.h"
#include <algorithm>

namespace Assembly {
AssemblyGenerator::AssemblyGenerator(
    std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
        irStaticVariables,
    std::unordered_map<std::string,
                       std::pair<std::shared_ptr<AST::Type>,
                                 std::shared_ptr<AST::IdentifierAttribute>>>
        frontendSymbolTable)
    : irStaticVariables(irStaticVariables),
      frontendSymbolTable(frontendSymbolTable) {}

std::shared_ptr<Assembly::Program>
AssemblyGenerator::generateAssembly(std::shared_ptr<IR::Program> irProgram) {
    auto irTopLevels = irProgram->getTopLevels();
    auto assyTopLevels =
        std::make_shared<std::vector<std::shared_ptr<TopLevel>>>();

    // Generate assembly instructions for each IR top-level function definition.
    for (auto irTopLevel : *irTopLevels) {
        if (auto irFunctionDefinition =
                std::dynamic_pointer_cast<IR::FunctionDefinition>(irTopLevel)) {
            auto instructions = std::make_shared<
                std::vector<std::shared_ptr<Assembly::Instruction>>>();
            auto assyFunctionDefinition = convertIRFunctionDefinitionToAssy(
                irFunctionDefinition, instructions);
            assyTopLevels->emplace_back(assyFunctionDefinition);
        }
        else {
            throw std::logic_error("Unsupported top-level element");
        }
    }

    // Generate assembly instructions for each IR (either top-level or local)
    // static variable.
    for (auto irStaticVariable : *irStaticVariables) {
        auto assyStaticVariable =
            convertIRStaticVariableToAssy(irStaticVariable);
        assyTopLevels->emplace_back(assyStaticVariable);
    }

    // Convert the frontend symbol table to backend symbol table at the very end
    // of the assembly-generation process.
    Assembly::convertFrontendToBackendSymbolTable(frontendSymbolTable);

    return std::make_shared<Program>(assyTopLevels);
}

std::shared_ptr<Assembly::FunctionDefinition>
AssemblyGenerator::convertIRFunctionDefinitionToAssy(
    std::shared_ptr<IR::FunctionDefinition> irFunctionDefinition,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    auto functionIdentifier = irFunctionDefinition->getFunctionIdentifier();
    auto functionGlobal = irFunctionDefinition->isGlobal();
    auto functionBody = irFunctionDefinition->getFunctionBody();

    // Generate instructions to move parameters from registers to the
    // stack.
    auto irParameters = irFunctionDefinition->getParameterIdentifiers();
    if (irParameters->size() > 0) {
        std::vector<std::string> argRegistersInStr = {"DI", "SI", "DX",
                                                      "CX", "R8", "R9"};
        size_t registerIndex = 0;
        for (size_t i = 0; i < irParameters->size(); ++i) {
            auto irParam = irParameters->at(i);
            auto irParamOperand =
                std::make_shared<Assembly::PseudoRegisterOperand>(irParam);
            if (i < 6) { // First six parameters from registers.
                auto registerOperand =
                    std::make_shared<Assembly::RegisterOperand>(
                        argRegistersInStr[registerIndex]);
                // Determine assembly type based on the parameter type from the
                // symbol table.
                auto assemblyType = determineAssemblyType(
                    std::make_shared<IR::VariableValue>(irParam));
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
                auto assemblyType = determineAssemblyType(
                    std::make_shared<IR::VariableValue>(irParam));
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
    for (auto irInstruction : *functionBody) {
        convertIRInstructionToAssy(irInstruction,
                                   assyFunctionDefinition->getFunctionBody());
    }

    return assyFunctionDefinition;
}

std::shared_ptr<Assembly::StaticVariable>
AssemblyGenerator::convertIRStaticVariableToAssy(
    std::shared_ptr<IR::StaticVariable> irStaticVariable) {
    auto identifier = irStaticVariable->getIdentifier();
    auto global = irStaticVariable->isGlobal();
    auto staticInit = irStaticVariable->getStaticInit();

    // Convert the static initializer to an assembly static variable.
    if (auto constInt = std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
        return std::make_shared<StaticVariable>(identifier, global, 4,
                                                std::move(staticInit));
    }
    else if (auto constLong =
                 std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
        return std::make_shared<StaticVariable>(identifier, global, 8,
                                                std::move(staticInit));
    }
    else {
        throw std::logic_error("Unsupported static initializer type");
    }
}

void AssemblyGenerator::convertIRInstructionToAssy(
    std::shared_ptr<IR::Instruction> irInstruction,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    if (auto returnInstr =
            std::dynamic_pointer_cast<IR::ReturnInstruction>(irInstruction)) {
        convertIRReturnInstructionToAssy(returnInstr, instructions);
    }
    else if (auto unaryInstr = std::dynamic_pointer_cast<IR::UnaryInstruction>(
                 irInstruction)) {
        convertIRUnaryInstructionToAssy(unaryInstr, instructions);
    }
    else if (auto binaryInstr =
                 std::dynamic_pointer_cast<IR::BinaryInstruction>(
                     irInstruction)) {
        convertIRBinaryInstructionToAssy(binaryInstr, instructions);
    }
    else if (auto copyInstr = std::dynamic_pointer_cast<IR::CopyInstruction>(
                 irInstruction)) {
        convertIRCopyInstructionToAssy(copyInstr, instructions);
    }
    else if (auto jumpInstr = std::dynamic_pointer_cast<IR::JumpInstruction>(
                 irInstruction)) {
        convertIRJumpInstructionToAssy(jumpInstr, instructions);
    }
    else if (auto jumpIfZeroInstr =
                 std::dynamic_pointer_cast<IR::JumpIfZeroInstruction>(
                     irInstruction)) {
        convertIRJumpIfZeroInstructionToAssy(jumpIfZeroInstr, instructions);
    }
    else if (auto jumpIfNotZeroInstr =
                 std::dynamic_pointer_cast<IR::JumpIfNotZeroInstruction>(
                     irInstruction)) {
        convertIRJumpIfNotZeroInstructionToAssy(jumpIfNotZeroInstr,
                                                instructions);
    }
    else if (auto labelInstr = std::dynamic_pointer_cast<IR::LabelInstruction>(
                 irInstruction)) {
        convertIRLabelInstructionToAssy(labelInstr, instructions);
    }
    else if (auto functionCallInstr =
                 std::dynamic_pointer_cast<IR::FunctionCallInstruction>(
                     irInstruction)) {
        convertIRFunctionCallInstructionToAssy(functionCallInstr, instructions);
    }
    else if (auto signExtendInstr =
                 std::dynamic_pointer_cast<IR::SignExtendInstruction>(
                     irInstruction)) {
        convertIRSignExtendInstructionToAssy(signExtendInstr, instructions);
    }
    else if (auto truncateInstr =
                 std::dynamic_pointer_cast<IR::TruncateInstruction>(
                     irInstruction)) {
        convertIRTruncateInstructionToAssy(truncateInstr, instructions);
    }
    else {
        throw std::logic_error("Unsupported IR instruction type");
    }
}

void AssemblyGenerator::convertIRReturnInstructionToAssy(
    std::shared_ptr<IR::ReturnInstruction> returnInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    auto returnValue = convertValue(returnInstr->getReturnValue());

    // Determine the assembly type based on the return value.
    auto assemblyType = determineAssemblyType(returnInstr->getReturnValue());

    // Move the return value into the `AX` register.
    auto axReg = std::make_shared<Assembly::AX>();
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        assemblyType, returnValue,
        std::make_shared<Assembly::RegisterOperand>(axReg)));

    // Generate a `Ret` instruction to return from the function.
    instructions->emplace_back(std::make_shared<Assembly::RetInstruction>());
}

void AssemblyGenerator::convertIRUnaryInstructionToAssy(
    std::shared_ptr<IR::UnaryInstruction> unaryInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(unaryInstr->getSrc());
    auto dstOperand = convertValue(unaryInstr->getDst());

    // Determine the assembly type of the source operand and the destination
    // operand based on the source and destination operands.
    auto srcAssemblyType = determineAssemblyType(unaryInstr->getSrc());
    auto dstAssemblyType = determineAssemblyType(unaryInstr->getDst());

    // Get the unary operator from the IR unary instruction.
    auto unaryIROperator = unaryInstr->getUnaryOperator();

    // Generate the assembly instructions based on the unary operator.
    if (auto notOperator =
            std::dynamic_pointer_cast<IR::NotOperator>(unaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            srcAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            std::move(srcOperand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::E>(), std::move(dstOperand)));
    }
    else {
        // Move the source operand to the destination operand.
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            srcAssemblyType, std::move(srcOperand), dstOperand));
        // Generate the assembly instructions based on the unary operator.
        if (auto negateOperator = std::dynamic_pointer_cast<IR::NegateOperator>(
                unaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::UnaryInstruction>(
                    std::make_shared<Assembly::NegateOperator>(),
                    srcAssemblyType, std::move(dstOperand)));
        }
        else if (auto complementOperator =
                     std::dynamic_pointer_cast<IR::ComplementOperator>(
                         unaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::UnaryInstruction>(
                    std::make_shared<Assembly::ComplementOperator>(),
                    srcAssemblyType, std::move(dstOperand)));
        }
    }
}

void AssemblyGenerator::convertIRBinaryInstructionToAssy(
    std::shared_ptr<IR::BinaryInstruction> binaryInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source operands to assembly operands.
    auto src1Operand = convertValue(binaryInstr->getSrc1());
    auto src2Operand = convertValue(binaryInstr->getSrc2());
    auto dstOperand = convertValue(binaryInstr->getDst());

    auto src1AssemblyType = determineAssemblyType(binaryInstr->getSrc1());
    auto dstAssemblyType = determineAssemblyType(binaryInstr->getDst());

    // Get the binary operator from the IR binary instruction.
    auto binaryIROperator = binaryInstr->getBinaryOperator();
    // Generate the assembly instructions based on the IR binary operator.
    if (std::dynamic_pointer_cast<IR::AddOperator>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, std::move(src1Operand), dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::AddOperator>(), src1AssemblyType,
                std::move(src2Operand), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::SubtractOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, std::move(src1Operand), dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::SubtractOperator>(),
                src1AssemblyType, std::move(src2Operand),
                std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::MultiplyOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, std::move(src1Operand), dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::MultiplyOperator>(),
                src1AssemblyType, std::move(src2Operand),
                std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::DivideOperator>(binaryIROperator)) {
        auto axReg = std::make_shared<Assembly::AX>();
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, std::move(src1Operand),
            std::make_shared<Assembly::RegisterOperand>(axReg)));
        instructions->emplace_back(
            std::make_shared<Assembly::CdqInstruction>(src1AssemblyType));
        instructions->emplace_back(std::make_shared<Assembly::IdivInstruction>(
            src1AssemblyType, std::move(src2Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType,
            std::make_shared<Assembly::RegisterOperand>(std::move(axReg)),
            std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::RemainderOperator>(
                 binaryIROperator)) {
        auto axReg = std::make_shared<Assembly::AX>();
        auto dxReg = std::make_shared<Assembly::DX>();
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType, std::move(src1Operand),
            std::make_shared<Assembly::RegisterOperand>(std::move(axReg))));
        instructions->emplace_back(
            std::make_shared<Assembly::CdqInstruction>(src1AssemblyType));
        instructions->emplace_back(std::make_shared<Assembly::IdivInstruction>(
            src1AssemblyType, std::move(src2Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            src1AssemblyType,
            std::make_shared<Assembly::RegisterOperand>(std::move(dxReg)),
            std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::EqualOperator>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::E>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::NotEqualOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::NE>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::LessThanOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::L>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::LessThanOrEqualOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::LE>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::GreaterThanOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::G>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::GreaterThanOrEqualOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            src1AssemblyType, std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            dstAssemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
            dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::GE>(), std::move(dstOperand)));
    }
    else {
        throw std::logic_error("Unsupported IR binary operator type");
    }
}

void AssemblyGenerator::convertIRJumpInstructionToAssy(
    std::shared_ptr<IR::JumpInstruction> jumpInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Jmp` instruction to jump to the target (label).
    instructions->emplace_back(
        std::make_shared<Assembly::JmpInstruction>(jumpInstr->getTarget()));
}

void AssemblyGenerator::convertIRJumpIfZeroInstructionToAssy(
    std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Determine the assembly type based on the condition operand.
    auto assemblyType = determineAssemblyType(jumpIfZeroInstr->getCondition());

    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
        assemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfZeroInstr->getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
        std::make_shared<Assembly::E>(), jumpIfZeroInstr->getTarget()));
}

void AssemblyGenerator::convertIRJumpIfNotZeroInstructionToAssy(
    std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Determine the assembly type based on the condition operand.
    auto assemblyType =
        determineAssemblyType(jumpIfNotZeroInstr->getCondition());

    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
        assemblyType, std::make_shared<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfNotZeroInstr->getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
        std::make_shared<Assembly::NE>(), jumpIfNotZeroInstr->getTarget()));
}

void AssemblyGenerator::convertIRCopyInstructionToAssy(
    std::shared_ptr<IR::CopyInstruction> copyInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(copyInstr->getSrc());
    auto dstOperand = convertValue(copyInstr->getDst());

    // Determine the assembly type based on the source operand.
    auto assemblyType = determineAssemblyType(copyInstr->getSrc());

    // Generate a `Mov` instruction to copy the source operand to the
    // destination operand.
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        assemblyType, std::move(srcOperand), std::move(dstOperand)));
}

void AssemblyGenerator::convertIRLabelInstructionToAssy(
    std::shared_ptr<IR::LabelInstruction> labelInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Label` instruction with the label name.
    instructions->emplace_back(
        std::make_shared<Assembly::LabelInstruction>(labelInstr->getLabel()));
}

void AssemblyGenerator::convertIRFunctionCallInstructionToAssy(
    std::shared_ptr<IR::FunctionCallInstruction> functionCallInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    std::vector<std::string> argRegistersInStr = {"DI", "SI", "DX",
                                                  "CX", "R8", "R9"};
    auto axReg = std::make_shared<Assembly::AX>();

    auto irArgs = functionCallInstr->getArgs();
    auto irRegisterArgs =
        std::make_shared<std::vector<std::shared_ptr<IR::Value>>>();
    auto irStackArgs =
        std::make_shared<std::vector<std::shared_ptr<IR::Value>>>();
    if (irArgs->size() < 6) {
        for (size_t i = 0; i < irArgs->size(); ++i) {
            irRegisterArgs->emplace_back(irArgs->at(i));
        }
    }
    else {
        for (size_t i = 0; i < irArgs->size(); ++i) {
            if (i < 6) {
                irRegisterArgs->emplace_back(irArgs->at(i));
            }
            else {
                irStackArgs->emplace_back(irArgs->at(i));
            }
        }
    }
    // Adjust the stack alignment (if the number of arguments on the stack is
    // odd).
    auto stackPadding = irStackArgs->size() % 2 != 0 ? 8 : 0;
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
    for (auto irRegisterArg : *irRegisterArgs) {
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
    std::reverse(irStackArgs->begin(), irStackArgs->end());
    for (auto irStackArg : *irStackArgs) {
        auto assyStackArg = convertValue(irStackArg);
        auto assemblyType = determineAssemblyType(irStackArg);

        // Check if the operand is a register, immediate, or has type
        // `Quadword`.
        if (std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                assyStackArg) ||
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                assyStackArg) ||
            std::dynamic_pointer_cast<Assembly::Quadword>(assemblyType)) {
            // If it is, push it directly.
            instructions->emplace_back(
                std::make_shared<Assembly::PushInstruction>(
                    std::move(assyStackArg)));
        }
        else {
            // Otherwise, move the operand to the `AX` register (of type
            // `Longword`) first, then push the `AX` register.
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::Longword>(),
                    std::move(assyStackArg),
                    std::make_shared<Assembly::RegisterOperand>(axReg)));
            instructions->emplace_back(
                std::make_shared<Assembly::PushInstruction>(
                    std::make_shared<Assembly::RegisterOperand>(axReg)));
        }
    }

    // Emit a `Call` instruction (to call the function).
    instructions->emplace_back(std::make_shared<Assembly::CallInstruction>(
        functionCallInstr->getFunctionIdentifier()));

    // Adjust the stack pointer (after the function call).
    auto bytesToPop =
        8 * irStackArgs->size() + static_cast<size_t>(stackPadding);
    if (bytesToPop != 0) {
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::AddOperator>(),
                std::make_shared<Assembly::Quadword>(),
                std::make_shared<Assembly::ImmediateOperand>(bytesToPop),
                std::make_shared<Assembly::RegisterOperand>("RSP")));
    }

    // Retrieve the return value.
    auto assyDst = convertValue(functionCallInstr->getDst());
    // Determine the assembly type of the destination operand.
    auto assemblyType = determineAssemblyType(functionCallInstr->getDst());
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        assemblyType,
        std::make_shared<Assembly::RegisterOperand>(std::move(axReg)),
        std::move(assyDst)));
}

std::shared_ptr<Assembly::Operand>
AssemblyGenerator::convertValue(std::shared_ptr<IR::Value> irValue) {
    if (auto constantVal =
            std::dynamic_pointer_cast<IR::ConstantValue>(irValue)) {
        if (auto constInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantVal->getASTConstant())) {
            return std::make_shared<Assembly::ImmediateOperand>(
                constInt->getValue());
        }
        else if (auto constLong = std::dynamic_pointer_cast<AST::ConstantLong>(
                     constantVal->getASTConstant())) {
            return std::make_shared<Assembly::ImmediateOperand>(
                static_cast<int>(constLong->getValue()));
        }
        else {
            throw std::logic_error("Unsupported constant type");
        }
    }
    else if (auto varVal =
                 std::dynamic_pointer_cast<IR::VariableValue>(irValue)) {
        return std::make_shared<Assembly::PseudoRegisterOperand>(
            varVal->getIdentifier());
    }
    else {
        throw std::logic_error("Unsupported IR value type");
    }
}

void AssemblyGenerator::convertIRSignExtendInstructionToAssy(
    std::shared_ptr<IR::SignExtendInstruction> signExtendInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(signExtendInstr->getSrc());
    auto dstOperand = convertValue(signExtendInstr->getDst());

    // Generate a `Movsx` instruction to sign extend from int to long.
    instructions->emplace_back(std::make_shared<Assembly::MovsxInstruction>(
        std::move(srcOperand), std::move(dstOperand)));
}

void AssemblyGenerator::convertIRTruncateInstructionToAssy(
    std::shared_ptr<IR::TruncateInstruction> truncateInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(truncateInstr->getSrc());
    auto dstOperand = convertValue(truncateInstr->getDst());

    // Generate a `Mov` instruction with Longword type to truncate from long to
    // int. This moves the lowest 4 bytes of the source to the destination.
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        std::make_shared<Assembly::Longword>(), std::move(srcOperand),
        std::move(dstOperand)));
}

std::shared_ptr<Assembly::AssemblyType>
AssemblyGenerator::determineAssemblyType(std::shared_ptr<IR::Value> irValue) {
    if (auto constantVal =
            std::dynamic_pointer_cast<IR::ConstantValue>(irValue)) {
        // For constants, determine type based on the AST constant type.
        if (auto constInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantVal->getASTConstant())) {
            return std::make_shared<Assembly::Longword>();
        }
        else if (auto constLong = std::dynamic_pointer_cast<AST::ConstantLong>(
                     constantVal->getASTConstant())) {
            return std::make_shared<Assembly::Quadword>();
        }
        else {
            throw std::logic_error(
                "Unsupported constant type for assembly type determination");
        }
    }
    else if (auto varVal =
                 std::dynamic_pointer_cast<IR::VariableValue>(irValue)) {
        // For variables, look up the type in the (frontend) symbol table.
        auto symbolIt = frontendSymbolTable.find(varVal->getIdentifier());
        if (symbolIt != frontendSymbolTable.end()) {
            auto varType = symbolIt->second.first;
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
AssemblyGenerator::convertASTTypeToAssemblyType(
    std::shared_ptr<AST::Type> astType) {
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
} // namespace Assembly
