#include "expression.h"
#include "function.h"
#include "ir.h"
#include "program.h"
#include "statement.h"

class IRGenerator {
  public:
    std::shared_ptr<IR::Program>
    generate(std::shared_ptr<AST::Program> astProgram) {
        // Get the function from the AST program.
        std::shared_ptr<AST::Function> astFunction = astProgram->getFunction();

        // Create a shared pointer to a vector of shared pointers of
        // FunctionDefinition.
        std::shared_ptr<std::vector<std::shared_ptr<IR::FunctionDefinition>>>
            functionDefinition = std::make_shared<
                std::vector<std::shared_ptr<IR::FunctionDefinition>>>();

        // Create a shared pointer for the specific FunctionDefinition and add
        // it to the vector.
        functionDefinition->emplace_back(
            std::make_shared<IR::FunctionDefinition>(astFunction->getName()));

        // Initialize the function body with an empty vector of instructions.
        functionDefinition->at(0)->setFunctionBody(
            std::make_shared<std::vector<std::shared_ptr<IR::Instruction>>>(
                std::vector<std::shared_ptr<IR::Instruction>>()));

        // Get the body of the function (single statement for now).
        std::shared_ptr<AST::Statement> astBody = astFunction->getBody();
        // Generate IR instructions for the body of the function (the
        // single statement aforementioned).
        generateStatement(astBody,
                          functionDefinition->at(0)->getFunctionBody());

        // Return the generated IR program.
        return std::make_shared<IR::Program>(functionDefinition);
    }

  private:
    void generateStatement(
        std::shared_ptr<AST::Statement> astStatement,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        if (auto returnStmt =
                std::dynamic_pointer_cast<AST::ReturnStatement>(astStatement)) {
            // If the statement is a return statement, generate a return
            // statement.
            generateReturnStatement(returnStmt, instructions);
        }
    }

    void generateReturnStatement(
        std::shared_ptr<AST::ReturnStatement> returnStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // Get the expression from the return statement.
        std::shared_ptr<AST::Expression> exp = returnStmt->getExpression();

        // Process the expression and generate the corresponding IR
        // instructions.
        std::shared_ptr<IR::Value> result =
            generateInstruction(exp, instructions);

        // Generate a return instruction with the result.
        instructions->emplace_back(
            std::make_shared<IR::ReturnInstruction>(result));
    }

    std::shared_ptr<IR::Value> generateInstruction(
        std::shared_ptr<AST::Expression> e,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // If the expression is a constant expression, generate a constant
        // value.
        if (auto constantExpr =
                std::dynamic_pointer_cast<AST::ConstantExpression>(e)) {
            return std::make_shared<IR::ConstantValue>(
                constantExpr->getValue());
        }

        // If the expression is a unary expression, ...
        else if (auto unaryExpr =
                     std::dynamic_pointer_cast<AST::UnaryExpression>(e)) {
            return generateUnaryInstruction(unaryExpr, instructions);
        }

        // If the expression is a binary expression, ...
        else if (auto binaryExpr =
                     std::dynamic_pointer_cast<AST::BinaryExpression>(e)) {
            // If the binary operator in the AST binary expression is a
            // logical-and operator, ...
            if (auto astBinaryOperator =
                    std::dynamic_pointer_cast<AST::AndOperator>(
                        binaryExpr->getOperator())) {
                // TODO(zzmic): Implement the logical-and operator case.
            }
            // If the binary operator in the AST binary expression is a
            // logical-or operator, ...
            else if (auto astBinaryOperator =
                         std::dynamic_pointer_cast<AST::OrOperator>(
                             binaryExpr->getOperator())) {
                // TODO(zzmic): Implement the logical-or operator case.
            }
            // Otherwise (i.e., if the binary operator in the AST binary
            // expression can be converted to a binary operator in the IR), ...
            else {
                return generateBinaryInstruction(binaryExpr, instructions);
            }
        }

        else {
            throw std::runtime_error("Unsupported expression type");
        }
    }

    std::shared_ptr<IR::VariableValue> generateUnaryInstruction(
        std::shared_ptr<AST::UnaryExpression> unaryExpr,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // Recursively generate the expression in the unary expression.
        std::shared_ptr<IR::Value> src =
            generateInstruction(unaryExpr->getExpression(), instructions);

        // Create a temporary variable (in string) to store the result of
        // the unary operation.
        std::string tmpName = generateTemporary();

        // Create a variable value for the temporary variable.
        std::shared_ptr<IR::VariableValue> dst =
            std::make_shared<IR::VariableValue>(tmpName);

        // Convert the unary operator in the unary expression to a IR
        // unary operator.
        std::shared_ptr<IR::UnaryOperator> IROp =
            convertUnop(unaryExpr->getOperator());

        // Generate a unary instruction with the IR unary operator, the
        // source value, and the destination value.
        instructions->emplace_back(
            std::make_shared<IR::UnaryInstruction>(IROp, src, dst));

        // Return the destination value.
        return dst;
    }

    std::shared_ptr<IR::VariableValue> generateBinaryInstruction(
        std::shared_ptr<AST::BinaryExpression> binaryExpr,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // Convert the binary operator in the binary expression to a IR
        // binary operator.
        std::shared_ptr<IR::BinaryOperator> IROp =
            convertBinop(binaryExpr->getOperator());

        // Create a temporary variable (in string) to store the result of
        // the binary operation.
        std::string tmpName = generateTemporary();
        // Create a variable value for the temporary variable.
        std::shared_ptr<IR::VariableValue> dst =
            std::make_shared<IR::VariableValue>(tmpName);

        // Recursively generate the left and right expressions in the binary
        // expression.
        std::shared_ptr<IR::Value> lhs =
            generateInstruction(binaryExpr->getLeft(), instructions);
        std::shared_ptr<IR::Value> rhs =
            generateInstruction(binaryExpr->getRight(), instructions);
        // Generate a binary instruction with the IR binary operator, the
        // left-hand side value, the right-hand side value, and the
        // destination value.
        instructions->emplace_back(
            std::make_shared<IR::BinaryInstruction>(IROp, lhs, rhs, dst));

        // Return the destination value.
        return dst;
    }

    void generateJumpInstruction(
        std::string target,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // Generate a jump instruction with the target label.
        instructions->emplace_back(
            std::make_shared<IR::JumpInstruction>(target));
    }

    void generateJumpIfZeroInstruction(
        std::shared_ptr<IR::Value> condition, std::string target,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // Generate a jump if zero instruction with the condition value and
        // the target label.
        instructions->emplace_back(
            std::make_shared<IR::JumpIfZeroInstruction>(condition, target));
    }

    void generateJumpIfNotZeroInstruction(
        std::shared_ptr<IR::Value> condition, std::string target,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // Generate a jump if not zero instruction with the condition value
        // and the target label.
        instructions->emplace_back(
            std::make_shared<IR::JumpIfNotZeroInstruction>(condition, target));
    }

    void generateLabelInstruction(
        std::string identifier,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions) {
        // Generate a label instruction with the label identifier.
        instructions->emplace_back(
            std::make_shared<IR::LabelInstruction>(identifier));
    }

    std::string generateTemporary() {
        // Create a temporary variable name with a unique number.
        // The number would be incremented each time this function is called.
        static int counter = 0;

        // Return the string representation of the temporary variable name.
        return "tmp." + std::to_string(counter++);
    }

    std::string generateLabel() {
        // Create a label with a unique number.
        // The number would be incremented each time this function is called.
        static int counter = 0;

        // Return the string representation of the label.
        // Note: An alternative is to use the string "and_falseN" as
        // "false_label" in the listing, "where N is the current value of a
        // global counter."
        return "L" + std::to_string(counter++);
    }

    std::shared_ptr<IR::UnaryOperator>
    convertUnop(std::shared_ptr<AST::UnaryOperator> op) {
        if (std::dynamic_pointer_cast<AST::NegateOperator>(op)) {
            return std::make_shared<IR::NegateOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::ComplementOperator>(op)) {
            return std::make_shared<IR::ComplementOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::NotOperator>(op)) {
            return std::make_shared<IR::NotOperator>();
        }
        throw std::runtime_error("Unsupported unary operator");
    }

    std::shared_ptr<IR::BinaryOperator>
    convertBinop(std::shared_ptr<AST::BinaryOperator> op) {
        if (std::dynamic_pointer_cast<AST::AddOperator>(op)) {
            return std::make_shared<IR::AddOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::SubtractOperator>(op)) {
            return std::make_shared<IR::SubtractOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::MultiplyOperator>(op)) {
            return std::make_shared<IR::MultiplyOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::DivideOperator>(op)) {
            return std::make_shared<IR::DivideOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::RemainderOperator>(op)) {
            return std::make_shared<IR::RemainderOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::EqualOperator>(op)) {
            return std::make_shared<IR::EqualOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::NotEqualOperator>(op)) {
            return std::make_shared<IR::NotEqualOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::LessThanOperator>(op)) {
            return std::make_shared<IR::LessThanOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::LessThanOrEqualOperator>(op)) {
            return std::make_shared<IR::LessThanOrEqualOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::GreaterThanOperator>(op)) {
            return std::make_shared<IR::GreaterThanOperator>();
        }
        else if (std::dynamic_pointer_cast<AST::GreaterThanOrEqualOperator>(
                     op)) {
            return std::make_shared<IR::GreaterThanOrEqualOperator>();
        }
        throw std::runtime_error("Unsupported binary operator");
    }
};