#include "irGenerator.h"

namespace IR {
IRGenerator::IRGenerator(
    int variableResolutionCounter,
    const std::unordered_map<
        std::string, std::pair<std::shared_ptr<AST::Type>,
                               std::shared_ptr<AST::IdentifierAttribute>>>
        &symbols)
    : irTemporariesCounter(variableResolutionCounter), symbols(symbols) {}

std::pair<std::shared_ptr<IR::Program>,
          std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
IRGenerator::generateIR(const std::shared_ptr<AST::Program> &astProgram) {
    // Initialize the vector of IR top-levels (top-level elements).
    auto topLevels =
        std::make_shared<std::vector<std::shared_ptr<IR::TopLevel>>>();

    // Generate IR instructions for each AST top-level element.
    auto astDeclarations = astProgram->getDeclarations();
    for (const auto &astDeclaration : *astDeclarations) {
        if (auto functionDeclaration =
                std::dynamic_pointer_cast<AST::FunctionDeclaration>(
                    astDeclaration)) {
            // Get the body of the function declaration.
            auto optBody = functionDeclaration->getOptBody();
            // Skip generating IR instructions for forward declarations.
            if (!optBody.has_value()) {
                continue;
            }
            // Create a new vector of IR instructions for the function.
            auto instructions = std::make_shared<
                std::vector<std::shared_ptr<IR::Instruction>>>();
            // Get the identifier and the parameters of the function
            // declaration.
            auto identifier = functionDeclaration->getIdentifier();
            auto parameters = functionDeclaration->getParameterIdentifiers();
            // Find the global attribute of the function declaration in the
            // symbols and set the global flag.
            bool global = false;
            if (symbols.find(identifier) != symbols.end()) {
                if (auto functionAttribute =
                        std::dynamic_pointer_cast<AST::FunctionAttribute>(
                            symbols[identifier].second)) {
                    global = functionAttribute->isGlobal();
                }
                else {
                    throw std::logic_error(
                        "Function attribute not found in symbols while "
                        "generating IR instructions for function definition");
                }
            }
            else {
                throw std::logic_error(
                    "Function declaration not found in symbols while "
                    "generating IR instructions for function definition");
            }
            // Generate IR instructions for the body of the function.
            generateIRBlock(optBody.value(), instructions);
            // Create a new IR function definition with the function identifier,
            // the parameters, and the vector of IR instructions.
            auto functionDefinition = std::make_shared<IR::FunctionDefinition>(
                functionDeclaration->getIdentifier(), global, parameters,
                instructions);
            // If the function does not end with a return instruction, add a
            // return instruction with constant value 0.
            if (functionDefinition->getFunctionBody()->empty() ||
                !std::dynamic_pointer_cast<IR::ReturnInstruction>(
                    functionDefinition->getFunctionBody()->back())) {
                functionDefinition->getFunctionBody()->emplace_back(
                    std::make_shared<IR::ReturnInstruction>(
                        std::make_shared<IR::ConstantValue>(
                            std::make_shared<AST::ConstantInt>(0))));
            }
            // Add the IR function definition to the vector of IR top-levels.
            topLevels->emplace_back(std::move(functionDefinition));
        }
        else if (auto variableDeclaration =
                     std::dynamic_pointer_cast<AST::VariableDeclaration>(
                         astDeclaration)) {
            // Continue: Do not generate IR instructions for file-scope variable
            // declarations or for local variable declarations with `static` or
            // `extern` storage-class specifiers (for now).
            if (variableDeclaration->getOptStorageClass().has_value()) {
                if (std::dynamic_pointer_cast<AST::StaticStorageClass>(
                        variableDeclaration->getOptStorageClass().value()) ||
                    std::dynamic_pointer_cast<AST::ExternStorageClass>(
                        variableDeclaration->getOptStorageClass().value())) {
                    continue;
                }
            }
        }
    }

    // Examine every entry in the symbol table and generate `StaticVariable` IR
    // constructs for some of them after traversing the AST.
    auto irStaticVariables = convertSymbolsToIRStaticVariables();

    // Return the generated IR program.
    return std::make_pair(std::make_shared<IR::Program>(std::move(topLevels)),
                          std::move(irStaticVariables));
}

void IRGenerator::generateIRBlock(
    const std::shared_ptr<AST::Block> &astBlock,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Get the block items from the block.
    auto blockItems = astBlock->getBlockItems();

    // Generate IR instructions for each block item.
    for (const auto &blockItem : *blockItems) {
        // If the block item is a `DBlockItem` (i.e., a declaration), ...
        if (auto dBlockItem =
                std::dynamic_pointer_cast<AST::DBlockItem>(blockItem)) {
            // Generate IR instructions for the variable declaration (that has
            // an initializer).
            if (auto variableDeclaration =
                    std::dynamic_pointer_cast<AST::VariableDeclaration>(
                        dBlockItem->getDeclaration())) {
                // Continue: Do not generate IR instructions for file-scope
                // variable declarations or for local variable declarations with
                // `static` or `extern` storage-class specifiers (for now).
                if (variableDeclaration->getOptStorageClass().has_value()) {
                    if (std::dynamic_pointer_cast<AST::StaticStorageClass>(
                            variableDeclaration->getOptStorageClass()
                                .value()) ||
                        std::dynamic_pointer_cast<AST::ExternStorageClass>(
                            variableDeclaration->getOptStorageClass()
                                .value())) {
                        continue;
                    }
                }
                else {
                    generateIRVariableDefinition(variableDeclaration,
                                                 instructions);
                }
            }
            // Generate IR instructions for the function declaration (that has a
            // body).
            else if (auto functionDeclaration =
                         std::dynamic_pointer_cast<AST::FunctionDeclaration>(
                             dBlockItem->getDeclaration())) {
                generateIRFunctionDefinition(functionDeclaration, instructions);
            }
        }
        // If the block item is a `SBockItem` (i.e., a statement), ...
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<AST::SBlockItem>(blockItem)) {
            // Generate IR instructions for the statement.
            generateIRStatement(sBlockItem->getStatement(), instructions);
        }
    }
}

void IRGenerator::generateIRFunctionDefinition(
    const std::shared_ptr<AST::FunctionDeclaration> &astFunctionDeclaration,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Get the body of the function.
    auto optBody = astFunctionDeclaration->getOptBody();
    if (optBody.has_value()) {
        // Generate IR instructions for the body of the function.
        generateIRBlock(optBody.value(), instructions);
    }
    // Otherwise (i.e., if the function does not have a body), we do not need
    // to generate any IR instructions for now.
}

void IRGenerator::generateIRVariableDefinition(
    const std::shared_ptr<AST::VariableDeclaration> &astVariableDeclaration,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    auto identifier = astVariableDeclaration->getIdentifier();
    auto initializer = astVariableDeclaration->getOptInitializer();
    // If the declaration has an initializer, ...
    if (initializer.has_value()) {
        // Generate IR instructions for the initializer.
        auto result = generateIRInstruction(initializer.value(), instructions);

        // Create a variable value for the identifier.
        auto dst = std::make_shared<IR::VariableValue>(identifier);

        // Generate a copy instruction with the result value and the
        // destination value.
        instructions->emplace_back(std::make_shared<IR::CopyInstruction>(
            std::move(result), std::move(dst)));
    }
    // Otherwise (i.e., if the declaration does not have an initializer),
    // we do not need to generate any IR instructions.
}

void IRGenerator::generateIRStatement(
    const std::shared_ptr<AST::Statement> &astStatement,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    if (auto returnStmt =
            std::dynamic_pointer_cast<AST::ReturnStatement>(astStatement)) {
        generateIRReturnStatement(returnStmt, instructions);
    }
    else if (auto expressionStmt =
                 std::dynamic_pointer_cast<AST::ExpressionStatement>(
                     astStatement)) {
        generateIRExpressionStatement(expressionStmt, instructions);
    }
    else if (auto compoundStmt =
                 std::dynamic_pointer_cast<AST::CompoundStatement>(
                     astStatement)) {
        // If the statement is a compound statement, generate a block.
        generateIRBlock(compoundStmt->getBlock(), instructions);
    }
    else if (auto ifStmt =
                 std::dynamic_pointer_cast<AST::IfStatement>(astStatement)) {
        generateIRIfStatement(ifStmt, instructions);
    }
    else if (auto breakStmt =
                 std::dynamic_pointer_cast<AST::BreakStatement>(astStatement)) {
        generateIRBreakStatement(breakStmt, instructions);
    }
    else if (auto continueStmt =
                 std::dynamic_pointer_cast<AST::ContinueStatement>(
                     astStatement)) {
        generateIRContinueStatement(continueStmt, instructions);
    }
    else if (auto whileStmt =
                 std::dynamic_pointer_cast<AST::WhileStatement>(astStatement)) {
        generateIRWhileStatement(whileStmt, instructions);
    }
    else if (auto doWhileStmt =
                 std::dynamic_pointer_cast<AST::DoWhileStatement>(
                     astStatement)) {
        generateIRDoWhileStatement(doWhileStmt, instructions);
    }
    else if (auto forStmt =
                 std::dynamic_pointer_cast<AST::ForStatement>(astStatement)) {
        generateIRForStatement(forStmt, instructions);
    }
    else if (auto nullStmt =
                 std::dynamic_pointer_cast<AST::NullStatement>(astStatement)) {
        // If the statement is a null statement, do nothing.
    }
    else {
        throw std::logic_error("Unsupported statement type while generating "
                               "IR instructions for statement");
    }
}

void IRGenerator::generateIRReturnStatement(
    const std::shared_ptr<AST::ReturnStatement> &returnStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Get the expression from the return statement.
    auto exp = returnStmt->getExpression();

    // Process the expression and generate the corresponding IR
    // instructions.
    auto result = generateIRInstruction(exp, instructions);

    // Generate a return instruction with the result value.
    instructions->emplace_back(
        std::make_shared<IR::ReturnInstruction>(std::move(result)));
}

void IRGenerator::generateIRExpressionStatement(
    const std::shared_ptr<AST::ExpressionStatement> &expressionStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Get the expression from the expression statement.
    auto exp = expressionStmt->getExpression();

    // Process the expression and generate the corresponding IR
    // instructions.
    auto result = generateIRInstruction(exp, instructions);
    // We do not need to do anything with the result value.
}

void IRGenerator::generateIRIfStatement(
    const std::shared_ptr<AST::IfStatement> &ifStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Get the condition from the if-statement.
    auto condition = ifStmt->getCondition();
    // Process the condition and generate the corresponding IR instructions.
    auto conditionValue = generateIRInstruction(condition, instructions);
    // Generate a new end label.
    auto endLabel = generateIREndLabel();
    // Generate a new else label.
    auto elseLabel = generateIRElseLabel();

    if (ifStmt->getElseOptStatement().has_value()) {
        // Generate a jump-if-zero instruction with the condition value and the
        // else label.
        instructions->emplace_back(std::make_shared<IR::JumpIfZeroInstruction>(
            std::move(conditionValue), elseLabel));
        // Get the then-statement from the if-statement.
        auto thenStatement = ifStmt->getThenStatement();
        // Process the then-statement and generate the corresponding IR
        // instructions.
        generateIRStatement(thenStatement, instructions);
        // Generate a jump instruction with the end label.
        generateIRJumpInstruction(endLabel, instructions);
        // Generate a label instruction with the same (new) else label.
        generateIRLabelInstruction(elseLabel, instructions);
        // Get the (optional) else-statement from the if-statement.
        auto elseOptStatement = ifStmt->getElseOptStatement().value();
        // Process the else-statement and generate the corresponding IR
        // instructions.
        generateIRStatement(elseOptStatement, instructions);
    }
    else {
        // Generate a jump-if-zero instruction with the condition value and the
        // end label.
        instructions->emplace_back(std::make_shared<IR::JumpIfZeroInstruction>(
            std::move(conditionValue), endLabel));
        // Get the then-statement from the if-statement.
        auto thenStatement = ifStmt->getThenStatement();
        // Process the then-statement and generate the corresponding IR
        // instructions.
        generateIRStatement(thenStatement, instructions);
    }

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);
}

void IRGenerator::generateIRBreakStatement(
    const std::shared_ptr<AST::BreakStatement> &breakStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a jump instruction with the extended break label.
    auto breakLabel = generateIRBreakLoopLabel(breakStmt->getLabel());
    generateIRJumpInstruction(breakLabel, instructions);
}

void IRGenerator::generateIRContinueStatement(
    const std::shared_ptr<AST::ContinueStatement> &continueStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a jump instruction with the extended continue label.
    auto continueLabel = generateIRContinueLoopLabel(continueStmt->getLabel());
    generateIRJumpInstruction(continueLabel, instructions);
}

void IRGenerator::generateIRDoWhileStatement(
    const std::shared_ptr<AST::DoWhileStatement> &doWhileStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a new start label.
    auto startLabel = generateIRStartLabel();
    // Generate a label instruction with the start label.
    generateIRLabelInstruction(startLabel, instructions);
    // Generate instructions for the body of the do-while-statement.
    auto body = doWhileStmt->getBody();
    generateIRStatement(body, instructions);
    // Generate a new continue label (based on the label of the do-while).
    auto continueLabel = generateIRContinueLoopLabel(doWhileStmt->getLabel());
    // Generate a label instruction with the continue label.
    generateIRLabelInstruction(continueLabel, instructions);
    // Generate instructions for the condition of the do-while-statement.
    auto condition = doWhileStmt->getCondition();
    auto conditionValue = generateIRInstruction(condition, instructions);
    // Generate a jump-if-not-zero instruction with the condition value and the
    // start label.
    generateIRJumpIfNotZeroInstruction(conditionValue, startLabel,
                                       instructions);
    // Generate a new break label (based on the label of the do-while).
    auto breakLabel = generateIRBreakLoopLabel(doWhileStmt->getLabel());
    // Generate a label instruction with the break label.
    generateIRLabelInstruction(breakLabel, instructions);
}

void IRGenerator::generateIRWhileStatement(
    const std::shared_ptr<AST::WhileStatement> &whileStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a new continue label (based on the label of the while).
    auto continueLabel = generateIRContinueLoopLabel(whileStmt->getLabel());
    // Generate a label instruction with the continue label.
    generateIRLabelInstruction(continueLabel, instructions);
    // Generate instructions for the condition of the while-statement.
    auto condition = whileStmt->getCondition();
    auto conditionValue = generateIRInstruction(condition, instructions);
    // Generate a new break label (based on the label of the while).
    auto breakLabel = generateIRBreakLoopLabel(whileStmt->getLabel());
    // Generate a jump-if-zero instruction with the condition value and the
    // break label.
    generateIRJumpIfZeroInstruction(conditionValue, breakLabel, instructions);
    // Generate instructions for the body of the while-statement.
    auto body = whileStmt->getBody();
    generateIRStatement(body, instructions);
    // Generate a jump instruction with the continue label.
    generateIRJumpInstruction(continueLabel, instructions);
    // Generate a label instruction with the break label.
    generateIRLabelInstruction(breakLabel, instructions);
}

void IRGenerator::generateIRForStatement(
    const std::shared_ptr<AST::ForStatement> &forStmt,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate instructions for the for-init of the for-statement.
    auto forInit = forStmt->getForInit();
    if (auto initExpr = std::dynamic_pointer_cast<AST::InitExpr>(forInit)) {
        auto optExpr = initExpr->getExpression();
        if (optExpr.has_value()) {
            auto resolvedExpr =
                generateIRInstruction(optExpr.value(), instructions);
        }
    }
    else if (auto initDecl =
                 std::dynamic_pointer_cast<AST::InitDecl>(forInit)) {
        generateIRVariableDefinition(initDecl->getVariableDeclaration(),
                                     instructions);
    }
    // Generate a new start label.
    auto startLabel = generateIRStartLabel();
    // Generate a label instruction with the start label.
    generateIRLabelInstruction(startLabel, instructions);
    // Generate a new break label (based on the label of the for).
    auto breakLabel = generateIRBreakLoopLabel(forStmt->getLabel());
    // Optionally generate instructions for the (optional) condition of the
    // for-statement.
    auto optCondition = forStmt->getOptCondition();
    if (optCondition.has_value()) {
        // Generate a jump-if-zero instruction with the condition value and the
        // break label.
        auto conditionValue =
            generateIRInstruction(optCondition.value(), instructions);
        generateIRJumpIfZeroInstruction(conditionValue, breakLabel,
                                        instructions);
    }
    // Generate instructions for the body of the for-statement.
    auto body = forStmt->getBody();
    generateIRStatement(body, instructions);
    // Generate a new continue label (based on the label of the for).
    auto continueLabel = generateIRContinueLoopLabel(forStmt->getLabel());
    // Generate a label instruction with the continue label.
    generateIRLabelInstruction(continueLabel, instructions);
    // Optionally generate instructions for the (optional) post of the
    // for-statement.
    auto optPost = forStmt->getOptPost();
    if (optPost.has_value()) {
        auto postValue = generateIRInstruction(optPost.value(), instructions);
    }
    // Generate a jump instruction with the start label.
    generateIRJumpInstruction(startLabel, instructions);
    // Generate a label instruction with the break label.
    generateIRLabelInstruction(breakLabel, instructions);
}

std::shared_ptr<IR::Value> IRGenerator::generateIRInstruction(
    const std::shared_ptr<AST::Expression> &e,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    if (auto constantExpr =
            std::dynamic_pointer_cast<AST::ConstantExpression>(e)) {
        auto variantValue = constantExpr->getConstantInIntOrLongVariant();
        if (std::holds_alternative<int>(variantValue)) {
            return std::make_shared<IR::ConstantValue>(
                std::make_shared<AST::ConstantInt>(
                    std::get<int>(variantValue)));
        }
        else if (std::holds_alternative<long>(variantValue)) {
            return std::make_shared<IR::ConstantValue>(
                std::make_shared<AST::ConstantLong>(
                    std::get<long>(variantValue)));
        }
        else {
            throw std::logic_error(
                "Unsupported constant type while generating IR instructions "
                "for expression");
        }
    }
    else if (auto unaryExpr =
                 std::dynamic_pointer_cast<AST::UnaryExpression>(e)) {
        return generateIRUnaryInstruction(unaryExpr, instructions);
    }
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
            instructions->emplace_back(std::make_shared<IR::CopyInstruction>(
                std::move(result), variableValue));
            return variableValue;
        }
        else {
            throw std::logic_error(
                "Unsupported lvalue type in assignment while generating IR "
                "instructions for expression");
        }
    }
    else if (auto conditionalExpr =
                 std::dynamic_pointer_cast<AST::ConditionalExpression>(e)) {
        auto conditionValue = generateIRInstruction(
            conditionalExpr->getCondition(), instructions);
        auto e2Label = generateIRE2Label();
        generateIRJumpIfZeroInstruction(conditionValue, e2Label, instructions);
        auto e1Value = generateIRInstruction(
            conditionalExpr->getThenExpression(), instructions);
        auto resultLabel = generateIRResultLabel();
        auto resultValue = std::make_shared<IR::VariableValue>(resultLabel);
        generateIRCopyInstruction(e1Value, resultValue, instructions);
        auto endLabel = generateIREndLabel();
        generateIRJumpInstruction(endLabel, instructions);
        generateIRLabelInstruction(e2Label, instructions);
        auto e2Value = generateIRInstruction(
            conditionalExpr->getElseExpression(), instructions);
        generateIRCopyInstruction(e2Value, resultValue, instructions);
        generateIRLabelInstruction(endLabel, instructions);
        return resultValue;
    }
    else if (auto functionCallExpr =
                 std::dynamic_pointer_cast<AST::FunctionCallExpression>(e)) {
        auto functionIdentifier = functionCallExpr->getIdentifier();
        auto args = std::make_shared<std::vector<std::shared_ptr<IR::Value>>>();
        for (auto &arg : *functionCallExpr->getArguments()) {
            args->emplace_back(generateIRInstruction(arg, instructions));
        }
        auto resultValue = generateIRFunctionCallInstruction(
            functionIdentifier, args, instructions);
        return resultValue;
    }
    else if (auto castExpr =
                 std::dynamic_pointer_cast<AST::CastExpression>(e)) {
        return generateIRCastInstruction(castExpr, instructions);
    }
    throw std::logic_error("Unsupported expression type while generating IR "
                           "instructions for expression");
}

std::shared_ptr<IR::VariableValue> IRGenerator::generateIRUnaryInstruction(
    const std::shared_ptr<AST::UnaryExpression> &unaryExpr,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Recursively generate the expression in the unary expression.
    auto src = generateIRInstruction(unaryExpr->getExpression(), instructions);

    // Create a temporary variable (in string) to store the result of
    // the unary operation.
    auto tmpName = generateIRTemporary();

    // Create a variable value for the temporary variable.
    auto dst = std::make_shared<IR::VariableValue>(tmpName);

    // Convert the unary operator in the unary expression to a IR
    // unary operator.
    auto IROp = convertUnop(unaryExpr->getOperator());

    // Generate a unary instruction with the IR unary operator, the
    // source value, and the destination value.
    instructions->emplace_back(std::make_shared<IR::UnaryInstruction>(
        std::move(IROp), std::move(src), dst));

    // Return the destination value.
    return dst;
}

std::shared_ptr<IR::VariableValue> IRGenerator::generateIRBinaryInstruction(
    const std::shared_ptr<AST::BinaryExpression> &binaryExpr,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Recursively generate the left and right expressions in the binary
    // expression.
    auto lhs = generateIRInstruction(binaryExpr->getLeft(), instructions);
    auto rhs = generateIRInstruction(binaryExpr->getRight(), instructions);

    // Create a temporary variable to store the result/destination of the
    // binary operation.
    auto dst = generateIRVariable(binaryExpr);

    // Convert the binary operator in the binary expression to a IR
    // binary operator.
    auto IROp = convertBinop(binaryExpr->getOperator());

    // Generate a binary instruction with the IR binary operator, the
    // left-hand side value, the right-hand side value, and the
    // destination value.
    instructions->emplace_back(std::make_shared<IR::BinaryInstruction>(
        std::move(IROp), std::move(lhs), std::move(rhs), dst));

    // Return the destination value.
    return dst;
}

std::shared_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalAnd(
    const std::shared_ptr<AST::BinaryExpression> &binaryExpr,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Recursively generate the left expression in the binary expression.
    auto lhs = generateIRInstruction(binaryExpr->getLeft(), instructions);

    // Generate a JumpIfZero instruction with the left-hand side value and a
    // (new) false label.
    auto falseLabel = generateIRFalseLabel();
    generateIRJumpIfZeroInstruction(std::move(lhs), falseLabel, instructions);

    // Recursively generate the right expression in the binary expression.
    auto rhs = generateIRInstruction(binaryExpr->getRight(), instructions);

    // Generate a JumpIfZero instruction with the right-hand side value and
    // the same (new) false label.
    generateIRJumpIfZeroInstruction(std::move(rhs), falseLabel, instructions);

    // Generate a copy instruction with 1 being copied to a (new) result
    // label.
    auto resultLabel = generateIRResultLabel();
    auto dst = std::make_shared<IR::VariableValue>(resultLabel);
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(
                                  std::make_shared<AST::ConstantInt>(1)),
                              dst, instructions);

    // Generate a jump instruction with a new end label.
    auto endLabel = generateIREndLabel();
    generateIRJumpInstruction(endLabel, instructions);

    // Generate a label instruction with the same (new) false label.
    generateIRLabelInstruction(falseLabel, instructions);

    // Generate a copy instruction with 0 being copied to the result.
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(
                                  std::make_shared<AST::ConstantInt>(0)),
                              dst, instructions);

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);

    // Return the destination value.
    return dst;
}

std::shared_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalOr(
    const std::shared_ptr<AST::BinaryExpression> &binaryExpr,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Recursively generate the left expression in the binary expression.
    auto lhs = generateIRInstruction(binaryExpr->getLeft(), instructions);

    // Generate a JumpIfNotZero instruction with the left-hand side value
    // and a (new) true label.
    auto trueLabel = generateIRTrueLabel();
    generateIRJumpIfNotZeroInstruction(std::move(lhs), trueLabel, instructions);

    // Recursively generate the right expression in the binary expression.
    auto rhs = generateIRInstruction(binaryExpr->getRight(), instructions);

    // Generate a JumpIfNotZero instruction with the right-hand side value
    // and the same (new) true label.
    auto trueLabelRight = generateIRTrueLabel();
    generateIRJumpIfNotZeroInstruction(std::move(rhs), trueLabel, instructions);

    // Generate a copy instruction with 0 being copied to a (new) result
    // label.
    auto resultLabel = generateIRResultLabel();
    auto dst = std::make_shared<IR::VariableValue>(resultLabel);
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(
                                  std::make_shared<AST::ConstantInt>(0)),
                              dst, instructions);

    // Generate a jump instruction with a new end label.
    auto endLabel = generateIREndLabel();
    generateIRJumpInstruction(endLabel, instructions);

    // Generate a label instruction with the same (new) true label.
    generateIRLabelInstruction(trueLabel, instructions);

    // Generate a copy instruction with 1 being copied to the result.
    generateIRCopyInstruction(std::make_shared<IR::ConstantValue>(
                                  std::make_shared<AST::ConstantInt>(1)),
                              dst, instructions);

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);

    // Return the destination value.
    return dst;
}

void IRGenerator::generateIRCopyInstruction(
    const std::shared_ptr<IR::Value> &src,
    const std::shared_ptr<IR::Value> &dst,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a copy instruction with the source value and the
    // destination value.
    instructions->emplace_back(std::make_shared<IR::CopyInstruction>(src, dst));
}

void IRGenerator::generateIRJumpInstruction(
    std::string_view target,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a jump instruction with the target label.
    instructions->emplace_back(std::make_shared<IR::JumpInstruction>(target));
}

void IRGenerator::generateIRJumpIfZeroInstruction(
    const std::shared_ptr<IR::Value> &condition, std::string_view target,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a jump if zero instruction with the condition value and
    // the target label.
    instructions->emplace_back(
        std::make_shared<IR::JumpIfZeroInstruction>(condition, target));
}

void IRGenerator::generateIRJumpIfNotZeroInstruction(
    const std::shared_ptr<IR::Value> &condition, std::string_view target,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a jump if not zero instruction with the condition value
    // and the target label.
    instructions->emplace_back(
        std::make_shared<IR::JumpIfNotZeroInstruction>(condition, target));
}

void IRGenerator::generateIRLabelInstruction(
    std::string_view identifier,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Generate a label instruction with the label identifier.
    instructions->emplace_back(
        std::make_shared<IR::LabelInstruction>(identifier));
}

std::shared_ptr<IR::VariableValue>
IRGenerator::generateIRFunctionCallInstruction(
    std::string_view functionIdentifier,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Value>>> &arguments,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Create a temporary variable (in string) to store the result of
    // the function call.
    auto tmpName = generateIRTemporary();
    // Create a variable value for the temporary variable.
    auto dst = std::make_shared<IR::VariableValue>(tmpName);
    // Generate a function call instruction with the function identifier,
    // the arguments, and the destination value.
    instructions->emplace_back(std::make_shared<IR::FunctionCallInstruction>(
        functionIdentifier, arguments, dst));
    // Return the destination value.
    return dst;
}

std::shared_ptr<IR::VariableValue> IRGenerator::generateIRCastInstruction(
    const std::shared_ptr<AST::CastExpression> &castExpr,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &instructions) {
    // Recursively generate the expression in the cast expression.
    auto result =
        generateIRInstruction(castExpr->getExpression(), instructions);
    // If the target type is the same as the expression type, return the
    // result as a `VariableValue`.
    if (*castExpr->getTargetType() == *castExpr->getExpType()) {
        if (auto variableValue =
                std::dynamic_pointer_cast<IR::VariableValue>(result)) {
            return variableValue;
        }
        // TODO(zzmic): Check if this is correct.
        else if (auto constantValue =
                     std::dynamic_pointer_cast<IR::ConstantValue>(result)) {
            auto dstName = generateIRTemporary();
            symbols[dstName] =
                std::make_pair(castExpr->getTargetType(),
                               std::make_shared<AST::LocalAttribute>());
            auto dst = std::make_shared<IR::VariableValue>(dstName);
            instructions->emplace_back(
                std::make_shared<IR::CopyInstruction>(std::move(result), dst));
            return dst;
        }
        else {
            throw std::logic_error(
                "Unknown result value type in cast instruction");
        }
    }
    // Create a temporary variable (in string) to store the result of
    // the cast operation.
    auto dstName = generateIRTemporary();
    // Create a new entry in the symbol table for the casted variable.
    symbols[dstName] = std::make_pair(castExpr->getTargetType(),
                                      std::make_shared<AST::LocalAttribute>());
    // Create a variable value for the temporary variable.
    auto dst = std::make_shared<IR::VariableValue>(dstName);
    // If the target type is a long type, generate a sign extend instruction.
    // Otherwise, generate a truncate instruction.
    if (std::dynamic_pointer_cast<AST::LongType>(castExpr->getTargetType())) {
        instructions->emplace_back(
            std::make_shared<IR::SignExtendInstruction>(result, dst));
    }
    else {
        instructions->emplace_back(
            std::make_shared<IR::TruncateInstruction>(result, dst));
    }
    // Return the destination value.
    return dst;
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
    static int counter = 0;
    return "or_true" + std::to_string(counter++);
}

std::string IRGenerator::generateIRResultLabel() {
    static int counter = 0;
    return "result" + std::to_string(counter++);
}

std::string IRGenerator::generateIREndLabel() {
    static int counter = 0;
    return "end" + std::to_string(counter++);
}

std::string IRGenerator::generateIRElseLabel() {
    static int counter = 0;
    return "else" + std::to_string(counter++);
}

std::string IRGenerator::generateIRE2Label() {
    static int counter = 0;
    return "e2" + std::to_string(counter++);
}

std::string IRGenerator::generateIRContinueLoopLabel(
    std::string_view loopLabelingLabel) const {
    return "continue_" + std::string(loopLabelingLabel);
}

std::string IRGenerator::generateIRBreakLoopLabel(
    std::string_view loopLabelingLabel) const {
    return "break_" + std::string(loopLabelingLabel);
}

std::string IRGenerator::generateIRStartLabel() const {
    static int counter = 0;
    return "start" + std::to_string(counter++);
}

std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
IRGenerator::convertSymbolsToIRStaticVariables() {
    auto irDefs =
        std::make_shared<std::vector<std::shared_ptr<IR::StaticVariable>>>();
    for (const auto &symbol : symbols) {
        auto name = symbol.first;
        auto type = symbol.second.first;
        auto attribute = symbol.second.second;
        if (auto staticAttribute =
                std::dynamic_pointer_cast<AST::StaticAttribute>(attribute)) {
            auto initialValue = staticAttribute->getInitialValue();
            auto global = staticAttribute->isGlobal();
            if (auto initial =
                    std::dynamic_pointer_cast<AST::Initial>(initialValue)) {
                auto valueVariant = initial->getStaticInit()->getValue();
                if (std::holds_alternative<int>(valueVariant)) {
                    irDefs->emplace_back(std::make_shared<StaticVariable>(
                        name, global, type,
                        std::make_shared<AST::IntInit>(
                            std::get<int>(valueVariant))));
                }
                else if (std::holds_alternative<long>(valueVariant)) {
                    irDefs->emplace_back(std::make_shared<StaticVariable>(
                        name, global, type,
                        std::make_shared<AST::LongInit>(
                            std::get<long>(valueVariant))));
                }
                else {
                    throw std::logic_error(
                        "Unsupported static initializer type while converting "
                        "symbols to IR static variables");
                }
            }
            else if (auto tentative = std::dynamic_pointer_cast<AST::Tentative>(
                         initialValue)) {
                if (std::dynamic_pointer_cast<AST::IntType>(type)) {
                    irDefs->emplace_back(std::make_shared<StaticVariable>(
                        name, global, type, std::make_shared<AST::IntInit>(0)));
                }
                else if (std::dynamic_pointer_cast<AST::LongType>(type)) {
                    irDefs->emplace_back(std::make_shared<StaticVariable>(
                        name, global, type,
                        std::make_shared<AST::LongInit>(0L)));
                }
                else {
                    throw std::logic_error(
                        "Unsupported tentative type while converting symbols "
                        "to IR static variables");
                }
            }
            else if (std::dynamic_pointer_cast<AST::NoInitializer>(
                         initialValue)) {
                continue;
            }
            else {
                throw std::logic_error(
                    "Unsupported initial value type while converting symbols "
                    "to IR static variables");
            }
        }
        else {
            continue;
        }
    }
    return irDefs;
}

std::shared_ptr<IR::UnaryOperator>
IRGenerator::convertUnop(const std::shared_ptr<AST::UnaryOperator> &op) {
    if (std::dynamic_pointer_cast<AST::NegateOperator>(op)) {
        return std::make_shared<IR::NegateOperator>();
    }
    else if (std::dynamic_pointer_cast<AST::ComplementOperator>(op)) {
        return std::make_shared<IR::ComplementOperator>();
    }
    else if (std::dynamic_pointer_cast<AST::NotOperator>(op)) {
        return std::make_shared<IR::NotOperator>();
    }
    throw std::logic_error("Unsupported unary operator while converting "
                           "unary operator to IR unary operator");
}

// Note: The logical-and and logical-or operators in the AST are NOT binary
// operators in the IR (and should NOT be converted to binary operators in
// the IR).
std::shared_ptr<IR::BinaryOperator>
IRGenerator::convertBinop(const std::shared_ptr<AST::BinaryOperator> &op) {
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
    throw std::logic_error("Unsupported binary operator while converting "
                           "binary operator to IR binary operator");
}

std::shared_ptr<IR::VariableValue> IRGenerator::generateIRVariable(
    const std::shared_ptr<AST::BinaryExpression> &binaryExpr) {
    // Create a temporary variable (in string) to store the result of
    // the binary operation.
    auto tmpName = generateIRTemporary();

    // Add the temporary variable to the symbols table with the appropriate type
    // and local attribute.
    symbols[tmpName] = std::make_pair(binaryExpr->getExpType(),
                                      std::make_shared<AST::LocalAttribute>());

    // Create a variable value for the temporary variable and return it.
    return std::make_shared<IR::VariableValue>(tmpName);
}
} // namespace IR
