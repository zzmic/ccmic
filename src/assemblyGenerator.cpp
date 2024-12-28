#include "assembly.h"
#include "ir.h"

namespace Assembly {
class AssemblyGenerator {
  public:
    std::shared_ptr<Assembly::Program>
    generate(std::shared_ptr<IR::Program> irProgram) {
        // Get the function from the IR program.
        auto irFunction = irProgram->getFunctionDefinition();

        // Create a function definition with the IR function's name to hold the
        // generated instructions.
        // At this point, there is only one function, `main`, in the IR program.
        // Create a shared pointer to a vector of shared pointers of
        // FunctionDefinition.
        auto functionDefinition = std::make_shared<
            std::vector<std::shared_ptr<FunctionDefinition>>>();
        // Create a shared pointer for the specific FunctionDefinition with the
        // function name.
        auto funcDef = std::make_shared<FunctionDefinition>(
            irFunction->at(0)->getFunctionIdentifier());

        // Initialize the function body with an empty vector of instructions.
        auto instructions = std::make_shared<
            std::vector<std::shared_ptr<Assembly::Instruction>>>();
        funcDef->setFunctionBody(instructions);
        // Push the function definition to the vector of function definitions.
        functionDefinition->push_back(funcDef);

        // Get the body of the function, which is a list of IR instructions.
        auto irBody = irFunction->at(0)->getFunctionBody();

        // Generate assembly instructions for the body of the function.
        for (auto irInstruction : *irBody) {
            generateAssyStatement(irInstruction,
                                  functionDefinition->at(0)->getFunctionBody());
        }

        // Return the generated assembly program.
        return std::make_shared<Assembly::Program>(functionDefinition);
    }

  private:
    void generateAssyStatement(
        std::shared_ptr<IR::Instruction> irInstruction,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        if (auto returnInstr = std::dynamic_pointer_cast<IR::ReturnInstruction>(
                irInstruction)) {
            generateAssyReturnInstruction(returnInstr, instructions);
        }
        else if (auto unaryInstr =
                     std::dynamic_pointer_cast<IR::UnaryInstruction>(
                         irInstruction)) {
            generateAssyUnaryInstruction(unaryInstr, instructions);
        }
        else if (auto binaryInstr =
                     std::dynamic_pointer_cast<IR::BinaryInstruction>(
                         irInstruction)) {
            generateAssyBinaryInstruction(binaryInstr, instructions);
        }
        else if (auto copyInstr =
                     std::dynamic_pointer_cast<IR::CopyInstruction>(
                         irInstruction)) {
            generateAssyCopyInstruction(copyInstr, instructions);
        }
        else if (auto jumpInstr =
                     std::dynamic_pointer_cast<IR::JumpInstruction>(
                         irInstruction)) {
            generateAssyJumpInstruction(jumpInstr, instructions);
        }
        else if (auto jumpIfZeroInstr =
                     std::dynamic_pointer_cast<IR::JumpIfZeroInstruction>(
                         irInstruction)) {
            generateAssyJumpIfZeroInstruction(jumpIfZeroInstr, instructions);
        }
        else if (auto jumpIfNotZeroInstr =
                     std::dynamic_pointer_cast<IR::JumpIfNotZeroInstruction>(
                         irInstruction)) {
            generateAssyJumpIfNotZeroInstruction(jumpIfNotZeroInstr,
                                                 instructions);
        }
        else if (auto labelInstr =
                     std::dynamic_pointer_cast<IR::LabelInstruction>(
                         irInstruction)) {
            generateAssyLabelInstruction(labelInstr, instructions);
        }
    }

    void generateAssyReturnInstruction(
        std::shared_ptr<IR::ReturnInstruction> returnInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        auto returnValue = convertValue(returnInstr->getReturnValue());

        // Move the return value into the `eax` register.
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            returnValue, std::make_shared<Assembly::RegisterOperand>("eax")));

        // Generate a `Ret` instruction to return from the function.
        instructions->emplace_back(
            std::make_shared<Assembly::RetInstruction>());
    }

    void generateAssyUnaryInstruction(
        std::shared_ptr<IR::UnaryInstruction> unaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Convert the source and destination operands to assembly operands.
        auto srcOperand = convertValue(unaryInstr->getSrc());
        auto dstOperand = convertValue(unaryInstr->getDst());

        // Get the unary operator from the IR unary instruction.
        auto unaryIROperator = unaryInstr->getUnaryOperator();

        // Generate the assembly instructions based on the unary operator.
        if (auto notOperator =
                std::dynamic_pointer_cast<IR::NotOperator>(unaryIROperator)) {
            // Generate a `Cmp` instruction to compare the source operand with
            // `0`.
            instructions->emplace_back(
                std::make_shared<Assembly::CmpInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    srcOperand));
            // Generate a `Mov` instruction to move `0` to the destination
            // operand.
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    dstOperand));
            // Generate a `SetCC` instruction to set the destination operand to
            // `E`.
            instructions->emplace_back(
                std::make_shared<Assembly::SetCCInstruction>(
                    std::make_shared<Assembly::E>(), dstOperand));
        }
        else {
            // Move the source operand to the destination operand.
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(srcOperand,
                                                           dstOperand));
            // Generate the assembly instructions based on the unary operator.
            if (auto negateOperator =
                    std::dynamic_pointer_cast<IR::NegateOperator>(
                        unaryIROperator)) {
                // Generate a `Neg` instruction to negate the destination
                // operand.
                instructions->emplace_back(
                    std::make_shared<Assembly::UnaryInstruction>(
                        std::make_shared<Assembly::NegateOperator>(),
                        dstOperand));
            }
            else if (auto complementOperator =
                         std::dynamic_pointer_cast<IR::ComplementOperator>(
                             unaryIROperator)) {
                // Generate a `Not` instruction to complement the destination
                // operand.
                instructions->emplace_back(
                    std::make_shared<Assembly::UnaryInstruction>(
                        std::make_shared<Assembly::ComplementOperator>(),
                        dstOperand));
            }
        }
    }

    void generateAssyBinaryInstruction(
        std::shared_ptr<IR::BinaryInstruction> binaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Convert the source operands to assembly operands.
        auto src1Operand = convertValue(binaryInstr->getSrc1());
        auto src2Operand = convertValue(binaryInstr->getSrc2());
        auto dstOperand = convertValue(binaryInstr->getDst());

        // Get the binary operator from the IR binary instruction.
        auto binaryIROperator = binaryInstr->getBinaryOperator();

        // Generate the assembly instructions based on the IR binary operator.
        if (std::dynamic_pointer_cast<IR::AddOperator>(binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(src1Operand,
                                                           dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::BinaryInstruction>(
                    std::make_shared<Assembly::AddOperator>(), src2Operand,
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::SubtractOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(src1Operand,
                                                           dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::BinaryInstruction>(
                    std::make_shared<Assembly::SubtractOperator>(), src2Operand,
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::MultiplyOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(src1Operand,
                                                           dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::BinaryInstruction>(
                    std::make_shared<Assembly::MultiplyOperator>(), src2Operand,
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::DivideOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    src1Operand,
                    std::make_shared<Assembly::RegisterOperand>("eax")));
            instructions->emplace_back(
                std::make_shared<Assembly::CdqInstruction>());
            instructions->emplace_back(
                std::make_shared<Assembly::IdivInstruction>(src2Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::RegisterOperand>("eax"),
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::RemainderOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    src1Operand,
                    std::make_shared<Assembly::RegisterOperand>("eax")));
            instructions->emplace_back(
                std::make_shared<Assembly::CdqInstruction>());
            instructions->emplace_back(
                std::make_shared<Assembly::IdivInstruction>(src2Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::RegisterOperand>("edx"),
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::EqualOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::CmpInstruction>(src2Operand,
                                                           src1Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::SetCCInstruction>(
                    std::make_shared<Assembly::E>(), dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::NotEqualOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::CmpInstruction>(src2Operand,
                                                           src1Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::SetCCInstruction>(
                    std::make_shared<Assembly::NE>(), dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::LessThanOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::CmpInstruction>(src2Operand,
                                                           src1Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::SetCCInstruction>(
                    std::make_shared<Assembly::L>(), dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::LessThanOrEqualOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::CmpInstruction>(src2Operand,
                                                           src1Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::SetCCInstruction>(
                    std::make_shared<Assembly::LE>(), dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::GreaterThanOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::CmpInstruction>(src2Operand,
                                                           src1Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::SetCCInstruction>(
                    std::make_shared<Assembly::G>(), dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::GreaterThanOrEqualOperator>(
                     binaryIROperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::CmpInstruction>(src2Operand,
                                                           src1Operand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::ImmediateOperand>(0),
                    dstOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::SetCCInstruction>(
                    std::make_shared<Assembly::GE>(), dstOperand));
        }
    }

    void generateAssyJumpInstruction(
        std::shared_ptr<IR::JumpInstruction> jumpInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Generate a `Jmp` instruction to jump to the target (label).
        instructions->emplace_back(
            std::make_shared<Assembly::JmpInstruction>(jumpInstr->getTarget()));
    }

    void generateAssyJumpIfZeroInstruction(
        std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Generate a `Cmp` instruction to compare the condition with `0`.
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0),
            convertValue(jumpIfZeroInstr->getCondition())));

        // Generate a `JmpCC` instruction to conditionally jump to the target
        // (label).
        instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
            std::make_shared<Assembly::E>(), jumpIfZeroInstr->getTarget()));
    }

    void generateAssyJumpIfNotZeroInstruction(
        std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Generate a `Cmp` instruction to compare the condition with `0`.
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0),
            convertValue(jumpIfNotZeroInstr->getCondition())));

        // Generate a `JmpCC` instruction to conditionally jump to the target
        // (label).
        instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
            std::make_shared<Assembly::NE>(), jumpIfNotZeroInstr->getTarget()));
    }

    void generateAssyCopyInstruction(
        std::shared_ptr<IR::CopyInstruction> copyInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Convert the source and destination operands to assembly operands.
        auto srcOperand = convertValue(copyInstr->getSrc());
        auto dstOperand = convertValue(copyInstr->getDst());

        // Generate a `Mov` instruction to copy the source operand to the
        // destination operand.
        instructions->emplace_back(
            std::make_shared<Assembly::MovInstruction>(srcOperand, dstOperand));
    }

    void generateAssyLabelInstruction(
        std::shared_ptr<IR::LabelInstruction> labelInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Generate a `Label` instruction with the label name.
        instructions->emplace_back(std::make_shared<Assembly::LabelInstruction>(
            labelInstr->getLabel()));
    }

    std::shared_ptr<Assembly::Operand>
    convertValue(std::shared_ptr<IR::Value> irValue) {
        if (auto constantVal =
                std::dynamic_pointer_cast<IR::ConstantValue>(irValue)) {
            return std::make_shared<Assembly::ImmediateOperand>(
                constantVal->getValue());
        }
        else if (auto varVal =
                     std::dynamic_pointer_cast<IR::VariableValue>(irValue)) {
            return std::make_shared<Assembly::PseudoRegisterOperand>(
                varVal->getIdentifier());
        }
        // Return a `nullptr` if the value is not convertible.
        return nullptr;
    }
};
} // namespace Assembly
