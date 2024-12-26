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
            generateStatement(irInstruction,
                              functionDefinition->at(0)->getFunctionBody());
        }

        // Return the generated assembly program.
        return std::make_shared<Assembly::Program>(functionDefinition);
    }

  private:
    void generateStatement(
        std::shared_ptr<IR::Instruction> irInstruction,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        if (auto returnInstr = std::dynamic_pointer_cast<IR::ReturnInstruction>(
                irInstruction)) {
            generateReturnInstruction(returnInstr, instructions);
        }
        else if (auto unaryInstr =
                     std::dynamic_pointer_cast<IR::UnaryInstruction>(
                         irInstruction)) {
            generateUnaryInstruction(unaryInstr, instructions);
        }
        else if (auto binaryInstr =
                     std::dynamic_pointer_cast<IR::BinaryInstruction>(
                         irInstruction)) {
            generateBinaryInstruction(binaryInstr, instructions);
        }
    }

    void generateReturnInstruction(
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

    void generateUnaryInstruction(
        std::shared_ptr<IR::UnaryInstruction> unaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Convert the source and destination operands to assembly operands.
        auto srcOperand = convertValue(unaryInstr->getSrc());
        auto dstOperand = convertValue(unaryInstr->getDst());

        // Move the source operand to the destination operand.
        instructions->emplace_back(
            std::make_shared<Assembly::MovInstruction>(srcOperand, dstOperand));

        // Apply the unary operator to the destination operand.
        auto unaryOperator =
            convertUnaryOperator(unaryInstr->getUnaryOperator());
        instructions->emplace_back(std::make_shared<Assembly::UnaryInstruction>(
            unaryOperator, dstOperand));
    }

    void generateBinaryInstruction(
        std::shared_ptr<IR::BinaryInstruction> binaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        auto lhsOperand = convertValue(binaryInstr->getLhs());
        auto rhsOperand = convertValue(binaryInstr->getRhs());
        auto dstOperand = convertValue(binaryInstr->getDst());

        // Move the left-hand side operand to the destination operand.
        instructions->emplace_back(
            std::make_shared<Assembly::MovInstruction>(lhsOperand, dstOperand));

        // Apply the binary operator.
        auto binaryOperator = binaryInstr->getBinaryOperator();
        if (std::dynamic_pointer_cast<IR::AddOperator>(binaryOperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::BinaryInstruction>(
                    std::make_shared<Assembly::AddOperator>(), rhsOperand,
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::SubtractOperator>(
                     binaryOperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::BinaryInstruction>(
                    std::make_shared<Assembly::SubtractOperator>(), rhsOperand,
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::MultiplyOperator>(
                     binaryOperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::BinaryInstruction>(
                    std::make_shared<Assembly::MultiplyOperator>(), rhsOperand,
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::DivideOperator>(
                     binaryOperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    lhsOperand,
                    std::make_shared<Assembly::RegisterOperand>("eax")));
            instructions->emplace_back(
                std::make_shared<Assembly::CdqInstruction>());
            instructions->emplace_back(
                std::make_shared<Assembly::IdivInstruction>(rhsOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::RegisterOperand>("eax"),
                    dstOperand));
        }
        else if (std::dynamic_pointer_cast<IR::RemainderOperator>(
                     binaryOperator)) {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    lhsOperand,
                    std::make_shared<Assembly::RegisterOperand>("eax")));
            instructions->emplace_back(
                std::make_shared<Assembly::CdqInstruction>());
            instructions->emplace_back(
                std::make_shared<Assembly::IdivInstruction>(rhsOperand));
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::make_shared<Assembly::RegisterOperand>("edx"),
                    dstOperand));
        }
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

    std::shared_ptr<Assembly::UnaryOperator>
    convertUnaryOperator(std::shared_ptr<IR::UnaryOperator> irOperator) {
        if (std::dynamic_pointer_cast<IR::NegateOperator>(irOperator)) {
            return std::make_shared<Assembly::NegateOperator>();
        }
        else if (std::dynamic_pointer_cast<IR::ComplementOperator>(
                     irOperator)) {
            return std::make_shared<Assembly::ComplementOperator>();
        }
        // Return a `nullptr` if the operator is not convertible.
        return nullptr;
    }
};
} // namespace Assembly
