#ifndef FRONTEND_DECLARATION_H
#define FRONTEND_DECLARATION_H

#include "ast.h"
#include "expression.h"
#include "storageClass.h"
#include "type.h"
#include <memory>
#include <optional>
#include <string_view>

namespace AST {
/**
 * Base class for declarations in the AST.
 *
 * A declaration can be either a variable declaration or a function declaration.
 */
class Declaration : public AST {};

/**
 * Class representing a variable declaration.
 */
class VariableDeclaration : public Declaration {
  public:
    /**
     * Constructor for the `VariableDeclaration` class without an optional
     * initializer and without an optional storage class.
     *
     * @param identifier The identifier of the variable.
     * @param varType The type of the variable.
     */
    explicit VariableDeclaration(std::string_view identifier,
                                 std::shared_ptr<Type> varType);
    /**
     * Constructor for the `VariableDeclaration` class with an optional
     * initializer and without an optional storage class.
     *
     * @param identifier The identifier of the variable.
     * @param optInitializer The optional initializer expression for the
     * variable.
     * @param varType The type of the variable.
     */
    explicit VariableDeclaration(
        std::string_view identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer,
        std::shared_ptr<Type> varType);

    /**
     * Constructor for the `VariableDeclaration` class without an optional
     * initializer and with an optional storage class.
     *
     * @param identifier The identifier of the variable.
     * @param varType The type of the variable.
     * @param optStorageClass The optional storage class of the variable.
     */
    explicit VariableDeclaration(
        std::string_view identifier, std::shared_ptr<Type> varType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);

    /**
     * Constructor for the `VariableDeclaration` class with an optional
     * initializer and with an optional storage class.
     *
     * @param identifier The identifier of the variable.
     * @param optInitializer The optional initializer expression for the
     * variable.
     * @param varType The type of the variable.
     * @param optStorageClass The optional storage class of the variable.
     */
    explicit VariableDeclaration(
        std::string_view identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer,
        std::shared_ptr<Type> varType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    [[nodiscard]] std::optional<std::shared_ptr<Expression>>
    getOptInitializer() const;

    void setOptInitializer(
        std::optional<std::shared_ptr<Expression>> newOptInitializer);

    [[nodiscard]] std::shared_ptr<Type> getVarType() const;

    [[nodiscard]] std::optional<std::shared_ptr<StorageClass>>
    getOptStorageClass() const;

  private:
    /**
     * The identifier of the variable.
     */
    std::string identifier;

    /**
     * The optional initializer expression of the variable.
     */
    std::optional<std::shared_ptr<Expression>> optInitializer;

    /**
     * The type of the variable.
     */
    std::shared_ptr<Type> varType;

    /**
     * The optional storage class of the variable.
     */
    std::optional<std::shared_ptr<StorageClass>> optStorageClass;
};

class FunctionDeclaration : public Declaration {
  public:
    /**
     * Constructor for the `FunctionDeclaration` class without an optional
     * body and without an optional storage class.
     *
     * @param identifier The identifier of the function.
     * @param parameters The parameter identifiers of the function
     * @param funType The function type of the function.
     */
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::shared_ptr<Type> funType);

    /**
     * Constructor for the `FunctionDeclaration` class with an optional body
     * and without an optional storage class.
     *
     * @param identifier The identifier of the function.
     * @param parameters The parameter identifiers of the function.
     * @param optBody The optional body of the function.
     * @param funType The function type of the function.
     */
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::optional<std::shared_ptr<Block>> optBody,
        std::shared_ptr<Type> funType);

    /**
     * Constructor for the `FunctionDeclaration` class without an optional body
     * and with an optional storage class.
     *
     * @param identifier The identifier of the function.
     * @param parameters The parameter identifiers of the function.
     * @param funType The function type of the function.
     * @param optStorageClass The optional storage class of the function.
     */
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::shared_ptr<Type> funType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);

    /**
     * Constructor for the `FunctionDeclaration` class with an optional body
     * and with an optional storage class.
     *
     * @param identifier The identifier of the function.
     * @param parameters The parameter identifiers of the function.
     * @param optBody The optional body of the function.
     * @param funType The function type of the function.
     * @param optStorageClass The optional storage class of the function.
     */
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::optional<std::shared_ptr<Block>> optBody,
        std::shared_ptr<Type> funType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    [[nodiscard]] const std::shared_ptr<std::vector<std::string>> &
    getParameterIdentifiers() const;

    [[nodiscard]] std::shared_ptr<Type> getFunType() const;

    [[nodiscard]] std::optional<std::shared_ptr<Block>> getOptBody() const;

    [[nodiscard]] std::optional<std::shared_ptr<StorageClass>>
    getOptStorageClass() const;

    void setParameters(std::shared_ptr<std::vector<std::string>> parameters);

    void setOptBody(std::optional<std::shared_ptr<Block>> optBody);

    void setFunType(std::shared_ptr<Type> funType);

    void setOptStorageClass(
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);

  private:
    /**
     * The identifier of the function.
     */
    std::string identifier;

    /**
     * The parameter identifiers of the function.
     */
    std::shared_ptr<std::vector<std::string>> parameters;

    /**
     * The optional body of the function.
     */
    std::optional<std::shared_ptr<Block>> optBody;

    /**
     * The function type of the function.
     */
    std::shared_ptr<Type> funType;

    /**
     * The optional storage class of the function.
     */
    std::optional<std::shared_ptr<StorageClass>> optStorageClass;
};
} // Namespace AST

#endif // FRONTEND_DECLARATION_H
