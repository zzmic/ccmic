#include "assembly.h"
#include "ir.h"

namespace Assembly {
class AssemblyGenerator {
  public:
    std::shared_ptr<Assembly::Program>
    generate(std::shared_ptr<IR::Program> irProgram) {
        // Get the function from the IR program.
        std::shared_ptr<std::vector<std::shared_ptr<IR::FunctionDefinition>>>
            irFunction = irProgram->getFunctionDefinition();

        // Create a function definition with the IR function's name to hold the
        // generated instructions.
        // At this point, there is only one function, `main`, in the IR program.
        // Create a shared pointer to a vector of shared pointers of
        // FunctionDefinition.
        std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
            functionDefinition = std::make_shared<
                std::vector<std::shared_ptr<FunctionDefinition>>>();
        // Create a shared pointer for the specific FunctionDefinition and add
        // it to the vector.
        functionDefinition->push_back(std::make_shared<FunctionDefinition>(
            irFunction->at(0)->getFunctionIdentifier()));

        // Obtain the function body of the function definition.
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions = functionDefinition->at(0)->getFunctionBody();

        // Initialize the function body with an empty vector of instructions.
        functionDefinition->at(0)->setFunctionBody(
            std::shared_ptr<
                std::vector<std::shared_ptr<Assembly::Instruction>>>(
                new std::vector<std::shared_ptr<Assembly::Instruction>>()));

        // Get the body of the function, which is a list of IR instructions.
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> irBody =
            irFunction->at(0)->getFunctionBody();

        // Generate assembly instructions for the body of the function.
        for (auto irInstruction : *irBody) {
            generateStatement(irInstruction,
                              functionDefinition->at(0)->getFunctionBody());
        }

        // Return the generated assembly program.
        return std::shared_ptr<Assembly::Program>(
            new Assembly::Program(functionDefinition));
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
    }

    void generateReturnInstruction(
        std::shared_ptr<IR::ReturnInstruction> returnInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        auto returnValue = convertValue(returnInstr->getReturnValue());

        // Move the return value into the `eax` register.
        instructions->emplace_back(std::shared_ptr<Assembly::MovInstruction>(
            new Assembly::MovInstruction(
                returnValue, std::shared_ptr<Assembly::Operand>(
                                 new Assembly::RegisterOperand("eax")))));

        // Generate a `Ret` instruction to return from the function.
        instructions->emplace_back(std::shared_ptr<Assembly::RetInstruction>(
            new Assembly::RetInstruction()));
    }

    void generateUnaryInstruction(
        std::shared_ptr<IR::UnaryInstruction> unaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions) {
        // Convert the source and destination operands to assembly operands.
        auto srcOperand = convertValue(unaryInstr->getSrc());
        // The destination operand, for now, is always the `eax` register.
        auto dstOperand = std::make_shared<Assembly::RegisterOperand>("eax");

        if (std::dynamic_pointer_cast<Assembly::ImmediateOperand>(srcOperand)) {
            // Move the source operand to the destination operand if it is an
            // immediate operand.
            instructions->emplace_back(
                std::shared_ptr<Assembly::MovInstruction>(
                    new Assembly::MovInstruction(srcOperand, dstOperand)));
        }

        // Apply the unary operator to the destination operand.
        auto unaryOperator =
            convertUnaryOperator(unaryInstr->getUnaryOperator());
        instructions->emplace_back(std::shared_ptr<Assembly::UnaryInstruction>(
            new Assembly::UnaryInstruction(unaryOperator, dstOperand)));
    }

    std::shared_ptr<Assembly::Operand>
    convertValue(std::shared_ptr<IR::Value> irValue) {
        if (auto integerVal =
                std::dynamic_pointer_cast<IR::IntegerValue>(irValue)) {
            return std::make_shared<Assembly::ImmediateOperand>(
                integerVal->getValue());
        }
        else if (auto varVal =
                     std::dynamic_pointer_cast<IR::VariableValue>(irValue)) {
            return std::make_shared<Assembly::PseudoRegisterOperand>(
                varVal->getIdentifier());
        }
        // Return a nullptr if the value is not convertible.
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
        // Return a nullptr if the operator is not convertible.
        return nullptr;
    }
};
} // namespace Assembly
