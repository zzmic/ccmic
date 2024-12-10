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
        functionDefinition->push_back(
            std::make_shared<IR::FunctionDefinition>(astFunction->getName()));

        // Initialize the function body with an empty vector of instructions.
        functionDefinition->at(0)->setFunctionBody(
            std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>(
                new std::vector<std::shared_ptr<IR::Instruction>>()));

        // Get the body of the function (single statement for now).
        std::shared_ptr<AST::Statement> astBody = astFunction->getBody();
        // Generate IR instructions for the body of the function (the
        // single statement aforementioned).
        generateStatement(astBody,
                          functionDefinition->at(0)->getFunctionBody());

        // Return the generated IR program.
        return std::shared_ptr<IR::Program>(
            new IR::Program(functionDefinition));
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

        // Process the expression and emit the corresponding IR instructions.
        std::shared_ptr<IR::Value> result = emit_tacky(exp, instructions);

        // Emit a return instruction with the result.
        instructions->emplace_back(std::shared_ptr<IR::ReturnInstruction>(
            new IR::ReturnInstruction(result)));
    }

    std::shared_ptr<IR::Value>
    emit_tacky(std::shared_ptr<AST::Expression> e,
               std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
                   instructions) {
        // If the expression is a constant expression, emit a constant value.
        if (auto integerExpr =
                std::dynamic_pointer_cast<AST::IntegerExpression>(e)) {
            return std::shared_ptr<IR::IntegerValue>(
                new IR::IntegerValue(integerExpr->getValue()));
        }
        // If the expression is a unary expression, ...
        else if (auto unaryExpr =
                     std::dynamic_pointer_cast<AST::UnaryExpression>(e)) {
            // Recursively emit the expression in the unary expression.
            std::shared_ptr<IR::Value> src =
                emit_tacky(unaryExpr->getExpression(), instructions);
            // Create a temporary variable (in string) to store the result of
            // the unary operation.
            std::string tmpName = make_temporary();
            // Create a variable value for the temporary variable.
            std::shared_ptr<IR::VariableValue> dst =
                std::shared_ptr<IR::VariableValue>(
                    new IR::VariableValue(tmpName));

            // Convert the unary operator in the unary expression to a tacky
            // unary operator.
            std::shared_ptr<IR::UnaryOperator> tackyOp =
                convert_unop(unaryExpr->getOperator());
            // Emit a unary instruction with the tacky unary operator, the
            // source value, and the destination value.
            instructions->emplace_back(std::shared_ptr<IR::UnaryInstruction>(
                new IR::UnaryInstruction(tackyOp, src, dst)));

            // Return the destination value.
            return dst;
        }
        throw std::runtime_error("Unsupported expression type");
    }

    std::string make_temporary() {
        // Create a temporary variable name with a unique number.
        // The number would be incremented each time this function is called.
        static int counter = 0;
        // Return the string representation of the temporary variable name.
        return "tmp." + std::to_string(counter++);
    }

    std::shared_ptr<IR::UnaryOperator>
    convert_unop(std::shared_ptr<AST::UnaryOperator> op) {
        // If the unary operator is a negate operator, return a tacky negate
        // operator.
        if (std::dynamic_pointer_cast<AST::NegateOperator>(op)) {
            return std::shared_ptr<IR::NegateOperator>(
                new IR::NegateOperator());
        }
        // If the unary operator is a complement operator, return a tacky
        // complement operator.
        else if (std::dynamic_pointer_cast<AST::ComplementOperator>(op)) {
            return std::shared_ptr<IR::ComplementOperator>(
                new IR::ComplementOperator());
        }
        throw std::runtime_error("Unsupported unary operator");
    }
};
