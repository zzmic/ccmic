#include "irGenerator.h"

namespace IR {
IRGenerator::IRGenerator(int variableResolutionCounter) {
    // Initialize the IR temporaries counter.
    this->irTemporariesCounter = variableResolutionCounter;
}

std::shared_ptr<IR::Program>
IRGenerator::generate(std::shared_ptr<AST::Program> astProgram) {
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

    // Get the body of the function.
    auto astBody = astFunction->getBody();

    // Generate IR instructions for the body of the function.
    for (auto &blockItem : *astBody) {
        // If the block item is a `DBlockItem` (i.e., a declaration), ...
        if (auto dBlockItem =
                std::dynamic_pointer_cast<AST::DBlockItem>(blockItem)) {
            // Generate IR instructions for the declaration.
            generateIRDeclaration(dBlockItem->getDeclaration(),
                                  functionDefinition->at(0)->getFunctionBody());
        }
        // If the block item is a `SBockItem` (i.e., a statement), ...
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<AST::SBlockItem>(blockItem)) {
            // Generate IR instructions for the statement.
            generateIRStatement(sBlockItem->getStatement(),
                                functionDefinition->at(0)->getFunctionBody());
        }
    }

    // If the function does not end with a return instruction, add a return
    // instruction with constant value 0.
    if ((functionDefinition->at(0)->getFunctionBody()->empty()) ||
        (std::dynamic_pointer_cast<IR::ReturnInstruction>(
             functionDefinition->at(0)->getFunctionBody()->back()) ==
         nullptr)) {
        auto functionBody = functionDefinition->at(0)->getFunctionBody();
        functionBody->emplace_back(std::make_shared<IR::ReturnInstruction>(
            std::make_shared<IR::ConstantValue>(0)));
        functionDefinition->at(0)->setFunctionBody(functionBody);
    }

    // Return the generated IR program.
    return std::make_shared<IR::Program>(functionDefinition);
}

void IRGenerator::generateIRDeclaration(
    std::shared_ptr<AST::Declaration> astDeclaration,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // If the declaration has an initializer, ...
    auto identifier = astDeclaration->getIdentifier();
    auto initializer = astDeclaration->getOptInitializer();
    if (initializer.has_value()) {
        // Generate IR instructions for the initializer.
        std::shared_ptr<IR::Value> result =
            generateIRInstruction(initializer.value(), instructions);

        // Create a variable value for the identifier.
        std::shared_ptr<IR::VariableValue> dst =
            std::make_shared<IR::VariableValue>(identifier);

        // Generate a copy instruction with the result value and the
        // destination value.
        instructions->emplace_back(
            std::make_shared<IR::CopyInstruction>(result, dst));
    }
    // Otherwise (i.e., if the declaration does not have an initializer),
    // we do not need to generate any IR instructions.
}

void IRGenerator::generateIRStatement(
    std::shared_ptr<AST::Statement> astStatement,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    if (auto returnStmt =
            std::dynamic_pointer_cast<AST::ReturnStatement>(astStatement)) {
        // If the statement is a return statement, generate a return
        // statement.
        generateIRReturnStatement(returnStmt, instructions);
    }
    else if (auto expressionStmt =
                 std::dynamic_pointer_cast<AST::ExpressionStatement>(
                     astStatement)) {
        // If the statement is an expression statement, generate an
        // expression statement.
        generateIRExpressionStatement(expressionStmt, instructions);
    }
    else if (auto nullStmt =
                 std::dynamic_pointer_cast<AST::NullStatement>(astStatement)) {
        // If the statement is a null statement, do nothing.
    }
    else {
        throw std::runtime_error("Unsupported statement type");
    }
}

void IRGenerator::generateIRReturnStatement(
    std::shared_ptr<AST::ReturnStatement> returnStmt,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Get the expression from the return statement.
    std::shared_ptr<AST::Expression> exp = returnStmt->getExpression();

    // Process the expression and generate the corresponding IR
    // instructions.
    std::shared_ptr<IR::Value> result =
        generateIRInstruction(exp, instructions);

    // Generate a return instruction with the result value.
    instructions->emplace_back(std::make_shared<IR::ReturnInstruction>(result));
}

void IRGenerator::generateIRExpressionStatement(
    std::shared_ptr<AST::ExpressionStatement> expressionStmt,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Get the expression from the expression statement.
    std::shared_ptr<AST::Expression> exp = expressionStmt->getExpression();

    // Process the expression and generate the corresponding IR
    // instructions.
    std::shared_ptr<IR::Value> result =
        generateIRInstruction(exp, instructions);
    // We do not need to do anything with the result value.
}

std::shared_ptr<IR::Value> IRGenerator::generateIRInstruction(
    std::shared_ptr<AST::Expression> e,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    if (auto constantExpr =
            std::dynamic_pointer_cast<AST::ConstantExpression>(e)) {
        return std::make_shared<IR::ConstantValue>(constantExpr->getValue());
    }

    else if (auto unaryExpr =
                 std::dynamic_pointer_cast<AST::UnaryExpression>(e)) {
        return generateIRUnaryInstruction(unaryExpr, instructions);
    }

    // If the expression is a binary expression, ...
    else if (auto binaryExpr =
                 std::dynamic_pointer_cast<AST::BinaryExpression>(e)) {
        // If the binary operator in the AST binary expression is a
        // logical-and operator, ...
        if (auto astBinaryOperator =
                std::dynamic_pointer_cast<AST::AndOperator>(
                    binaryExpr->getOperator())) {
            return generateIRInstructionWithLogicalAnd(binaryExpr,
                                                       instructions);
        }
        // If the binary operator in the AST binary expression is a
        // logical-or operator, ...
        else if (auto astBinaryOperator =
                     std::dynamic_pointer_cast<AST::OrOperator>(
                         binaryExpr->getOperator())) {
            return generateIRInstructionWithLogicalOr(binaryExpr, instructions);
        }
        // Otherwise (i.e., if the binary operator in the AST binary
        // expression can be converted to a binary operator in the IR), ...
        else {
            return generateIRBinaryInstruction(binaryExpr, instructions);
        }
    }

    else if (auto variableExpr =
                 std::dynamic_pointer_cast<AST::VariableExpression>(e)) {
        return std::make_shared<IR::VariableValue>(
            variableExpr->getIdentifier());
    }

    else if (auto assignmentExpr =
                 std::dynamic_pointer_cast<AST::AssignmentExpression>(e)) {
        if (auto variableExpr =
                std::dynamic_pointer_cast<AST::VariableExpression>(
                    assignmentExpr->getLeft())) {
            std::shared_ptr<IR::VariableValue> variableValue =
                std::make_shared<IR::VariableValue>(
                    variableExpr->getIdentifier());
            auto result =
                generateIRInstruction(assignmentExpr->getRight(), instructions);
            instructions->emplace_back(
                std::make_shared<IR::CopyInstruction>(result, variableValue));
            return variableValue;
        }
        else {
            throw std::runtime_error("Unsupported lvalue type in assignment");
        }
    }
    else {
        throw std::runtime_error("Unsupported assignment type");
    }
}

std::shared_ptr<IR::VariableValue> IRGenerator::generateIRUnaryInstruction(
    std::shared_ptr<AST::UnaryExpression> unaryExpr,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Recursively generate the expression in the unary expression.
    std::shared_ptr<IR::Value> src =
        generateIRInstruction(unaryExpr->getExpression(), instructions);

    // Create a temporary variable (in string) to store the result of
    // the unary operation.
    std::string tmpName = generateIRTemporary();

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

std::shared_ptr<IR::VariableValue> IRGenerator::generateIRBinaryInstruction(
    std::shared_ptr<AST::BinaryExpression> binaryExpr,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Convert the binary operator in the binary expression to a IR
    // binary operator.
    std::shared_ptr<IR::BinaryOperator> IROp =
        convertBinop(binaryExpr->getOperator());

    // Create a temporary variable (in string) to store the result of
    // the binary operation.
    std::string tmpName = generateIRTemporary();
    // Create a variable value for the temporary variable.
    std::shared_ptr<IR::VariableValue> dst =
        std::make_shared<IR::VariableValue>(tmpName);

    // Recursively generate the left and right expressions in the binary
    // expression.
    std::shared_ptr<IR::Value> lhs =
        generateIRInstruction(binaryExpr->getLeft(), instructions);
    std::shared_ptr<IR::Value> rhs =
        generateIRInstruction(binaryExpr->getRight(), instructions);
    // Generate a binary instruction with the IR binary operator, the
    // left-hand side value, the right-hand side value, and the
    // destination value.
    instructions->emplace_back(
        std::make_shared<IR::BinaryInstruction>(IROp, lhs, rhs, dst));

    // Return the destination value.
    return dst;
}

std::shared_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalAnd(
    std::shared_ptr<AST::BinaryExpression> binaryExpr,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Recursively generate the left expression in the binary expression.
    std::shared_ptr<IR::Value> lhs =
        generateIRInstruction(binaryExpr->getLeft(), instructions);

    // Generate a JumpIfZero instruction with the left-hand side value and a
    // (new) false label.
    std::string falseLabel = generateIRFalseLabel();
    generateIRJumpIfZeroInstruction(lhs, falseLabel, instructions);

    // Recursively generate the right expression in the binary expression.
    std::shared_ptr<IR::Value> rhs =
        generateIRInstruction(binaryExpr->getRight(), instructions);

    // Generate a JumpIfZero instruction with the right-hand side value and
    // the same (new) false label.
    generateIRJumpIfZeroInstruction(rhs, falseLabel, instructions);

    // Generate a copy instruction with 1 being copied to a (new) result
    // label.
    std::string resultLabel = generateIRResultLabel();
    std::shared_ptr<IR::VariableValue> dst =
        std::make_shared<IR::VariableValue>(resultLabel);
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(1), dst,
                              instructions);

    // Generate a jump instruction with a new end label.
    std::string endLabel = generateIREndLabel();
    generateIRJumpInstruction(endLabel, instructions);

    // Generate a label instruction with the same (new) false label.
    generateIRLabelInstruction(falseLabel, instructions);

    // Generate a copy instruction with 0 being copied to the result.
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(0), dst,
                              instructions);

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);

    // Return the destination value.
    return dst;
}

std::shared_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalOr(
    std::shared_ptr<AST::BinaryExpression> binaryExpr,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Recursively generate the left expression in the binary expression.
    std::shared_ptr<IR::Value> lhs =
        generateIRInstruction(binaryExpr->getLeft(), instructions);

    // Generate a JumpIfNotZero instruction with the left-hand side value
    // and a (new) true label.
    std::string trueLabel = generateIRTrueLabel();
    generateIRJumpIfNotZeroInstruction(lhs, trueLabel, instructions);

    // Recursively generate the right expression in the binary expression.
    std::shared_ptr<IR::Value> rhs =
        generateIRInstruction(binaryExpr->getRight(), instructions);

    // Generate a JumpIfNotZero instruction with the right-hand side value
    // and the same (new) true label.
    std::string trueLabelRight = generateIRTrueLabel();
    generateIRJumpIfNotZeroInstruction(rhs, trueLabel, instructions);

    // Generate a copy instruction with 0 being copied to a (new) result
    // label.
    std::string resultLabel = generateIRResultLabel();
    std::shared_ptr<IR::VariableValue> dst =
        std::make_shared<IR::VariableValue>(resultLabel);
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(0), dst,
                              instructions);

    // Generate a jump instruction with a new end label.
    std::string endLabel = generateIREndLabel();
    generateIRJumpInstruction(endLabel, instructions);

    // Generate a label instruction with the same (new) true label.
    generateIRLabelInstruction(trueLabel, instructions);

    // Generate a copy instruction with 1 being copied to the result.
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(1), dst,
                              instructions);

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);

    // Return the destination value.
    return dst;
}

void IRGenerator::generateIRCopyInstruction(
    std::shared_ptr<IR::Value> src, std::shared_ptr<IR::Value> dst,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Generate a copy instruction with the source value and the
    // destination value.
    instructions->emplace_back(std::make_shared<IR::CopyInstruction>(src, dst));
}

void IRGenerator::generateIRJumpInstruction(
    std::string target,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Generate a jump instruction with the target label.
    instructions->emplace_back(std::make_shared<IR::JumpInstruction>(target));
}

void IRGenerator::generateIRJumpIfZeroInstruction(
    std::shared_ptr<IR::Value> condition, std::string target,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Generate a jump if zero instruction with the condition value and
    // the target label.
    instructions->emplace_back(
        std::make_shared<IR::JumpIfZeroInstruction>(condition, target));
}

void IRGenerator::generateIRJumpIfNotZeroInstruction(
    std::shared_ptr<IR::Value> condition, std::string target,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Generate a jump if not zero instruction with the condition value
    // and the target label.
    instructions->emplace_back(
        std::make_shared<IR::JumpIfNotZeroInstruction>(condition, target));
}

void IRGenerator::generateIRLabelInstruction(
    std::string identifier,
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        instructions) {
    // Generate a label instruction with the label identifier.
    instructions->emplace_back(
        std::make_shared<IR::LabelInstruction>(identifier));
}

std::string IRGenerator::generateIRTemporary() {
    // Return the string representation of the temporary variable name.
    return "tmp." + std::to_string(irTemporariesCounter++);
}

std::string IRGenerator::generateIRFalseLabel() {
    // Create a label with a unique number.
    // The number would be incremented each time this function is called.
    static int counter = 0;

    // Return the string representation of the (unique) label using the
    // string "and_falseN" (as "false_label" in the listing), "where N is
    // the current value of a global counter."
    return "and_false" + std::to_string(counter++);
}

std::string IRGenerator::generateIRTrueLabel() {
    // Create a label with a unique number.
    // The number would be incremented each time this function is called.
    static int counter = 0;

    // Return the string representation of the (unique) label using the
    // string "or_trueN" (similar to "false_label" in the listing), "where
    // N is the current value of a global counter."
    return "or_true" + std::to_string(counter++);
}

std::string IRGenerator::generateIRResultLabel() {
    // Create a label with a unique number.
    // The number would be incremented each time this function is called.
    static int counter = 0;

    // Return the string representation of the (unique) label using the
    // string "resultN" (similar to "false_label" in the listing), "where N
    // is the current value of a global counter."
    return "result" + std::to_string(counter++);
}

std::string IRGenerator::generateIREndLabel() {
    // Create a label with a unique number.
    // The number would be incremented each time this function is called.
    static int counter = 0;

    // Return the string representation of the (unique) label using the
    // string "endN" (similar to "false_label" in the listing), "where N is
    // the current value of a global counter."
    return "end" + std::to_string(counter++);
}

std::shared_ptr<IR::UnaryOperator>
IRGenerator::convertUnop(std::shared_ptr<AST::UnaryOperator> op) {
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

// Note: The logical-and and logical-or operators in the AST are NOT binary
// operators in the IR (and should NOT be converted to binary operators in
// the IR).
std::shared_ptr<IR::BinaryOperator>
IRGenerator::convertBinop(std::shared_ptr<AST::BinaryOperator> op) {
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
    else if (std::dynamic_pointer_cast<AST::GreaterThanOrEqualOperator>(op)) {
        return std::make_shared<IR::GreaterThanOrEqualOperator>();
    }
    throw std::runtime_error("Unsupported binary operator");
}
} // namespace IR
