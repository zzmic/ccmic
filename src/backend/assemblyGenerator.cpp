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

std::unique_ptr<Assembly::Program>
AssemblyGenerator::generateAssembly(const IR::Program &irProgram) {
    auto &irTopLevels = irProgram.getTopLevels();
    auto assyTopLevels =
        std::make_unique<std::vector<std::unique_ptr<TopLevel>>>();

    // Generate assembly instructions for each IR top-level function definition.
    for (const auto &irTopLevel : irTopLevels) {
        if (auto *irFunctionDefinition =
                dynamic_cast<IR::FunctionDefinition *>(irTopLevel.get())) {
            auto instructions = std::make_unique<
                std::vector<std::unique_ptr<Assembly::Instruction>>>();
            auto assyFunctionDefinition = convertIRFunctionDefinitionToAssy(
                *irFunctionDefinition, std::move(instructions));
            assyTopLevels->emplace_back(std::move(assyFunctionDefinition));
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
        assyTopLevels->emplace_back(std::move(assyStaticVariable));
    }

    return std::make_unique<Program>(std::move(assyTopLevels));
}

std::unique_ptr<Assembly::FunctionDefinition>
AssemblyGenerator::convertIRFunctionDefinitionToAssy(
    const IR::FunctionDefinition &irFunctionDefinition,
    std::unique_ptr<std::vector<std::unique_ptr<Assembly::Instruction>>>
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
                std::make_unique<Assembly::PseudoRegisterOperand>(irParam);
            if (i < 6) { // First six parameters from registers.
                auto registerOperand =
                    std::make_unique<Assembly::RegisterOperand>(
                        argRegistersInStr[registerIndex]);
                // Determine assembly type based on the parameter type from the
                // symbol table.
                IR::VariableValue irParamValue(irParam);
                auto assemblyType = determineAssemblyType(&irParamValue);
                // Generate a `Mov` instruction to move the
                // parameter from the register to the stack.
                instructions->emplace_back(
                    std::make_unique<Assembly::MovInstruction>(
                        std::move(assemblyType), std::move(registerOperand),
                        std::move(irParamOperand)));
                registerIndex++;
            }
            else { // Remaining parameters from the stack.
                // Calculate the offset from the base pointer.
                // `(%rbp)` stores the base pointer.
                // `(%rbp + 8)` stores the return address.
                // `(%rbp + 16)` stores the first stack parameter (if any).
                // ...
                auto stackOffset = 8 * (i - 6 + 2);
                auto stackOperand = std::make_unique<Assembly::StackOperand>(
                    stackOffset, std::make_unique<Assembly::BP>());
                // Determine assembly type based on the parameter type from the
                // symbol table.
                IR::VariableValue irParamValue(irParam);
                auto assemblyType = determineAssemblyType(&irParamValue);
                // Generate a `Mov` instruction to move the
                // parameter from the stack to the register.
                instructions->emplace_back(
                    std::make_unique<Assembly::MovInstruction>(
                        std::move(assemblyType), std::move(stackOperand),
                        std::move(irParamOperand)));
            }
        }
    }

    // Generate function definition that is corresponding to the IR function
    // definition (after conversion).
    auto assyFunctionDefinition = std::make_unique<FunctionDefinition>(
        functionIdentifier, functionGlobal, std::move(instructions), 0);

    // Generate assembly instructions for the function body.
    for (const auto &irInstruction : functionBody) {
        convertIRInstructionToAssy(*irInstruction,
                                   assyFunctionDefinition->getFunctionBody());
    }

    return assyFunctionDefinition;
}

std::unique_ptr<Assembly::StaticVariable>
AssemblyGenerator::convertIRStaticVariableToAssy(
    const IR::StaticVariable &irStaticVariable) {
    auto identifier = irStaticVariable.getIdentifier();
    auto global = irStaticVariable.isGlobal();
    auto staticInit = irStaticVariable.getStaticInit();

    // Convert the static initializer to an assembly static variable.
    if (auto constInt = dynamic_cast<const AST::IntInit *>(staticInit)) {
        auto assyInit =
            std::make_unique<AST::IntInit>(std::get<int>(constInt->getValue()));
        return std::make_unique<StaticVariable>(identifier, global, 4,
                                                std::move(assyInit));
    }
    else if (auto constLong = dynamic_cast<const AST::LongInit *>(staticInit)) {
        auto assyInit = std::make_unique<AST::LongInit>(
            std::get<long>(constLong->getValue()));
        return std::make_unique<StaticVariable>(identifier, global, 8,
                                                std::move(assyInit));
    }
    else {
        throw std::logic_error("Unsupported static initializer type");
    }
}

void AssemblyGenerator::convertIRInstructionToAssy(
    const IR::Instruction &irInstruction,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
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
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    auto returnValue = convertValue(returnInstr.getReturnValue());

    // Determine the assembly type based on the return value.
    auto assemblyType = determineAssemblyType(returnInstr.getReturnValue());

    // Move the return value into the `AX` register.
    instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
        std::move(assemblyType), std::move(returnValue),
        std::make_unique<Assembly::RegisterOperand>("AX")));

    // Generate a `Ret` instruction to return from the function.
    instructions.emplace_back(std::make_unique<Assembly::RetInstruction>());
}

void AssemblyGenerator::convertIRUnaryInstructionToAssy(
    const IR::UnaryInstruction &unaryInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {

    // Get the unary operator from the IR unary instruction.
    auto unaryIROperator = unaryInstr.getUnaryOperator();

    // Generate the assembly instructions based on the unary operator.
    if (dynamic_cast<const IR::NotOperator *>(unaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
            determineAssemblyType(unaryInstr.getSrc()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(unaryInstr.getSrc())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(unaryInstr.getDst()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(unaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::SetCCInstruction>(
            std::make_unique<Assembly::E>(),
            convertValue(unaryInstr.getDst())));
    }
    else {
        // Move the source operand to the destination operand.
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(unaryInstr.getSrc()),
            convertValue(unaryInstr.getSrc()),
            convertValue(unaryInstr.getDst())));
        // Generate the assembly instructions based on the unary operator.
        if (dynamic_cast<const IR::NegateOperator *>(unaryIROperator)) {
            instructions.emplace_back(
                std::make_unique<Assembly::UnaryInstruction>(
                    std::make_unique<Assembly::NegateOperator>(),
                    determineAssemblyType(unaryInstr.getSrc()),
                    convertValue(unaryInstr.getDst())));
        }
        else if (dynamic_cast<const IR::ComplementOperator *>(
                     unaryIROperator)) {
            instructions.emplace_back(
                std::make_unique<Assembly::UnaryInstruction>(
                    std::make_unique<Assembly::ComplementOperator>(),
                    determineAssemblyType(unaryInstr.getSrc()),
                    convertValue(unaryInstr.getDst())));
        }
    }
}

void AssemblyGenerator::convertIRBinaryInstructionToAssy(
    const IR::BinaryInstruction &binaryInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    auto binaryIROperator = binaryInstr.getBinaryOperator();
    if (dynamic_cast<const IR::AddOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::BinaryInstruction>(
            std::make_unique<Assembly::AddOperator>(),
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::SubtractOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::BinaryInstruction>(
            std::make_unique<Assembly::SubtractOperator>(),
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::MultiplyOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::BinaryInstruction>(
            std::make_unique<Assembly::MultiplyOperator>(),
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::DivideOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc1()),
            std::make_unique<Assembly::RegisterOperand>("AX")));
        instructions.emplace_back(std::make_unique<Assembly::CdqInstruction>(
            determineAssemblyType(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::IdivInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            std::make_unique<Assembly::RegisterOperand>("AX"),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::RemainderOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc1()),
            std::make_unique<Assembly::RegisterOperand>("AX")));
        instructions.emplace_back(std::make_unique<Assembly::CdqInstruction>(
            determineAssemblyType(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::IdivInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            std::make_unique<Assembly::RegisterOperand>("DX"),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::EqualOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getDst()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::SetCCInstruction>(
            std::make_unique<Assembly::E>(),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::NotEqualOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getDst()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::SetCCInstruction>(
            std::make_unique<Assembly::NE>(),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::LessThanOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getDst()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::SetCCInstruction>(
            std::make_unique<Assembly::L>(),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::LessThanOrEqualOperator *>(
                 binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getDst()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::SetCCInstruction>(
            std::make_unique<Assembly::LE>(),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::GreaterThanOperator *>(binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getDst()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::SetCCInstruction>(
            std::make_unique<Assembly::G>(),
            convertValue(binaryInstr.getDst())));
    }
    else if (dynamic_cast<const IR::GreaterThanOrEqualOperator *>(
                 binaryIROperator)) {
        instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
            determineAssemblyType(binaryInstr.getSrc1()),
            convertValue(binaryInstr.getSrc2()),
            convertValue(binaryInstr.getSrc1())));
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            determineAssemblyType(binaryInstr.getDst()),
            std::make_unique<Assembly::ImmediateOperand>(0),
            convertValue(binaryInstr.getDst())));
        instructions.emplace_back(std::make_unique<Assembly::SetCCInstruction>(
            std::make_unique<Assembly::GE>(),
            convertValue(binaryInstr.getDst())));
    }
    else {
        throw std::logic_error("Unsupported IR binary operator type");
    }
}

void AssemblyGenerator::convertIRJumpInstructionToAssy(
    const IR::JumpInstruction &jumpInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    // Generate a `Jmp` instruction to jump to the target (label).
    instructions.emplace_back(
        std::make_unique<Assembly::JmpInstruction>(jumpInstr.getTarget()));
}

void AssemblyGenerator::convertIRJumpIfZeroInstructionToAssy(
    const IR::JumpIfZeroInstruction &jumpIfZeroInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
        determineAssemblyType(jumpIfZeroInstr.getCondition()),
        std::make_unique<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfZeroInstr.getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions.emplace_back(std::make_unique<Assembly::JmpCCInstruction>(
        std::make_unique<Assembly::E>(), jumpIfZeroInstr.getTarget()));
}

void AssemblyGenerator::convertIRJumpIfNotZeroInstructionToAssy(
    const IR::JumpIfNotZeroInstruction &jumpIfNotZeroInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions.emplace_back(std::make_unique<Assembly::CmpInstruction>(
        determineAssemblyType(jumpIfNotZeroInstr.getCondition()),
        std::make_unique<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfNotZeroInstr.getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions.emplace_back(std::make_unique<Assembly::JmpCCInstruction>(
        std::make_unique<Assembly::NE>(), jumpIfNotZeroInstr.getTarget()));
}

void AssemblyGenerator::convertIRCopyInstructionToAssy(
    const IR::CopyInstruction &copyInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(copyInstr.getSrc());
    auto dstOperand = convertValue(copyInstr.getDst());

    // Determine the assembly type of the `Mov` instruction.
    auto assemblyType =
        determineMovType(srcOperand.get(), dstOperand.get(), copyInstr.getSrc(),
                         copyInstr.getDst());

    // Generate a `Mov` instruction to copy the source operand to the
    // destination operand.
    instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
        std::move(assemblyType), std::move(srcOperand), std::move(dstOperand)));
}

void AssemblyGenerator::convertIRLabelInstructionToAssy(
    const IR::LabelInstruction &labelInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    // Generate a `Label` instruction with the label name.
    instructions.emplace_back(
        std::make_unique<Assembly::LabelInstruction>(labelInstr.getLabel()));
}

void AssemblyGenerator::convertIRFunctionCallInstructionToAssy(
    const IR::FunctionCallInstruction &functionCallInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    std::vector<std::string> argRegistersInStr = {"DI", "SI", "DX",
                                                  "CX", "R8", "R9"};

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
        instructions.emplace_back(std::make_unique<Assembly::BinaryInstruction>(
            std::make_unique<Assembly::SubtractOperator>(),
            std::make_unique<Assembly::Quadword>(),
            std::make_unique<Assembly::ImmediateOperand>(stackPadding),
            std::make_unique<Assembly::RegisterOperand>("RSP")));
    }

    // Pass the arguments in registers.
    size_t registerIndex = 0;
    for (auto irRegisterArg : irRegisterArgs) {
        auto registerOperand = std::make_unique<Assembly::RegisterOperand>(
            argRegistersInStr[registerIndex]);
        auto assyRegisterArg = convertValue(irRegisterArg);
        auto assemblyType = determineAssemblyType(irRegisterArg);
        instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
            std::move(assemblyType), std::move(assyRegisterArg),
            std::move(registerOperand)));
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
        if (dynamic_cast<Assembly::RegisterOperand *>(assyStackArg.get())) {
            // If it's already a register, push it directly.
            instructions.emplace_back(
                std::make_unique<Assembly::PushInstruction>(
                    std::move(assyStackArg)));
        }
        else {
            // For immediates, stack operands, or data operands, move to a
            // register first. Use R10 for quadword values, AX for longword
            // values.
            bool isQuadword =
                dynamic_cast<Assembly::Quadword *>(assemblyType.get());
            auto tempRegisterName = isQuadword ? "R10" : "AX";
            instructions.emplace_back(
                std::make_unique<Assembly::MovInstruction>(
                    std::move(assemblyType), std::move(assyStackArg),
                    std::make_unique<Assembly::RegisterOperand>(
                        tempRegisterName)));
            instructions.emplace_back(
                std::make_unique<Assembly::PushInstruction>(
                    std::make_unique<Assembly::RegisterOperand>(
                        tempRegisterName)));
        }
    }

    // Emit a `Call` instruction (to call the function).
    instructions.emplace_back(std::make_unique<Assembly::CallInstruction>(
        functionCallInstr.getFunctionIdentifier()));

    // Adjust the stack pointer (after the function call).
    auto bytesToPop =
        8 * irStackArgs.size() + static_cast<size_t>(stackPadding);
    if (bytesToPop != 0) {
        instructions.emplace_back(std::make_unique<Assembly::BinaryInstruction>(
            std::make_unique<Assembly::AddOperator>(),
            std::make_unique<Assembly::Quadword>(),
            std::make_unique<Assembly::ImmediateOperand>(
                static_cast<long>(bytesToPop)),
            std::make_unique<Assembly::RegisterOperand>("RSP")));
    }

    // Retrieve the return value.
    auto assyDst = convertValue(functionCallInstr.getDst());
    // Determine the assembly type of the destination operand.
    auto assemblyType = determineAssemblyType(functionCallInstr.getDst());
    instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
        std::move(assemblyType),
        std::make_unique<Assembly::RegisterOperand>("AX"), std::move(assyDst)));
}

std::unique_ptr<Assembly::Operand>
AssemblyGenerator::convertValue(const IR::Value *irValue) {
    if (auto constantVal = dynamic_cast<const IR::ConstantValue *>(irValue)) {
        if (auto constInt = dynamic_cast<const AST::ConstantInt *>(
                constantVal->getASTConstant())) {
            return std::make_unique<Assembly::ImmediateOperand>(
                constInt->getValue());
        }
        else if (auto constLong = dynamic_cast<const AST::ConstantLong *>(
                     constantVal->getASTConstant())) {
            // For long constants, we need to handle the full 64-bit value.
            return std::make_unique<Assembly::ImmediateOperand>(
                constLong->getValue());
        }
        else {
            throw std::logic_error("Unsupported constant type");
        }
    }
    else if (auto varVal = dynamic_cast<const IR::VariableValue *>(irValue)) {
        return std::make_unique<Assembly::PseudoRegisterOperand>(
            varVal->getIdentifier());
    }
    else {
        throw std::logic_error("Unsupported IR value type");
    }
}

void AssemblyGenerator::convertIRSignExtendInstructionToAssy(
    const IR::SignExtendInstruction &signExtendInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(signExtendInstr.getSrc());
    auto dstOperand = convertValue(signExtendInstr.getDst());

    // Generate a `Movsx` instruction to sign extend from int to long.
    instructions.emplace_back(std::make_unique<Assembly::MovsxInstruction>(
        std::move(srcOperand), std::move(dstOperand)));
}

void AssemblyGenerator::convertIRTruncateInstructionToAssy(
    const IR::TruncateInstruction &truncateInstr,
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(truncateInstr.getSrc());
    auto dstOperand = convertValue(truncateInstr.getDst());

    // Generate a `Mov` instruction with Longword type to truncate from long to
    // int. This moves the lowest 4 bytes of the source to the destination.
    instructions.emplace_back(std::make_unique<Assembly::MovInstruction>(
        std::make_unique<Assembly::Longword>(), std::move(srcOperand),
        std::move(dstOperand)));
}

std::unique_ptr<Assembly::AssemblyType>
AssemblyGenerator::determineAssemblyType(const IR::Value *irValue) {
    if (auto constantVal = dynamic_cast<const IR::ConstantValue *>(irValue)) {
        // For constants, determine type based on the AST constant type.
        if (dynamic_cast<const AST::ConstantInt *>(
                constantVal->getASTConstant())) {
            return std::make_unique<Assembly::Longword>();
        }
        else if (dynamic_cast<const AST::ConstantLong *>(
                     constantVal->getASTConstant())) {
            return std::make_unique<Assembly::Quadword>();
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

std::unique_ptr<Assembly::AssemblyType>
AssemblyGenerator::convertASTTypeToAssemblyType(const AST::Type *astType) {
    if (!astType) {
        throw std::logic_error("AST type is null");
    }
    if (*astType == AST::IntType()) {
        return std::make_unique<Assembly::Longword>();
    }
    else if (*astType == AST::LongType()) {
        return std::make_unique<Assembly::Quadword>();
    }
    else {
        throw std::logic_error(
            "Unsupported AST type for assembly value type conversion");
    }
}

std::unique_ptr<Assembly::AssemblyType> AssemblyGenerator::determineMovType(
    const Assembly::Operand *src, const Assembly::Operand *dst,
    const IR::Value *irSrc, const IR::Value *irDst) {
    // Determine the assembly type based on the IR operands.
    auto srcType = determineAssemblyType(irSrc);
    auto dstType = determineAssemblyType(irDst);

    // Use the larger type as base.
    std::unique_ptr<Assembly::AssemblyType> baseType;
    if (dynamic_cast<Assembly::Quadword *>(srcType.get()) ||
        dynamic_cast<Assembly::Quadword *>(dstType.get())) {
        baseType = std::make_unique<Assembly::Quadword>();
    }
    else {
        baseType = std::make_unique<Assembly::Longword>();
    }

    // Check if we can optimize to `movl` when using `quadword` type.
    // This is only safe when the destination is a register and the immediate
    // is non-negative, as `movl` zero-extends to 64 bits and does not write
    // the upper 4 bytes of memory destinations.
    if (dynamic_cast<Assembly::Quadword *>(baseType.get())) {
        if (dynamic_cast<const Assembly::RegisterOperand *>(dst)) {
            if (auto srcImm =
                    dynamic_cast<const Assembly::ImmediateOperand *>(src)) {
                long value = srcImm->getImmediateLong();
                if (value >= 0 && value <= std::numeric_limits<int>::max()) {
                    return std::make_unique<Assembly::Longword>();
                }
            }
        }
    }

    return baseType;
}
} // namespace Assembly
