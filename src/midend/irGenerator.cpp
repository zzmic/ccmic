#include "irGenerator.h"
#include "../frontend/blockItem.h"
#include "../frontend/constant.h"
#include "../frontend/declaration.h"
#include "../frontend/expression.h"
#include "../frontend/forInit.h"
#include "../frontend/frontendSymbolTable.h"
#include "../frontend/operator.h"
#include "../frontend/program.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/statement.h"
#include "../frontend/storageClass.h"
#include "../frontend/type.h"
#include "ir.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

/**
 * Unnamed namespace for helper functions for the IR generator.
 */
namespace {
/**
 * Clone a type.
 *
 * @param type The type to clone.
 * @return The cloned type.
 */
std::unique_ptr<AST::Type> cloneType(const AST::Type *type) {
    if (!type) {
        return nullptr;
    }
    if (dynamic_cast<const AST::IntType *>(type)) {
        return std::make_unique<AST::IntType>();
    }
    else if (dynamic_cast<const AST::LongType *>(type)) {
        return std::make_unique<AST::LongType>();
    }
    else if (dynamic_cast<const AST::UIntType *>(type)) {
        return std::make_unique<AST::UIntType>();
    }
    else if (dynamic_cast<const AST::ULongType *>(type)) {
        return std::make_unique<AST::ULongType>();
    }
    else if (auto functionType =
                 dynamic_cast<const AST::FunctionType *>(type)) {
        auto parameterTypes =
            std::make_unique<std::vector<std::unique_ptr<AST::Type>>>();
        parameterTypes->reserve(functionType->getParameterTypes().size());
        for (const auto &parameter : functionType->getParameterTypes()) {
            parameterTypes->emplace_back(cloneType(parameter.get()));
        }
        return std::make_unique<AST::FunctionType>(
            std::move(parameterTypes),
            cloneType(&functionType->getReturnType()));
    }
    const auto &r = *type;
    throw std::logic_error("Unsupported type in cloneType: " +
                           std::string(typeid(r).name()));
}

/**
 * Get the size of a type in bytes.
 *
 * For X86-64 System V ABI:
 * - `int` and `unsigned int` are 4 bytes (32-bit).
 * - `long` and `unsigned long` are 8 bytes (64-bit).
 *
 * @param type The type to get the size of.
 * @return The size of the type in bytes.
 */
int getTypeSize(const AST::Type *type) {
    if (dynamic_cast<const AST::IntType *>(type) ||
        dynamic_cast<const AST::UIntType *>(type)) {
        return 4;
    }
    else if (dynamic_cast<const AST::LongType *>(type) ||
             dynamic_cast<const AST::ULongType *>(type)) {
        return 8;
    }
    const auto &r = *type;
    throw std::logic_error("Unsupported type in getTypeSize: " +
                           std::string(typeid(r).name()));
}

/**
 * Check if a type is a signed type (`int` or `long`).
 *
 * @param type The type to check.
 * @return True if the type is a signed type, false otherwise.
 */
bool isSigned(const AST::Type *type) {
    return dynamic_cast<const AST::IntType *>(type) ||
           dynamic_cast<const AST::LongType *>(type);
}

/**
 * Clone a static initializer.
 *
 * @param staticInit The static initializer to clone.
 * @return The cloned static initializer.
 */
std::unique_ptr<AST::StaticInit>
cloneStaticInit(const AST::StaticInit *staticInit) {
    if (auto intInit = dynamic_cast<const AST::IntInit *>(staticInit)) {
        auto value = intInit->getValue();
        return std::make_unique<AST::IntInit>(std::get<int>(value));
    }
    else if (auto longInit = dynamic_cast<const AST::LongInit *>(staticInit)) {
        auto value = longInit->getValue();
        return std::make_unique<AST::LongInit>(std::get<long>(value));
    }
    else if (auto uintInit = dynamic_cast<const AST::UIntInit *>(staticInit)) {
        auto value = uintInit->getValue();
        return std::make_unique<AST::UIntInit>(std::get<unsigned int>(value));
    }
    else if (auto ulongInit =
                 dynamic_cast<const AST::ULongInit *>(staticInit)) {
        auto value = ulongInit->getValue();
        return std::make_unique<AST::ULongInit>(std::get<unsigned long>(value));
    }
    throw std::logic_error(
        "Unsupported static initializer type in cloneStaticInit");
}
} // namespace

namespace IR {
IRGenerator::IRGenerator(int variableResolutionCounter,
                         AST::FrontendSymbolTable &frontendSymbolTable)
    : irTemporariesCounter(variableResolutionCounter),
      frontendSymbolTable(frontendSymbolTable) {}

std::pair<std::unique_ptr<IR::Program>,
          std::unique_ptr<std::vector<std::unique_ptr<IR::StaticVariable>>>>
IRGenerator::generateIR(const AST::Program &astProgram) {
    // Initialize the vector of IR top-levels (top-level elements).
    auto topLevels =
        std::make_unique<std::vector<std::unique_ptr<IR::TopLevel>>>();

    // Generate IR instructions for each AST top-level element.
    for (const auto &astDeclaration : astProgram.getDeclarations()) {
        if (auto *functionDeclaration =
                dynamic_cast<AST::FunctionDeclaration *>(
                    astDeclaration.get())) {
            // Get the body of the function declaration.
            auto optBody = functionDeclaration->getOptBody();

            // Skip generating IR instructions for forward declarations.
            if (!optBody) {
                continue;
            }

            // Create a new vector of IR instructions for the function.
            auto instructions = std::make_unique<
                std::vector<std::unique_ptr<IR::Instruction>>>();

            // Get the identifier and the parameters of the function
            // declaration.
            auto identifier = functionDeclaration->getIdentifier();
            auto parameters = std::make_unique<std::vector<std::string>>(
                functionDeclaration->getParameterIdentifiers());

            // Find the global attribute of the function declaration in the
            // frontend symbol table and set the global flag.
            bool global = false;
            if (frontendSymbolTable.find(identifier) !=
                frontendSymbolTable.end()) {
                auto &symbolEntry = frontendSymbolTable[identifier];
                if (auto *functionAttribute =
                        dynamic_cast<AST::FunctionAttribute *>(
                            symbolEntry.second.get())) {
                    global = functionAttribute->isGlobal();
                }
                else {
                    const auto &r = *symbolEntry.second;
                    throw std::logic_error(
                        "Function attribute not found in frontendSymbolTable "
                        "while generating IR instructions for function "
                        "definition in generateIR in IRGenerator: " +
                        std::string(typeid(r).name()));
                }
            }
            else {
                const auto &r = *astDeclaration;
                throw std::logic_error(
                    "Function declaration not found in frontendSymbolTable "
                    "while generating IR instructions for function definition "
                    "in generateIR in IRGenerator: " +
                    std::string(typeid(r).name()));
            }

            // Generate IR instructions for the function body.
            generateIRBlock(optBody, *instructions);

            // Check if the function has any return statements.
            bool hasReturnStatement = false;
            for (const auto &instruction : *instructions) {
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
                if (instructions->empty() ||
                    !dynamic_cast<IR::ReturnInstruction *>(
                        instructions->back().get())) {
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
                auto functionType = frontendSymbolTable[identifier].first.get();
                auto functionTypePtr =
                    dynamic_cast<AST::FunctionType *>(functionType);
                if (functionTypePtr) {
                    const auto &returnType = functionTypePtr->getReturnType();

                    // Create a constant value based on the return type.
                    std::unique_ptr<IR::Value> returnValue;
                    if (dynamic_cast<const AST::IntType *>(&returnType)) {
                        returnValue = std::make_unique<IR::ConstantValue>(
                            std::make_unique<AST::ConstantInt>(0));
                    }
                    else if (dynamic_cast<const AST::LongType *>(&returnType)) {
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
                    instructions->emplace_back(
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
            topLevels->emplace_back(std::move(irFunctionDefinition));
        }
        else if (auto *variableDeclaration =
                     dynamic_cast<AST::VariableDeclaration *>(
                         astDeclaration.get())) {
            // Continue: Do not generate IR instructions for file-scope variable
            // declarations or for local variable declarations with `static` or
            // `extern` storage-class specifiers (for now).
            if (variableDeclaration->getOptStorageClass()) {
                if (dynamic_cast<AST::StaticStorageClass *>(
                        variableDeclaration->getOptStorageClass()) ||
                    dynamic_cast<AST::ExternStorageClass *>(
                        variableDeclaration->getOptStorageClass())) {
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
    const AST::Block *astBlock,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Get the block items from the block.
    // Generate IR instructions for each block item.
    for (const auto &blockItem : astBlock->getBlockItems()) {
        // If the block item is a `DBlockItem` (i.e., a declaration), ...
        if (auto *dBlockItem =
                dynamic_cast<AST::DBlockItem *>(blockItem.get())) {
            // Generate IR instructions for the variable declaration (that has
            // an initializer).
            if (auto *variableDeclaration =
                    dynamic_cast<AST::VariableDeclaration *>(
                        dBlockItem->getDeclaration())) {
                // Continue: Do not generate IR instructions for file-scope
                // variable declarations or for local variable declarations with
                // `static` or `extern` storage-class specifiers (for now).
                if (variableDeclaration->getOptStorageClass()) {
                    if (dynamic_cast<AST::StaticStorageClass *>(
                            variableDeclaration->getOptStorageClass()) ||
                        dynamic_cast<AST::ExternStorageClass *>(
                            variableDeclaration->getOptStorageClass())) {
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
            else if (auto *functionDeclaration =
                         dynamic_cast<AST::FunctionDeclaration *>(
                             dBlockItem->getDeclaration())) {
                generateIRFunctionDefinition(functionDeclaration, instructions);
            }
        }

        // If the block item is a `SBockItem` (i.e., a statement), ...
        else if (auto *sBlockItem =
                     dynamic_cast<AST::SBlockItem *>(blockItem.get())) {
            // Generate IR instructions for the statement.
            generateIRStatement(sBlockItem->getStatement(), instructions);
        }
    }
}

void IRGenerator::generateIRFunctionDefinition(
    const AST::FunctionDeclaration *astFunctionDeclaration,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Get the body of the function.
    auto optBody = astFunctionDeclaration->getOptBody();
    if (optBody) {
        // Generate IR instructions for the body of the function.
        generateIRBlock(optBody, instructions);
    }
    // Otherwise (i.e., if the function does not have a body), we do not need
    // to generate any IR instructions for now.
}

void IRGenerator::generateIRVariableDefinition(
    const AST::VariableDeclaration *astVariableDeclaration,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    auto identifier = astVariableDeclaration->getIdentifier();
    auto initializer = astVariableDeclaration->getOptInitializer();
    // If the declaration has an initializer, ...
    if (initializer) {
        // Generate IR instructions for the initializer.
        auto result = generateIRInstruction(initializer, instructions);

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
    const AST::Statement *astStatement,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    if (auto *returnStmt =
            dynamic_cast<const AST::ReturnStatement *>(astStatement)) {
        generateIRReturnStatement(returnStmt, instructions);
    }
    else if (auto *expressionStmt =
                 dynamic_cast<const AST::ExpressionStatement *>(astStatement)) {
        generateIRExpressionStatement(expressionStmt, instructions);
    }
    else if (auto *compoundStmt =
                 dynamic_cast<const AST::CompoundStatement *>(astStatement)) {
        // If the statement is a compound statement, generate a block.
        generateIRBlock(compoundStmt->getBlock(), instructions);
    }
    else if (auto *ifStmt =
                 dynamic_cast<const AST::IfStatement *>(astStatement)) {
        generateIRIfStatement(ifStmt, instructions);
    }
    else if (auto *breakStmt =
                 dynamic_cast<const AST::BreakStatement *>(astStatement)) {
        generateIRBreakStatement(breakStmt, instructions);
    }
    else if (auto *continueStmt =
                 dynamic_cast<const AST::ContinueStatement *>(astStatement)) {
        generateIRContinueStatement(continueStmt, instructions);
    }
    else if (auto *whileStmt =
                 dynamic_cast<const AST::WhileStatement *>(astStatement)) {
        generateIRWhileStatement(whileStmt, instructions);
    }
    else if (auto *doWhileStmt =
                 dynamic_cast<const AST::DoWhileStatement *>(astStatement)) {
        generateIRDoWhileStatement(doWhileStmt, instructions);
    }
    else if (auto *forStmt =
                 dynamic_cast<const AST::ForStatement *>(astStatement)) {
        generateIRForStatement(forStmt, instructions);
    }
    else if (dynamic_cast<const AST::NullStatement *>(astStatement)) {
        // If the statement is a null statement, do nothing.
    }
    else {
        throw std::logic_error(
            "Unsupported statement type while generating IR instructions for "
            "statement in generateIRStatement in IRGenerator");
    }
}

void IRGenerator::generateIRReturnStatement(
    const AST::ReturnStatement *returnStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Get the expression from the return statement.
    auto exp = returnStmt->getExpression();

    // Process the expression and generate the corresponding IR
    // instructions.
    auto result = generateIRInstruction(exp, instructions);

    // Generate a return instruction with the result value.
    instructions.emplace_back(
        std::make_unique<IR::ReturnInstruction>(std::move(result)));
}

void IRGenerator::generateIRExpressionStatement(
    const AST::ExpressionStatement *expressionStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Get the expression from the expression statement.
    auto exp = expressionStmt->getExpression();

    // Process the expression and generate the corresponding IR
    // instructions.
    auto result = generateIRInstruction(exp, instructions);
    // We do not need to do anything with the result value.
}

void IRGenerator::generateIRIfStatement(
    const AST::IfStatement *ifStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Get the condition from the if-statement.
    auto condition = ifStmt->getCondition();
    // Process the condition and generate the corresponding IR instructions.
    auto conditionValue = generateIRInstruction(condition, instructions);
    // Generate a new end label.
    auto endLabel = generateIREndLabel();
    // Generate a new else label.
    auto elseLabel = generateIRElseLabel();

    if (ifStmt->getElseOptStatement()) {
        // Generate a jump-if-zero instruction with the condition value and the
        // else label.
        instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
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
        auto elseOptStatement = ifStmt->getElseOptStatement();
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
        auto thenStatement = ifStmt->getThenStatement();
        // Process the then-statement and generate the corresponding IR
        // instructions.
        generateIRStatement(thenStatement, instructions);
    }

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);
}

void IRGenerator::generateIRBreakStatement(
    const AST::BreakStatement *breakStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate a jump instruction with the extended break label.
    auto breakLabel = generateIRBreakLoopLabel(breakStmt->getLabel());
    generateIRJumpInstruction(breakLabel, instructions);
}

void IRGenerator::generateIRContinueStatement(
    const AST::ContinueStatement *continueStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate a jump instruction with the extended continue label.
    auto continueLabel = generateIRContinueLoopLabel(continueStmt->getLabel());
    generateIRJumpInstruction(continueLabel, instructions);
}

void IRGenerator::generateIRDoWhileStatement(
    const AST::DoWhileStatement *doWhileStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
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
    generateIRJumpIfNotZeroInstruction(std::move(conditionValue), startLabel,
                                       instructions);
    // Generate a new break label (based on the label of the do-while).
    auto breakLabel = generateIRBreakLoopLabel(doWhileStmt->getLabel());
    // Generate a label instruction with the break label.
    generateIRLabelInstruction(breakLabel, instructions);
}

void IRGenerator::generateIRWhileStatement(
    const AST::WhileStatement *whileStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
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
    generateIRJumpIfZeroInstruction(std::move(conditionValue), breakLabel,
                                    instructions);
    // Generate instructions for the body of the while-statement.
    auto body = whileStmt->getBody();
    generateIRStatement(body, instructions);
    // Generate a jump instruction with the continue label.
    generateIRJumpInstruction(continueLabel, instructions);
    // Generate a label instruction with the break label.
    generateIRLabelInstruction(breakLabel, instructions);
}

void IRGenerator::generateIRForStatement(
    const AST::ForStatement *forStmt,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate instructions for the for-init of the for-statement.
    auto forInit = forStmt->getForInit();
    if (auto initExpr = dynamic_cast<const AST::InitExpr *>(forInit)) {
        if (auto optExpr = initExpr->getExpression()) {
            auto resolvedExpr = generateIRInstruction(optExpr, instructions);
        }
    }
    else if (auto initDecl = dynamic_cast<const AST::InitDecl *>(forInit)) {
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
    if (optCondition) {
        // Generate a jump-if-zero instruction with the condition value and the
        // break label.
        auto conditionValue = generateIRInstruction(optCondition, instructions);
        generateIRJumpIfZeroInstruction(std::move(conditionValue), breakLabel,
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
    if (optPost) {
        auto postValue = generateIRInstruction(optPost, instructions);
    }
    // Generate a jump instruction with the start label.
    generateIRJumpInstruction(startLabel, instructions);
    // Generate a label instruction with the break label.
    generateIRLabelInstruction(breakLabel, instructions);
}

std::unique_ptr<IR::Value> IRGenerator::generateIRInstruction(
    const AST::Expression *e,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    if (auto constantExpr = dynamic_cast<const AST::ConstantExpression *>(e)) {
        auto variantValue = constantExpr->getConstantInVariant();
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
        else if (std::holds_alternative<unsigned int>(variantValue)) {
            return std::make_unique<IR::ConstantValue>(
                std::make_unique<AST::ConstantUInt>(
                    std::get<unsigned int>(variantValue)));
        }
        else if (std::holds_alternative<unsigned long>(variantValue)) {
            return std::make_unique<IR::ConstantValue>(
                std::make_unique<AST::ConstantULong>(
                    std::get<unsigned long>(variantValue)));
        }
        else {
            throw std::logic_error(
                "Unsupported constant type in constant expression in "
                "getConstantInVariant in ConstantExpression: " +
                std::string(typeid(variantValue).name()));
        }
    }
    else if (auto unaryExpr = dynamic_cast<const AST::UnaryExpression *>(e)) {
        return generateIRUnaryInstruction(unaryExpr, instructions);
    }
    else if (auto binaryExpr = dynamic_cast<const AST::BinaryExpression *>(e)) {
        // If the binary operator in the AST binary expression is a
        // logical-and operator, ...
        if (dynamic_cast<AST::AndOperator *>(binaryExpr->getOperator())) {
            return generateIRInstructionWithLogicalAnd(binaryExpr,
                                                       instructions);
        }
        // If the binary operator in the AST binary expression is a
        // logical-or operator, ...
        else if (dynamic_cast<AST::OrOperator *>(binaryExpr->getOperator())) {
            return generateIRInstructionWithLogicalOr(binaryExpr, instructions);
        }
        // Otherwise (i.e., if the binary operator in the AST binary
        // expression contains neither a logical-and nor a logical-or
        // operator), ...
        else {
            return generateIRBinaryInstruction(binaryExpr, instructions);
        }
    }
    else if (auto variableExpr =
                 dynamic_cast<const AST::VariableExpression *>(e)) {
        return std::make_unique<IR::VariableValue>(
            variableExpr->getIdentifier());
    }
    else if (auto assignmentExpr =
                 dynamic_cast<const AST::AssignmentExpression *>(e)) {
        if (auto variableExpr1 = dynamic_cast<AST::VariableExpression *>(
                assignmentExpr->getLeft())) {
            auto variableValue = std::make_unique<IR::VariableValue>(
                variableExpr1->getIdentifier());
            auto result =
                generateIRInstruction(assignmentExpr->getRight(), instructions);
            instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
                std::move(result), std::make_unique<IR::VariableValue>(
                                       variableExpr1->getIdentifier())));
            return variableValue;
        }
        else {
            const auto &r = *assignmentExpr;
            throw std::logic_error(
                "Unsupported lvalue type in assignment while generating IR "
                "instructions for expression in generateIRInstruction in "
                "IRGenerator: " +
                std::string(typeid(r).name()));
        }
    }
    else if (auto conditionalExpr =
                 dynamic_cast<const AST::ConditionalExpression *>(e)) {
        auto conditionValue = generateIRInstruction(
            conditionalExpr->getCondition(), instructions);
        auto e2Label = generateIRE2Label();
        generateIRJumpIfZeroInstruction(std::move(conditionValue), e2Label,
                                        instructions);
        auto e1Value = generateIRInstruction(
            conditionalExpr->getThenExpression(), instructions);
        auto resultLabel = generateIRResultLabel();

        auto resultType = conditionalExpr->getExpType();
        if (!resultType) {
            throw std::logic_error(
                "Missing result type for conditional expression in "
                "generateIRConditionalExpression in IRGenerator");
        }
        // Add the result variable to the frontend symbol table with the type of
        // the conditional expression's result.
        frontendSymbolTable[resultLabel] = std::make_pair(
            cloneType(resultType), std::make_unique<AST::LocalAttribute>());

        auto resultValue = std::make_unique<IR::VariableValue>(resultLabel);
        generateIRCopyInstruction(
            std::move(e1Value),
            std::make_unique<IR::VariableValue>(resultLabel), instructions);
        auto endLabel = generateIREndLabel();
        generateIRJumpInstruction(endLabel, instructions);
        generateIRLabelInstruction(e2Label, instructions);
        auto e2Value = generateIRInstruction(
            conditionalExpr->getElseExpression(), instructions);
        generateIRCopyInstruction(
            std::move(e2Value),
            std::make_unique<IR::VariableValue>(resultLabel), instructions);
        generateIRLabelInstruction(endLabel, instructions);
        return resultValue;
    }
    else if (auto functionCallExpr =
                 dynamic_cast<const AST::FunctionCallExpression *>(e)) {
        auto functionIdentifier = functionCallExpr->getIdentifier();
        auto args = std::make_unique<std::vector<std::unique_ptr<IR::Value>>>();
        for (auto &arg : functionCallExpr->getArguments()) {
            args->emplace_back(generateIRInstruction(arg.get(), instructions));
        }
        auto resultValue = generateIRFunctionCallInstruction(
            functionIdentifier, std::move(args), instructions);
        return resultValue;
    }
    else if (auto castExpr = dynamic_cast<const AST::CastExpression *>(e)) {
        return generateIRCastInstruction(castExpr, instructions);
    }
    const auto &r = *e;
    throw std::logic_error(
        "Unsupported expression type while generating IR instructions for "
        "expression in generateIRInstruction in IRGenerator: " +
        std::string(typeid(r).name()));
}

std::unique_ptr<IR::VariableValue> IRGenerator::generateIRUnaryInstruction(
    const AST::UnaryExpression *unaryExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Recursively generate the expression in the unary expression.
    auto src = generateIRInstruction(unaryExpr->getExpression(), instructions);

    // Create a temporary variable (in string) to store the result of
    // the unary operation.
    auto tmpName = generateIRTemporary();

    // Add the temporary variable to the frontend symbol table with the type
    // of the expression and local attribute.
    frontendSymbolTable[tmpName] =
        std::make_pair(cloneType(unaryExpr->getExpType()),
                       std::make_unique<AST::LocalAttribute>());

    // Create a variable value for the temporary variable.
    auto dst = std::make_unique<IR::VariableValue>(tmpName);

    // Convert the unary operator in the unary expression to a IR
    // unary operator.
    auto IROp = convertUnop(unaryExpr->getOperator());

    // Generate a unary instruction with the IR unary operator, the
    // source value, and the destination value.
    instructions.emplace_back(std::make_unique<IR::UnaryInstruction>(
        std::move(IROp), std::move(src),
        std::make_unique<IR::VariableValue>(tmpName)));

    // Return the destination value.
    return dst;
}

std::unique_ptr<IR::VariableValue> IRGenerator::generateIRBinaryInstruction(
    const AST::BinaryExpression *binaryExpr,
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
        std::move(IROp), std::move(lhs), std::move(rhs),
        std::make_unique<IR::VariableValue>(dst->getIdentifier())));

    // Return the destination value.
    return dst;
}

std::unique_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalAnd(
    const AST::BinaryExpression *binaryExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
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

    // Add the result variable to the frontend symbol table with type int.
    frontendSymbolTable[resultLabel] =
        std::make_pair(std::make_unique<AST::IntType>(),
                       std::make_unique<AST::LocalAttribute>());

    auto dst = std::make_unique<IR::VariableValue>(resultLabel);
    generateIRCopyInstruction(std::make_unique<IR::ConstantValue>(
                                  std::make_unique<AST::ConstantInt>(1)),
                              std::make_unique<IR::VariableValue>(resultLabel),
                              instructions);

    // Generate a jump instruction with a new end label.
    auto endLabel = generateIREndLabel();
    generateIRJumpInstruction(endLabel, instructions);

    // Generate a label instruction with the same (new) false label.
    generateIRLabelInstruction(falseLabel, instructions);

    // Generate a copy instruction with 0 being copied to the result.
    generateIRCopyInstruction(std::make_unique<IR::ConstantValue>(
                                  std::make_unique<AST::ConstantInt>(0)),
                              std::make_unique<IR::VariableValue>(resultLabel),
                              instructions);

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);

    // Return the destination value.
    return dst;
}

std::unique_ptr<IR::VariableValue>
IRGenerator::generateIRInstructionWithLogicalOr(
    const AST::BinaryExpression *binaryExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
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
    generateIRJumpIfNotZeroInstruction(std::move(rhs), trueLabel, instructions);

    // Generate a copy instruction with 0 being copied to a (new) result
    // label.
    auto resultLabel = generateIRResultLabel();

    // Add the result variable to the frontend symbol table with type int.
    frontendSymbolTable[resultLabel] =
        std::make_pair(std::make_unique<AST::IntType>(),
                       std::make_unique<AST::LocalAttribute>());

    auto dst = std::make_unique<IR::VariableValue>(resultLabel);
    generateIRCopyInstruction(std::make_unique<IR::ConstantValue>(
                                  std::make_unique<AST::ConstantInt>(0)),
                              std::make_unique<IR::VariableValue>(resultLabel),
                              instructions);

    // Generate a jump instruction with a new end label.
    auto endLabel = generateIREndLabel();
    generateIRJumpInstruction(endLabel, instructions);

    // Generate a label instruction with the same (new) true label.
    generateIRLabelInstruction(trueLabel, instructions);

    // Generate a copy instruction with 1 being copied to the result.
    generateIRCopyInstruction(std::make_unique<IR::ConstantValue>(
                                  std::make_unique<AST::ConstantInt>(1)),
                              std::make_unique<IR::VariableValue>(resultLabel),
                              instructions);

    // Generate a label instruction with the same (new) end label.
    generateIRLabelInstruction(endLabel, instructions);

    // Return the destination value.
    return dst;
}

void IRGenerator::generateIRCopyInstruction(
    std::unique_ptr<IR::Value> src, std::unique_ptr<IR::Value> dst,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate a copy instruction with the source value and the
    // destination value.
    instructions.emplace_back(
        std::make_unique<IR::CopyInstruction>(std::move(src), std::move(dst)));
}

void IRGenerator::generateIRJumpInstruction(
    std::string_view target,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate a jump instruction with the target label.
    instructions.emplace_back(std::make_unique<IR::JumpInstruction>(target));
}

void IRGenerator::generateIRJumpIfZeroInstruction(
    std::unique_ptr<IR::Value> condition, std::string_view target,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate a jump if zero instruction with the condition value and
    // the target label.
    instructions.emplace_back(std::make_unique<IR::JumpIfZeroInstruction>(
        std::move(condition), target));
}

void IRGenerator::generateIRJumpIfNotZeroInstruction(
    std::unique_ptr<IR::Value> condition, std::string_view target,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate a jump if not zero instruction with the condition value
    // and the target label.
    instructions.emplace_back(std::make_unique<IR::JumpIfNotZeroInstruction>(
        std::move(condition), target));
}

void IRGenerator::generateIRLabelInstruction(
    std::string_view identifier,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Generate a label instruction with the label identifier.
    instructions.emplace_back(
        std::make_unique<IR::LabelInstruction>(identifier));
}

std::unique_ptr<IR::VariableValue>
IRGenerator::generateIRFunctionCallInstruction(
    std::string_view functionIdentifier,
    std::unique_ptr<std::vector<std::unique_ptr<IR::Value>>> arguments,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Create a temporary variable (in string) to store the result of
    // the function call.
    auto tmpName = generateIRTemporary();

    // Look up the function's return type in the frontend symbol table.
    auto functionType =
        frontendSymbolTable[std::string(functionIdentifier)].first.get();
    auto functionTypePtr = dynamic_cast<AST::FunctionType *>(functionType);
    if (!functionTypePtr) {
        throw std::logic_error(
            "Function type not found in symbol table in "
            "generateIRFunctionCallInstruction in IRGenerator: " +
            std::string(functionIdentifier));
    }
    const auto &returnType = functionTypePtr->getReturnType();

    // Add the temporary variable to the frontend symbol table with the type
    // of the function's return type and local attribute.
    frontendSymbolTable[tmpName] = std::make_pair(
        cloneType(&returnType), std::make_unique<AST::LocalAttribute>());

    // Create a variable value for the temporary variable.
    auto dst = std::make_unique<IR::VariableValue>(tmpName);
    // Generate a function call instruction with the function identifier,
    // the arguments, and the destination value.
    instructions.emplace_back(std::make_unique<IR::FunctionCallInstruction>(
        functionIdentifier, std::move(arguments),
        std::make_unique<IR::VariableValue>(tmpName)));
    // Return the destination value.
    return dst;
}

std::unique_ptr<IR::VariableValue> IRGenerator::generateIRCastInstruction(
    const AST::CastExpression *castExpr,
    std::vector<std::unique_ptr<IR::Instruction>> &instructions) {
    // Recursively generate the expression in the cast expression.
    auto result =
        generateIRInstruction(castExpr->getExpression(), instructions);
    auto sourceType = castExpr->getExpression()->getExpType();
    auto targetType = castExpr->getTargetType();

    // If the source and target types are the same, no cast is needed.
    if (*sourceType == *targetType) {
        // Return the result as a `VariableValue` if it's already a variable.
        // If it's a constant, we need to copy it to a variable, which we handle
        // later as a fallback.
        auto *varValue = dynamic_cast<IR::VariableValue *>(result.get());
        if (varValue) {
            return std::make_unique<IR::VariableValue>(
                varValue->getIdentifier());
        }
    }

    // Create a temporary variable to store the result of the cast operation.
    auto dstName = generateIRTemporary();
    // Add the temporary variable to the frontend symbol table with the type
    // of the target type and local attribute.
    frontendSymbolTable[dstName] = std::make_pair(
        cloneType(targetType), std::make_unique<AST::LocalAttribute>());
    // Create a variable value for the temporary variable.
    auto dst = std::make_unique<IR::VariableValue>(dstName);

    int sourceSize = getTypeSize(sourceType);
    int targetSize = getTypeSize(targetType);
    if (sourceSize == targetSize) {
        // The source and target types are the same size: just copy.
        instructions.emplace_back(std::make_unique<IR::CopyInstruction>(
            std::move(result), std::make_unique<IR::VariableValue>(dstName)));
    }
    else if (targetSize < sourceSize) {
        // The target type is smaller than the source type: truncate.
        instructions.emplace_back(std::make_unique<IR::TruncateInstruction>(
            std::move(result), std::make_unique<IR::VariableValue>(dstName)));
    }
    else if (isSigned(sourceType)) {
        // The target type is larger than the source type and the source type is
        // signed: sign-extend.
        instructions.emplace_back(std::make_unique<IR::SignExtendInstruction>(
            std::move(result), std::make_unique<IR::VariableValue>(dstName)));
    }
    else {
        // The target type is larger than the source type and the source type is
        // unsigned: zero-extend.
        instructions.emplace_back(std::make_unique<IR::ZeroExtendInstruction>(
            std::move(result), std::make_unique<IR::VariableValue>(dstName)));
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

std::unique_ptr<std::vector<std::unique_ptr<IR::StaticVariable>>>
IRGenerator::convertFrontendSymbolTableToIRStaticVariables() {
    auto irDefs =
        std::make_unique<std::vector<std::unique_ptr<IR::StaticVariable>>>();
    for (const auto &symbol : frontendSymbolTable) {
        auto name = symbol.first;
        auto *type = symbol.second.first.get();
        auto *attribute = symbol.second.second.get();
        if (auto staticAttribute =
                dynamic_cast<AST::StaticAttribute *>(attribute)) {
            auto initialValue = staticAttribute->getInitialValue();
            auto global = staticAttribute->isGlobal();
            if (auto initial = dynamic_cast<AST::Initial *>(initialValue)) {
                irDefs->emplace_back(std::make_unique<StaticVariable>(
                    name, global, cloneType(type),
                    cloneStaticInit(initial->getStaticInit())));
                continue;
            }
            if (dynamic_cast<AST::NoInitializer *>(initialValue)) {
                continue;
            }
            if (dynamic_cast<AST::Tentative *>(initialValue)) {
                if (dynamic_cast<AST::IntType *>(type)) {
                    irDefs->emplace_back(std::make_unique<StaticVariable>(
                        name, global, cloneType(type),
                        std::make_unique<AST::IntInit>(0)));
                }
                else if (dynamic_cast<AST::LongType *>(type)) {
                    irDefs->emplace_back(std::make_unique<StaticVariable>(
                        name, global, cloneType(type),
                        std::make_unique<AST::LongInit>(0L)));
                }
                else if (dynamic_cast<AST::UIntType *>(type)) {
                    irDefs->emplace_back(std::make_unique<StaticVariable>(
                        name, global, cloneType(type),
                        std::make_unique<AST::UIntInit>(0U)));
                }
                else if (dynamic_cast<AST::ULongType *>(type)) {
                    irDefs->emplace_back(std::make_unique<StaticVariable>(
                        name, global, cloneType(type),
                        std::make_unique<AST::ULongInit>(0UL)));
                }
                else {
                    const auto &r = *type;
                    throw std::logic_error(
                        "Unsupported tentative type while converting "
                        "frontendSymbolTable to IR static variables in "
                        "convertFrontendSymbolTableToIRStaticVariables in "
                        "IRGenerator: " +
                        std::string(typeid(r).name()));
                }
                continue;
            }
            throw std::logic_error(
                "Unsupported initial value type while converting "
                "frontendSymbolTable to IR static variables in "
                "convertFrontendSymbolTableToIRStaticVariables in "
                "IRGenerator");
        }
        else {
            continue;
        }
    }
    return irDefs;
}

std::unique_ptr<IR::UnaryOperator>
IRGenerator::convertUnop(const AST::UnaryOperator *op) {
    if (dynamic_cast<const AST::NegateOperator *>(op)) {
        return std::make_unique<IR::NegateOperator>();
    }
    else if (dynamic_cast<const AST::ComplementOperator *>(op)) {
        return std::make_unique<IR::ComplementOperator>();
    }
    else if (dynamic_cast<const AST::NotOperator *>(op)) {
        return std::make_unique<IR::NotOperator>();
    }
    const auto &r = *op;
    throw std::logic_error(
        "Unsupported unary operator while converting unary operator to IR "
        "unary operator in convertUnop in IRGenerator: " +
        std::string(typeid(r).name()));
}

// Note: The logical-and and logical-or operators in the AST are NOT binary
// operators in the IR (and should NOT be converted to binary operators in
// the IR).
std::unique_ptr<IR::BinaryOperator>
IRGenerator::convertBinop(const AST::BinaryOperator *op) {
    if (dynamic_cast<const AST::AddOperator *>(op)) {
        return std::make_unique<IR::AddOperator>();
    }
    else if (dynamic_cast<const AST::SubtractOperator *>(op)) {
        return std::make_unique<IR::SubtractOperator>();
    }
    else if (dynamic_cast<const AST::MultiplyOperator *>(op)) {
        return std::make_unique<IR::MultiplyOperator>();
    }
    else if (dynamic_cast<const AST::DivideOperator *>(op)) {
        return std::make_unique<IR::DivideOperator>();
    }
    else if (dynamic_cast<const AST::RemainderOperator *>(op)) {
        return std::make_unique<IR::RemainderOperator>();
    }
    else if (dynamic_cast<const AST::EqualOperator *>(op)) {
        return std::make_unique<IR::EqualOperator>();
    }
    else if (dynamic_cast<const AST::NotEqualOperator *>(op)) {
        return std::make_unique<IR::NotEqualOperator>();
    }
    else if (dynamic_cast<const AST::LessThanOperator *>(op)) {
        return std::make_unique<IR::LessThanOperator>();
    }
    else if (dynamic_cast<const AST::LessThanOrEqualOperator *>(op)) {
        return std::make_unique<IR::LessThanOrEqualOperator>();
    }
    else if (dynamic_cast<const AST::GreaterThanOperator *>(op)) {
        return std::make_unique<IR::GreaterThanOperator>();
    }
    else if (dynamic_cast<const AST::GreaterThanOrEqualOperator *>(op)) {
        return std::make_unique<IR::GreaterThanOrEqualOperator>();
    }
    const auto &r = *op;
    throw std::logic_error(
        "Unsupported binary operator while converting binary operator to IR "
        "binary operator in convertBinop in IRGenerator: " +
        std::string(typeid(r).name()));
}

std::unique_ptr<IR::VariableValue>
IRGenerator::generateIRVariable(const AST::BinaryExpression *binaryExpr) {
    // Create a temporary variable (in string) to store the result of
    // the binary operation.
    auto tmpName = generateIRTemporary();

    // Add the temporary variable to the frontend symbol table with the
    // appropriate type and local attribute.
    frontendSymbolTable[tmpName] =
        std::make_pair(cloneType(binaryExpr->getExpType()),
                       std::make_unique<AST::LocalAttribute>());

    // Create a variable value for the temporary variable and return it.
    return std::make_unique<IR::VariableValue>(tmpName);
}
} // namespace IR
