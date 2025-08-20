#include "irGenerator.h"
#include "../frontend/frontendSymbolTable.h"
#include <memory>

namespace IR {
IRGenerator::IRGenerator(int variableResolutionCounter)
    : irTemporariesCounter(variableResolutionCounter) {}

std::pair<std::unique_ptr<IR::Program>,
          std::vector<std::unique_ptr<IR::StaticVariable>>>
IRGenerator::generateIR(const std::unique_ptr<AST::Program> &astProgram) {
    // Initialize the vector of IR top-levels (top-level elements).
    std::vector<std::unique_ptr<IR::TopLevel>> topLevels;

    // Generate IR instructions for each AST top-level element.
    auto astDeclarations = astProgram->getDeclarations();
    for (auto &astDeclaration : astDeclarations) {
        if (dynamic_cast<AST::FunctionDeclaration *>(astDeclaration.get())) {
            auto functionDeclaration =
                std::unique_ptr<AST::FunctionDeclaration>(
                    static_cast<AST::FunctionDeclaration *>(
                        astDeclaration.release()));

            // Get the body of the function declaration.
            auto optBody = functionDeclaration->getOptBody();

            // Skip generating IR instructions for forward declarations.
            if (!optBody.has_value()) {
                continue;
            }

            // Create a new vector of IR instructions for the function.
            std::vector<std::unique_ptr<IR::Instruction>> instructions;

            // Get the identifier and the parameters of the function
            // declaration.
            auto identifier = functionDeclaration->getIdentifier();
            auto parameters = functionDeclaration->getParameterIdentifiers();

            // Find the global attribute of the function declaration in the
            // frontend symbol table and set the global flag.
            bool global = false;
            if (AST::frontendSymbolTable.find(identifier) !=
                AST::frontendSymbolTable.end()) {
                if (dynamic_cast<AST::FunctionAttribute *>(
                        AST::frontendSymbolTable[identifier].second.get())) {
                    auto functionAttributePtr =
                        std::unique_ptr<AST::FunctionAttribute>(
                            static_cast<AST::FunctionAttribute *>(
                                AST::frontendSymbolTable[identifier]
                                    .second.release()));
                    global = functionAttributePtr->isGlobal();
                }
                else {
                    throw std::logic_error(
                        "Function attribute not found in frontendSymbolTable "
                        "while "
                        "generating IR instructions for function definition");
                }
            }
            else {
                throw std::logic_error(
                    "Function declaration not found in frontendSymbolTable "
                    "while "
                    "generating IR instructions for function definition");
            }

            // Generate IR instructions for the function body.
            generateIRBlock(optBody.value(), instructions);

            // Check if the function has any return statements.
            bool hasReturnStatement = false;
            for (const auto &instruction : instructions) {
                if (dynamic_cast<IR::ReturnInstruction *>(instruction.get())) {
                    hasReturnStatement = true;
                    break;
                }
            }

            // Check if the function needs an implicit return at the end, which
            // happens when there are return statements but not all code paths
            // return.
            bool needsImplicitReturn = false;
            if (hasReturnStatement) {
                // Check if the last instruction is a return statement.
                if (instructions.empty() ||
                    !dynamic_cast<IR::ReturnInstruction *>(
                        instructions.back().get())) {
                    needsImplicitReturn = true;
                }
            }
            else {
                // No return statements at all, definitely need implicit return.
                needsImplicitReturn = true;
            }

            // If the function needs an implicit return, add it.
            if (needsImplicitReturn) {
                // Get the function's return type from the symbol table.
                auto functionType =
                    std::move(AST::frontendSymbolTable[identifier].first);
                auto functionTypePtr =
                    dynamic_cast<AST::FunctionType *>(functionType.get());
                if (functionTypePtr) {
                    auto returnType =
                        std::move(functionTypePtr->getReturnType());

                    // Create a constant value based on the return type.
                    std::unique_ptr<IR::Value> returnValue;
                    if (dynamic_cast<AST::IntType *>(returnType.get())) {
                        returnValue = std::make_unique<IR::ConstantValue>(
                            std::make_unique<AST::ConstantInt>(0));
                    }
                    else if (dynamic_cast<AST::LongType *>(returnType.get())) {
                        returnValue = std::make_unique<IR::ConstantValue>(
                            std::make_unique<AST::ConstantLong>(0L));
                    }
                    else {
                        // For void functions, we don't need to return anything,
                        // but we still need a return instruction for proper
                        // function termination.
                        returnValue = std::make_unique<IR::ConstantValue>(
                            std::make_unique<AST::ConstantInt>(0));
                    }

                    // Add the implicit return instruction.
                    instructions.emplace_back(
                        std::make_unique<IR::ReturnInstruction>(
                            std::move(returnValue)));
                }
            }

            // Create a new IR function definition with the function identifier,
            // the global flag, the parameters, and the instructions.
            auto irFunctionDefinition =
                std::make_unique<IR::FunctionDefinition>(
                    identifier, global, std::move(parameters),
                    std::move(instructions));

            // Add the IR function definition to the vector of IR top-levels.
            topLevels.emplace_back(std::move(irFunctionDefinition));
        }
        else if (auto variableDeclaration =
                     dynamic_cast<AST::VariableDeclaration *>(
                         astDeclaration.get())) {
            // Continue: Do not generate IR instructions for file-scope variable
            // declarations or for local variable declarations with `static` or
            // `extern` storage-class specifiers (for now).
            if (variableDeclaration->getOptStorageClass().has_value()) {
                if (dynamic_cast<AST::StaticStorageClass *>(
                        variableDeclaration->getOptStorageClass()
                            .value()
                            .get()) ||
                    dynamic_cast<AST::ExternStorageClass *>(
                        variableDeclaration->getOptStorageClass()
                            .value()
                            .get())) {
                    continue;
                }
            }
        }
    }

    // Examine every entry in the symbol table and generate `StaticVariable` IR
    // constructs for some of them after traversing the AST.
    auto irStaticVariables = convertFrontendSymbolTableToIRStaticVariables();

    // Return the generated IR program along with the static variables.
    return std::make_pair(std::make_unique<IR::Program>(std::move(topLevels)),
                          std::move(irStaticVariables));
}

void IRGenerator::generateIRBlock(
    AST::Block *astBlock,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Get the block items from the block.
    auto blockItems = astBlock->getBlockItems();

    // Generate IR instructions for each block item.
    for (const auto &blockItem : blockItems) {
        // If the block item is a `DBlockItem` (i.e., a declaration), ...
        if (auto dBlockItem =
                dynamic_cast<AST::DBlockItem *>(blockItem.get())) {

            // Generate IR instructions for the variable declaration (that has
            // an initializer).
            if (auto variableDeclaration =
                    dynamic_cast<AST::VariableDeclaration *>(
                        dBlockItem->getDeclaration().get())) {
                auto variableDeclarationPtr =
                    std::unique_ptr<AST::VariableDeclaration>(
                        static_cast<AST::VariableDeclaration *>(
                            dBlockItem->getDeclaration().release()));

                // Continue: Do not generate IR instructions for file-scope
                // variable declarations or for local variable declarations with
                // `static` or `extern` storage-class specifiers (for now).
                if (variableDeclaration->getOptStorageClass().has_value()) {
                    if (dynamic_cast<AST::StaticStorageClass *>(
                            variableDeclaration->getOptStorageClass()
                                .value()
                                .get()) ||
                        dynamic_cast<AST::ExternStorageClass *>(
                            variableDeclaration->getOptStorageClass()
                                .value()
                                .get())) {
                        continue;
                    }
                }
                else {
                    generateIRVariableDefinition(variableDeclarationPtr,
                                                 instructions);
                }
            }

            // Generate IR instructions for the function declaration (that has a
            // body).
            else if (dynamic_cast<AST::FunctionDeclaration *>(
                         dBlockItem->getDeclaration().get())) {
                auto functionDeclarationPtr =
                    std::unique_ptr<AST::FunctionDeclaration>(
                        static_cast<AST::FunctionDeclaration *>(
                            dBlockItem->getDeclaration().release()));
                generateIRFunctionDefinition(functionDeclarationPtr,
                                             instructions);
            }
        }

        // If the block item is a `SBockItem` (i.e., a statement), ...
        else if (auto sBlockItem =
                     dynamic_cast<AST::SBlockItem *>(blockItem.get())) {
            // Generate IR instructions for the statement.
            generateIRStatement(sBlockItem->getStatement(), instructions);
        }
    }
}

void IRGenerator::generateIRFunctionDefinition(
    std::unique_ptr<AST::FunctionDeclaration> &astFunctionDeclaration,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Get the body of the function.
    auto optBody = astFunctionDeclaration->getOptBody();
    if (optBody.has_value()) {
        // Generate IR instructions for the body of the function.
        generateIRBlock(optBody.value(), instructions);
    }
    // Otherwise (i.e., if the function does not have a body), we do not need
    // to generate any IR instructions (for now).
}

void IRGenerator::generateIRVariableDefinition(
    std::unique_ptr<AST::VariableDeclaration> &astVariableDeclaration,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Get the identifier and the initializer of the variable declaration.
    auto identifier = astVariableDeclaration->getIdentifier();
    auto initializer = std::move(astVariableDeclaration->getOptInitializer());

    // If the declaration has an initializer, ...
    if (initializer.has_value()) {
        // Generate IR instructions for the initializer.
        auto result = generateIRInstruction(initializer.value(), instructions);

        // Create a variable value for the identifier.
        auto dst = std::make_unique<IR::VariableValue>(identifier);

        // Generate a copy instruction with the result value and the
        // destination value.
        instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
            std::move(result), std::move(dst)));
    }
    // Otherwise (i.e., if the declaration does not have an initializer),
    // we do not need to generate any IR instructions.
}

void IRGenerator::generateIRStatement(
    std::unique_ptr<AST::Statement> &astStatement,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    if (auto returnStmt =
            dynamic_cast<AST::ReturnStatement *>(astStatement.get())) {
        auto returnStmtPtr = std::unique_ptr<AST::ReturnStatement>(
            static_cast<AST::ReturnStatement *>(astStatement.release()));
        generateIRReturnStatement(returnStmtPtr, instructions);
    }
    else if (auto expressionStmt =
                 dynamic_cast<AST::ExpressionStatement *>(astStatement.get())) {
        auto expressionStmtPtr = std::unique_ptr<AST::ExpressionStatement>(
            static_cast<AST::ExpressionStatement *>(astStatement.release()));
        generateIRExpressionStatement(expressionStmtPtr, instructions);
    }
    else if (auto compoundStmt =
                 dynamic_cast<AST::CompoundStatement *>(astStatement.get())) {
        // If the statement is a compound statement, generate a block.
        generateIRBlock(compoundStmt->getBlock(), instructions);
    }
    else if (auto ifStmt =
                 dynamic_cast<AST::IfStatement *>(astStatement.get())) {
        auto ifStmtPtr = std::unique_ptr<AST::IfStatement>(
            static_cast<AST::IfStatement *>(astStatement.release()));
        generateIRIfStatement(ifStmtPtr, instructions);
    }
    else if (auto breakStmt =
                 dynamic_cast<AST::BreakStatement *>(astStatement.get())) {
        auto breakStmtPtr = std::unique_ptr<AST::BreakStatement>(
            static_cast<AST::BreakStatement *>(astStatement.release()));
        generateIRBreakStatement(breakStmtPtr, instructions);
    }
    else if (auto continueStmt =
                 dynamic_cast<AST::ContinueStatement *>(astStatement.get())) {
        auto continueStmtPtr = std::unique_ptr<AST::ContinueStatement>(
            static_cast<AST::ContinueStatement *>(astStatement.release()));
        generateIRContinueStatement(continueStmtPtr, instructions);
    }
    else if (auto whileStmt =
                 dynamic_cast<AST::WhileStatement *>(astStatement.get())) {
        auto whileStmtPtr = std::unique_ptr<AST::WhileStatement>(
            static_cast<AST::WhileStatement *>(astStatement.release()));
        generateIRWhileStatement(whileStmtPtr, instructions);
    }
    else if (auto doWhileStmt =
                 dynamic_cast<AST::DoWhileStatement *>(astStatement.get())) {
        auto doWhileStmtPtr = std::unique_ptr<AST::DoWhileStatement>(
            static_cast<AST::DoWhileStatement *>(astStatement.release()));
        generateIRDoWhileStatement(doWhileStmtPtr, instructions);
    }
    else if (auto forStmt =
                 dynamic_cast<AST::ForStatement *>(astStatement.get())) {
        auto forStmtPtr = std::unique_ptr<AST::ForStatement>(
            static_cast<AST::ForStatement *>(astStatement.release()));
        generateIRForStatement(forStmtPtr, instructions);
    }
    else if (auto nullStmt =
                 dynamic_cast<AST::NullStatement *>(astStatement.get())) {
        // If the statement is a null statement, do nothing.
    }
    else {
        throw std::logic_error("Unsupported statement type while generating "
                               "IR instructions for statement");
    }
}

void IRGenerator::generateIRReturnStatement(
    std::unique_ptr<AST::ReturnStatement> &returnStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Get the expression from the return statement.
    auto exp = std::move(returnStmt->getExpression());

    // Process the expression and generate the corresponding IR
    // instructions.
    auto result = generateIRInstruction(exp, instructions);
    instructions.emplace_back(
        std::make_unique<IR::ReturnInstruction>(std::move(result)));
}

void IRGenerator::generateIRExpressionStatement(
    std::unique_ptr<AST::ExpressionStatement> &expressionStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Get the expression from the expression statement.
    auto exp = std::move(expressionStmt->getExpression());

    // Process the expression and generate the corresponding IR
    // instructions.
    auto result = generateIRInstruction(exp, instructions);
    instructions.emplace_back(std::move(result));
}

void IRGenerator::generateIRIfStatement(
    std::unique_ptr<AST::IfStatement> &ifStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Get the condition from the if-statement.
    auto condition = std::move(ifStmt->getCondition());
    // Process the condition and generate the corresponding IR instructions.
    auto conditionValue = generateIRInstruction(condition, instructions);
    // Generate a new end label.
    auto endLabel = generateIREndLabel();
    // Generate a new else label.
    auto elseLabel = generateIRElseLabel();

    if (ifStmt->getElseOptStatement().has_value()) {
        // Generate a jump-if-zero instruction with the condition value and the
        // else label.
        instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
            std::move(conditionValue), elseLabel));
        // Get the then-statement from the if-statement.
        auto thenStatement = std::move(ifStmt->getThenStatement());
        // Process the then-statement and generate the corresponding IR
        // instructions.
        generateIRStatement(thenStatement, instructions);
        // Generate a jump instruction with the end label.
        instructions.emplace_back(
            std::make_unique<IR::JumpInstruction>(endLabel));
        // Generate a label instruction with the same (new) else label.
        instructions.emplace_back(
            std::make_unique<IR::LabelInstruction>(elseLabel));
        // Get the (optional) else-statement from the if-statement.
        auto elseOptStatement =
            std::move(ifStmt->getElseOptStatement().value());
        // Process the else-statement and generate the corresponding IR
        // instructions.
        generateIRStatement(elseOptStatement, instructions);
    }
    else {
        // Generate a jump-if-zero instruction with the condition value and the
        // end label.
        instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
            std::move(conditionValue), endLabel));
        // Get the then-statement from the if-statement.
        auto thenStatement = std::move(ifStmt->getThenStatement());
        // Process the then-statement and generate the corresponding IR
        // instructions.
        generateIRStatement(thenStatement, instructions);
    }

    // Generate a label instruction with the same (new) end label.
    instructions.emplace_back(std::make_unique<IR::LabelInstruction>(endLabel));
}

void IRGenerator::generateIRBreakStatement(
    std::unique_ptr<AST::BreakStatement> &breakStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Generate a jump instruction with the extended break label.
    auto breakLabel = generateIRBreakLoopLabel(breakStmt->getLabel());
    instructions.emplace_back(
        std::make_unique<IR::JumpInstruction>(breakLabel));
}

void IRGenerator::generateIRContinueStatement(
    std::unique_ptr<AST::ContinueStatement> &continueStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Generate a jump instruction with the extended continue label.
    auto continueLabel = generateIRContinueLoopLabel(continueStmt->getLabel());
    instructions.emplace_back(
        std::make_unique<IR::JumpInstruction>(continueLabel));
}

void IRGenerator::generateIRDoWhileStatement(
    std::unique_ptr<AST::DoWhileStatement> &doWhileStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Generate a new start label.
    auto startLabel = generateIRStartLabel();
    // Generate a label instruction with the start label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(startLabel));
    // Generate instructions for the body of the do-while-statement.
    auto body = std::move(doWhileStmt->getBody());
    generateIRStatement(body, instructions);
    // Generate a new continue label (based on the label of the do-while).
    auto continueLabel = generateIRContinueLoopLabel(doWhileStmt->getLabel());
    // Generate a label instruction with the continue label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(continueLabel));
    // Generate instructions for the condition of the do-while-statement.
    auto condition = std::move(doWhileStmt->getCondition());
    auto conditionValue = generateIRInstruction(condition, instructions);
    // Generate a jump-if-not-zero instruction with the condition value and the
    // start label.
    instructions.emplace_back(std::make_unique<IR::JumpIfNotZeroInstruction>(
        std::move(conditionValue), startLabel));
    // Generate a new break label (based on the label of the do-while).
    auto breakLabel = generateIRBreakLoopLabel(doWhileStmt->getLabel());
    // Generate a label instruction with the break label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(breakLabel));
}

void IRGenerator::generateIRWhileStatement(
    std::unique_ptr<AST::WhileStatement> &whileStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Generate a new continue label (based on the label of the while).
    auto continueLabel = generateIRContinueLoopLabel(whileStmt->getLabel());
    // Generate a label instruction with the continue label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(continueLabel));
    // Generate instructions for the condition of the while-statement.
    auto condition = std::move(whileStmt->getCondition());
    auto conditionValue = generateIRInstruction(condition, instructions);
    // Generate a new break label (based on the label of the while).
    auto breakLabel = generateIRBreakLoopLabel(whileStmt->getLabel());
    // Generate a jump-if-zero instruction with the condition value and the
    // break label.
    instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
        std::move(conditionValue), breakLabel));
    // Generate instructions for the body of the while-statement.
    auto body = std::move(whileStmt->getBody());
    generateIRStatement(body, instructions);
    // Generate a jump instruction with the continue label.
    instructions.emplace_back(
        std::make_unique<IR::JumpInstruction>(continueLabel));
    // Generate a label instruction with the break label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(breakLabel));
}

void IRGenerator::generateIRForStatement(
    std::unique_ptr<AST::ForStatement> &forStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Generate instructions for the for-init of the for-statement.
    auto forInit = std::move(forStmt->getForInit());
    if (dynamic_cast<AST::InitExpr *>(forInit.get())) {
        auto initExprPtr = std::unique_ptr<AST::InitExpr>(
            static_cast<AST::InitExpr *>(forInit.release()));
        auto optExpr = std::move(initExprPtr->getExpression());
        if (optExpr.has_value()) {
            auto resolvedExpr =
                generateIRInstruction(optExpr.value(), instructions);
            instructions.emplace_back(std::move(resolvedExpr));
        }
    }
    else if (dynamic_cast<AST::InitDecl *>(forInit.get())) {
        auto initDeclPtr = std::unique_ptr<AST::InitDecl>(
            static_cast<AST::InitDecl *>(forInit.release()));
        auto variableDeclaration =
            std::move(initDeclPtr->getVariableDeclaration());
        generateIRVariableDefinition(variableDeclaration, instructions);
    }
    // Generate a new start label.
    auto startLabel = generateIRStartLabel();
    // Generate a label instruction with the start label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(startLabel));
    // Generate a new break label (based on the label of the for).
    auto breakLabel = generateIRBreakLoopLabel(forStmt->getLabel());
    // Optionally generate instructions for the (optional) condition of the
    // for-statement.
    auto optCondition = std::move(forStmt->getOptCondition());
    if (optCondition.has_value()) {
        // Generate a jump-if-zero instruction with the condition value and the
        // break label.
        auto conditionValue =
            generateIRInstruction(optCondition.value(), instructions);
        instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
            std::move(conditionValue), breakLabel));
    }
    // Generate instructions for the body of the for-statement.
    auto body = std::move(forStmt->getBody());
    generateIRStatement(body, instructions);
    // Generate a new continue label (based on the label of the for).
    auto continueLabel = generateIRContinueLoopLabel(forStmt->getLabel());
    // Generate a label instruction with the continue label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(continueLabel));
    // Optionally generate instructions for the (optional) post of the
    // for-statement.
    auto optPost = std::move(forStmt->getOptPost());
    if (optPost.has_value()) {
        auto postValue = generateIRInstruction(optPost.value(), instructions);
        instructions.emplace_back(std::move(postValue));
    }
    // Generate a jump instruction with the start label.
    instructions.emplace_back(
        std::make_unique<IR::JumpInstruction>(startLabel));
    // Generate a label instruction with the break label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(breakLabel));
}

std::unique_ptr<IR::Value> IRGenerator::generateIRInstruction(
    std::unique_ptr<AST::Expression> &e,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    if (dynamic_cast<AST::ConstantExpression *>(e.get())) {
        auto constantExprPtr = std::unique_ptr<AST::ConstantExpression>(
            static_cast<AST::ConstantExpression *>(e.release()));
        auto variantValue = constantExprPtr->getConstantInIntOrLongVariant();
        if (std::holds_alternative<int>(variantValue)) {
            return std::make_unique<IR::ConstantValue>(
                std::make_unique<AST::ConstantInt>(
                    std::get<int>(variantValue)));
        }
        else if (std::holds_alternative<long>(variantValue)) {
            return std::make_unique<IR::ConstantValue>(
                std::make_unique<AST::ConstantLong>(
                    std::get<long>(variantValue)));
        }
        else {
            throw std::logic_error(
                "Unsupported constant type while generating IR instructions "
                "for expression");
        }
    }
    else if (dynamic_cast<AST::UnaryExpression *>(e.get())) {
        auto unaryExprPtr = std::unique_ptr<AST::UnaryExpression>(
            static_cast<AST::UnaryExpression *>(e.release()));
        return generateIRUnaryInstruction(unaryExprPtr, instructions);
    }
    else if (dynamic_cast<AST::BinaryExpression *>(e.get())) {
        auto binaryExprPtr = std::unique_ptr<AST::BinaryExpression>(
            static_cast<AST::BinaryExpression *>(e.release()));
        // If the binary operator in the AST binary expression is a
        // logical-and operator, ...
        if (dynamic_cast<AST::AndOperator *>(
                binaryExprPtr->getOperator().get())) {
            return generateIRInstructionWithLogicalAnd(binaryExprPtr,
                                                       instructions);
        }
        // If the binary operator in the AST binary expression is a
        // logical-or operator, ...
        else if (dynamic_cast<AST::OrOperator *>(
                     binaryExprPtr->getOperator().get())) {
            return generateIRInstructionWithLogicalOr(binaryExprPtr,
                                                      instructions);
        }
        // Otherwise (i.e., if the binary operator in the AST binary
        // expression contains neither a logical-and nor a logical-or
        // operator), ...
        else {
            return generateIRBinaryInstruction(binaryExprPtr, instructions);
        }
    }
    else if (dynamic_cast<AST::VariableExpression *>(e.get())) {
        auto variableExprPtr = std::unique_ptr<AST::VariableExpression>(
            static_cast<AST::VariableExpression *>(e.release()));
        return std::make_unique<IR::VariableValue>(
            variableExprPtr->getIdentifier());
    }
    else if (dynamic_cast<AST::AssignmentExpression *>(e.get())) {
        auto assignmentExprPtr = std::unique_ptr<AST::AssignmentExpression>(
            static_cast<AST::AssignmentExpression *>(e.release()));
        if (dynamic_cast<AST::VariableExpression *>(
                assignmentExprPtr->getLeft().get())) {
            auto variableExprPtr = std::unique_ptr<AST::VariableExpression>(
                static_cast<AST::VariableExpression *>(
                    assignmentExprPtr->getLeft().release()));
            auto variableValue = std::make_unique<IR::VariableValue>(
                variableExprPtr->getIdentifier());
            auto result = generateIRInstruction(assignmentExprPtr->getRight(),
                                                instructions);
            instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
                std::move(result), std::move(variableValue)));
            return variableValue;
        }
        else {
            throw std::logic_error(
                "Unsupported lvalue type in assignment while generating IR "
                "instructions for expression");
        }
    }
    else if (dynamic_cast<AST::ConditionalExpression *>(e.get())) {
        auto conditionalExprPtr = std::unique_ptr<AST::ConditionalExpression>(
            static_cast<AST::ConditionalExpression *>(e.release()));
        auto conditionValue = generateIRInstruction(
            conditionalExprPtr->getCondition(), instructions);
        auto e2Label = generateIRE2Label();
        instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
            std::move(conditionValue), e2Label));
        auto e1Value = generateIRInstruction(
            conditionalExprPtr->getThenExpression(), instructions);
        auto resultLabel = generateIRResultLabel();

        // Add the result variable to the frontend symbol table with type int.
        AST::frontendSymbolTable[resultLabel] =
            std::make_pair(std::make_unique<AST::IntType>(),
                           std::make_unique<AST::LocalAttribute>());

        auto resultValue = std::make_unique<IR::VariableValue>(resultLabel);
        instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
            std::move(e1Value), std::move(resultValue)));
        auto endLabel = generateIREndLabel();
        instructions.emplace_back(
            std::make_unique<IR::JumpInstruction>(endLabel));
        instructions.emplace_back(
            std::make_unique<IR::LabelInstruction>(e2Label));
        auto e2Value = generateIRInstruction(
            conditionalExprPtr->getElseExpression(), instructions);
        instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
            std::move(e2Value), std::move(resultValue)));
        instructions.emplace_back(
            std::make_unique<IR::LabelInstruction>(endLabel));
        return resultValue;
    }
    else if (dynamic_cast<AST::FunctionCallExpression *>(e.get())) {
        auto functionCallExprPtr = std::unique_ptr<AST::FunctionCallExpression>(
            static_cast<AST::FunctionCallExpression *>(e.release()));
        auto functionIdentifier = functionCallExprPtr->getIdentifier();
        std::vector<std::unique_ptr<IR::Value>> args;
        for (auto &arg : functionCallExprPtr->getArguments()) {
            auto argValue = generateIRInstruction(arg, instructions);
            args.emplace_back(std::move(argValue));
        }
        auto resultValue = generateIRFunctionCallInstruction(
            functionIdentifier, args, instructions);
        return resultValue;
    }
    else if (dynamic_cast<AST::CastExpression *>(e.get())) {
        auto castExprPtr = std::unique_ptr<AST::CastExpression>(
            static_cast<AST::CastExpression *>(e.release()));
        return generateIRCastInstruction(castExprPtr, instructions);
    }
    throw std::logic_error("Unsupported expression type while generating IR "
                           "instructions for expression");
}

std::unique_ptr<IR::VariableValue> IRGenerator::generateIRUnaryInstruction(
    std::unique_ptr<AST::UnaryExpression> &unaryExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Recursively generate the expression in the unary expression.
    auto factorExpr = std::move(unaryExpr->getExpression());
    auto expressionExprPtr = std::unique_ptr<AST::Expression>(
        static_cast<AST::Expression *>(factorExpr.release()));
    auto src = generateIRInstruction(expressionExprPtr, instructions);

    // Create a temporary variable (in string) to store the result of
    // the unary operation.
    auto tmpName = generateIRTemporary();

    // Add the temporary variable to the frontend symbol table with the type
    // of the expression and local attribute.
    AST::frontendSymbolTable[tmpName] =
        std::make_pair(std::move(unaryExpr->getExpType()),
                       std::make_unique<AST::LocalAttribute>());

    // Create a variable value for the temporary variable.
    auto dst = std::make_unique<IR::VariableValue>(tmpName);

    // Convert the unary operator in the unary expression to a IR
    // unary operator.
    auto IROp = convertUnop(unaryExpr->getOperator());

    // Generate a unary instruction with the IR unary operator, the
    // source value, and the destination value.
    instructions.emplace_back(std::make_unique<IR::UnaryInstruction>(
        std::move(IROp), std::move(src), std::move(dst)));

    return std::make_unique<IR::VariableValue>(tmpName);
}

std::unique_ptr<IR::VariableValue> IRGenerator::generateIRBinaryInstruction(
    std::unique_ptr<AST::BinaryExpression> &binaryExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
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
    instructions.emplace_back(std::make_unique<IR::BinaryInstruction>(
        std::move(IROp), std::move(lhs), std::move(rhs), std::move(dst)));

    return std::make_unique<IR::VariableValue>(dst->getIdentifier());
}

std::unique_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalAnd(
    std::unique_ptr<AST::BinaryExpression> &binaryExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Recursively generate the left expression in the binary expression.
    auto lhs = generateIRInstruction(binaryExpr->getLeft(), instructions);

    // Generate a JumpIfZero instruction with the left-hand side value and a
    // (new) false label.
    auto falseLabel = generateIRFalseLabel();
    instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
        std::move(lhs), falseLabel));

    // Recursively generate the right expression in the binary expression.
    auto rhs = generateIRInstruction(binaryExpr->getRight(), instructions);

    // Generate a JumpIfZero instruction with the right-hand side value and
    // the same (new) false label.
    instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
        std::move(rhs), falseLabel));

    // Generate a copy instruction with 1 being copied to a (new) result
    // label.
    auto resultLabel = generateIRResultLabel();

    // Add the result variable to the frontend symbol table with type int.
    AST::frontendSymbolTable[resultLabel] =
        std::make_pair(std::make_unique<AST::IntType>(),
                       std::make_unique<AST::LocalAttribute>());

    auto dst = std::make_unique<IR::VariableValue>(resultLabel);
    instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
        std::make_unique<IR::ConstantValue>(
            std::make_unique<AST::ConstantInt>(1)),
        std::move(dst)));

    // Generate a jump instruction with a new end label.
    auto endLabel = generateIREndLabel();
    instructions.emplace_back(std::make_unique<IR::JumpInstruction>(endLabel));

    // Generate a label instruction with the same (new) false label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(falseLabel));

    // Generate a copy instruction with 0 being copied to the result.
    instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
        std::make_unique<IR::ConstantValue>(
            std::make_unique<AST::ConstantInt>(0)),
        std::make_unique<IR::VariableValue>(resultLabel)));

    // Generate a label instruction with the same (new) end label.
    instructions.emplace_back(std::make_unique<IR::LabelInstruction>(endLabel));

    return std::make_unique<IR::VariableValue>(resultLabel);
}

std::unique_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalOr(
    std::unique_ptr<AST::BinaryExpression> &binaryExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Recursively generate the left expression in the binary expression.
    auto lhs = generateIRInstruction(binaryExpr->getLeft(), instructions);

    // Generate a JumpIfNotZero instruction with the left-hand side value
    // and a (new) true label.
    auto trueLabel = generateIRTrueLabel();
    instructions.emplace_back(std::make_unique<IR::JumpIfNotZeroInstruction>(
        std::move(lhs), trueLabel));

    // Recursively generate the right expression in the binary expression.
    auto rhs = generateIRInstruction(binaryExpr->getRight(), instructions);

    // Generate a JumpIfNotZero instruction with the right-hand side value
    // and the same (new) true label.
    auto trueLabelRight = generateIRTrueLabel();
    instructions.emplace_back(std::make_unique<IR::JumpIfNotZeroInstruction>(
        std::move(rhs), trueLabel));

    // Generate a copy instruction with 0 being copied to a (new) result
    // label.
    auto resultLabel = generateIRResultLabel();

    // Add the result variable to the frontend symbol table with type int.
    AST::frontendSymbolTable[resultLabel] =
        std::make_pair(std::make_unique<AST::IntType>(),
                       std::make_unique<AST::LocalAttribute>());

    instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
        std::make_unique<IR::ConstantValue>(
            std::make_unique<AST::ConstantInt>(0)),
        std::make_unique<IR::VariableValue>(resultLabel)));

    // Generate a jump instruction with a new end label.
    auto endLabel = generateIREndLabel();
    instructions.emplace_back(std::make_unique<IR::JumpInstruction>(endLabel));

    // Generate a label instruction with the same (new) true label.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(trueLabel));

    // Generate a copy instruction with 1 being copied to the result.
    instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
        std::make_unique<IR::ConstantValue>(
            std::make_unique<AST::ConstantInt>(1)),
        std::make_unique<IR::VariableValue>(resultLabel)));

    // Generate a label instruction with the same (new) end label.
    instructions.emplace_back(std::make_unique<IR::LabelInstruction>(endLabel));

    return std::make_unique<IR::VariableValue>(resultLabel);
}

void IRGenerator::generateIRCopyInstruction(
    std::unique_ptr<IR::Value> src, std::unique_ptr<IR::Value> dst,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    instructions.emplace_back(
        std::make_unique<IR::CopyInstruction>(std::move(src), std::move(dst)));
}

void IRGenerator::generateIRJumpInstruction(
    std::string_view target,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    instructions.emplace_back(std::make_unique<IR::JumpInstruction>(target));
}

void IRGenerator::generateIRJumpIfZeroInstruction(
    std::unique_ptr<IR::Value> condition, std::string_view target,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
        std::move(condition), target));
}

void IRGenerator::generateIRJumpIfNotZeroInstruction(
    std::unique_ptr<IR::Value> condition, std::string_view target,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    instructions.emplace_back(std::make_unique<IR::JumpIfNotZeroInstruction>(
        std::move(condition), target));
}

void IRGenerator::generateIRLabelInstruction(
    std::string_view identifier,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(identifier));
}

std::unique_ptr<IR::VariableValue>
IRGenerator::generateIRFunctionCallInstruction(
    std::string_view functionIdentifier,
    std::vector<std::unique_ptr<IR::Value>> &args,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Create a temporary variable (in string) to store the result of
    // the function call.
    auto tmpName = generateIRTemporary();

    // Look up the function's return type in the frontend symbol table.
    auto functionType = std::move(
        AST::frontendSymbolTable[std::string(functionIdentifier)].first);
    auto functionTypePtr =
        dynamic_cast<AST::FunctionType *>(functionType.get());
    if (!functionTypePtr) {
        throw std::logic_error("Function type not found in symbol table: " +
                               std::string(functionIdentifier));
    }
    auto returnType = std::move(functionTypePtr->getReturnType());

    // Add the temporary variable to the frontend symbol table with the type
    // of the function's return type and local attribute.
    AST::frontendSymbolTable[tmpName] = std::make_pair(
        std::move(returnType), std::make_unique<AST::LocalAttribute>());

    // Create a variable value for the temporary variable.
    // Generate a function call instruction with the function identifier,
    // the arguments, and the destination value.
    instructions.emplace_back(std::make_unique<IR::FunctionCallInstruction>(
        functionIdentifier, std::move(args),
        std::make_unique<IR::VariableValue>(tmpName)));
    // Return the destination value.
    return std::make_unique<IR::VariableValue>(tmpName);
}

std::unique_ptr<IR::VariableValue> IRGenerator::generateIRCastInstruction(
    std::unique_ptr<AST::CastExpression> &castExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {

    // Recursively generate the expression in the cast expression.
    auto result =
        generateIRInstruction(castExpr->getExpression(), instructions);
    auto sourceType = std::move(castExpr->getExpression()->getExpType());
    auto targetType = std::move(castExpr->getTargetType());
    // If the target type is the same as the source type, just copy or forward
    // the value.
    if (*targetType == *sourceType) {
        if (dynamic_cast<IR::VariableValue *>(result.get())) {
            auto variableValuePtr = std::unique_ptr<IR::VariableValue>(
                static_cast<IR::VariableValue *>(result.release()));
            return variableValuePtr;
        }
        else if (dynamic_cast<IR::ConstantValue *>(result.get())) {
            auto constantValuePtr = std::unique_ptr<IR::ConstantValue>(
                static_cast<IR::ConstantValue *>(result.release()));
            auto dstName = generateIRTemporary();
            AST::frontendSymbolTable[dstName] = std::make_pair(
                std::move(targetType), std::make_unique<AST::LocalAttribute>());
            instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
                std::move(result),
                std::make_unique<IR::VariableValue>(dstName)));
            return std::make_unique<IR::VariableValue>(dstName);
        }
        else {
            throw std::logic_error(
                "Unknown result value type in cast instruction");
        }
    }
    // Create a temporary variable (in string) to store the result of
    // the cast operation.
    auto dstName = generateIRTemporary();
    // Add the temporary variable to the frontend symbol table with the type
    // of the target type and local attribute.
    AST::frontendSymbolTable[dstName] = std::make_pair(
        std::move(targetType), std::make_unique<AST::LocalAttribute>());
    // Create a variable value for the temporary variable.
    // Check the types and emit the correct instruction.
    if (dynamic_cast<AST::IntType *>(sourceType.get()) &&
        dynamic_cast<AST::LongType *>(targetType.get())) {
        // Casting from int to long.
        instructions.emplace_back(std::make_unique<IR::SignExtendInstruction>(
            std::move(result), std::make_unique<IR::VariableValue>(dstName)));
    }
    else if (dynamic_cast<AST::LongType *>(sourceType.get()) &&
             dynamic_cast<AST::IntType *>(targetType.get())) {
        // Casting from long to int.
        instructions.emplace_back(std::make_unique<IR::TruncateInstruction>(
            std::move(result), std::make_unique<IR::VariableValue>(dstName)));
    }
    else {
        // Fallback to copy if types are not int/long.
        instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
            std::move(result), std::make_unique<IR::VariableValue>(dstName)));
    }
    return std::make_unique<IR::VariableValue>(dstName);
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

std::vector<std::unique_ptr<IR::StaticVariable>>
IRGenerator::convertFrontendSymbolTableToIRStaticVariables() {
    std::vector<std::unique_ptr<IR::StaticVariable>> irDefs;
    for (auto &symbol : AST::frontendSymbolTable) {
        auto name = symbol.first;
        auto type = std::move(symbol.second.first);
        auto attribute = std::move(symbol.second.second);
        if (dynamic_cast<AST::StaticAttribute *>(attribute.get())) {
            auto staticAttributePtr = std::unique_ptr<AST::StaticAttribute>(
                static_cast<AST::StaticAttribute *>(attribute.release()));
            auto initialValue =
                std::move(staticAttributePtr->getInitialValue());
            auto global = staticAttributePtr->isGlobal();
            if (dynamic_cast<AST::Initial *>(initialValue.get())) {
                auto initialPtr = std::move(std::unique_ptr<AST::Initial>(
                    static_cast<AST::Initial *>(initialValue.release())));
                auto valueVariant = initialPtr->getStaticInit()->getValue();
                if (std::holds_alternative<int>(valueVariant)) {
                    irDefs.emplace_back(std::make_unique<StaticVariable>(
                        name, global, std::move(type),
                        std::make_unique<AST::IntInit>(
                            std::get<int>(valueVariant))));
                }
                else if (std::holds_alternative<long>(valueVariant)) {
                    irDefs.emplace_back(std::make_unique<StaticVariable>(
                        name, global, std::move(type),
                        std::make_unique<AST::LongInit>(
                            std::get<long>(valueVariant))));
                }
                else {
                    throw std::logic_error(
                        "Unsupported static initializer type while converting "
                        "frontendSymbolTable to IR static variables");
                }
            }
            else if (dynamic_cast<AST::Tentative *>(initialValue.get())) {
                if (dynamic_cast<AST::IntType *>(type.get())) {
                    irDefs.emplace_back(std::make_unique<StaticVariable>(
                        name, global, std::move(type),
                        std::make_unique<AST::IntInit>(0)));
                }
                else if (dynamic_cast<AST::LongType *>(type.get())) {
                    irDefs.emplace_back(std::make_unique<StaticVariable>(
                        name, global, std::move(type),
                        std::make_unique<AST::LongInit>(0L)));
                }
                else {
                    throw std::logic_error("Unsupported tentative type while "
                                           "converting frontendSymbolTable "
                                           "to IR static variables");
                }
            }
            else if (dynamic_cast<AST::NoInitializer *>(initialValue.get())) {
                continue;
            }
            else {
                throw std::logic_error("Unsupported initial value type while "
                                       "converting frontendSymbolTable "
                                       "to IR static variables");
            }
        }
        else {
            continue;
        }
    }
    return irDefs;
}

std::unique_ptr<IR::UnaryOperator>
IRGenerator::convertUnop(std::unique_ptr<AST::UnaryOperator> &op) {
    if (dynamic_cast<AST::NegateOperator *>(op.get())) {
        return std::make_unique<IR::NegateOperator>();
    }
    else if (dynamic_cast<AST::ComplementOperator *>(op.get())) {
        return std::make_unique<IR::ComplementOperator>();
    }
    else if (dynamic_cast<AST::NotOperator *>(op.get())) {
        return std::make_unique<IR::NotOperator>();
    }
    throw std::logic_error("Unsupported unary operator while converting "
                           "unary operator to IR unary operator");
}

// Note: The logical-and and logical-or operators in the AST are NOT binary
// operators in the IR (and should NOT be converted to binary operators in
// the IR).
std::unique_ptr<IR::BinaryOperator>
IRGenerator::convertBinop(std::unique_ptr<AST::BinaryOperator> &op) {
    if (dynamic_cast<AST::AddOperator *>(op.get())) {
        return std::make_unique<IR::AddOperator>();
    }
    else if (dynamic_cast<AST::SubtractOperator *>(op.get())) {
        return std::make_unique<IR::SubtractOperator>();
    }
    else if (dynamic_cast<AST::MultiplyOperator *>(op.get())) {
        return std::make_unique<IR::MultiplyOperator>();
    }
    else if (dynamic_cast<AST::DivideOperator *>(op.get())) {
        return std::make_unique<IR::DivideOperator>();
    }
    else if (dynamic_cast<AST::RemainderOperator *>(op.get())) {
        return std::make_unique<IR::RemainderOperator>();
    }
    else if (dynamic_cast<AST::EqualOperator *>(op.get())) {
        return std::make_unique<IR::EqualOperator>();
    }
    else if (dynamic_cast<AST::NotEqualOperator *>(op.get())) {
        return std::make_unique<IR::NotEqualOperator>();
    }
    else if (dynamic_cast<AST::LessThanOperator *>(op.get())) {
        return std::make_unique<IR::LessThanOperator>();
    }
    else if (dynamic_cast<AST::LessThanOrEqualOperator *>(op.get())) {
        return std::make_unique<IR::LessThanOrEqualOperator>();
    }
    else if (dynamic_cast<AST::GreaterThanOperator *>(op.get())) {
        return std::make_unique<IR::GreaterThanOperator>();
    }
    else if (dynamic_cast<AST::GreaterThanOrEqualOperator *>(op.get())) {
        return std::make_unique<IR::GreaterThanOrEqualOperator>();
    }
    throw std::logic_error("Unsupported binary operator while converting "
                           "binary operator to IR binary operator");
}

std::unique_ptr<IR::VariableValue> IRGenerator::generateIRVariable(
    std::unique_ptr<AST::BinaryExpression> &binaryExpr) {
    // Create a temporary variable (in string) to store the result of
    // the binary operation.
    auto tmpName = generateIRTemporary();

    // Add the temporary variable to the frontend symbol table with the
    // appropriate type and local attribute.
    AST::frontendSymbolTable[tmpName] =
        std::make_pair(std::move(binaryExpr->getExpType()),
                       std::make_unique<AST::LocalAttribute>());

    // Create a variable value for the temporary variable and return it.
    return std::make_unique<IR::VariableValue>(tmpName);
}
} // namespace IR
