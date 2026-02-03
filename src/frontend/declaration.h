#ifndef FRONTEND_DECLARATION_H
#define FRONTEND_DECLARATION_H

#include "ast.h"
#include "expression.h"
#include "storageClass.h"
#include "type.h"
#include "visitor.h"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace AST {
// Forward declaration for `Block` (used by `FunctionDeclaration`).
class Block;

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
                                 std::unique_ptr<Type> varType);
    /**
     * Constructor for the `VariableDeclaration` class with an optional
     * initializer and without an optional storage class.
     *
     * @param identifier The identifier of the variable.
     * @param optInitializer The optional initializer expression for the
     * variable (can be `nullptr`).
     * @param varType The type of the variable.
     */
    explicit VariableDeclaration(std::string_view identifier,
                                 std::unique_ptr<Expression> optInitializer,
                                 std::unique_ptr<Type> varType);

    /**
     * Constructor for the `VariableDeclaration` class without an optional
     * initializer and with an optional storage class.
     *
     * @param identifier The identifier of the variable.
     * @param varType The type of the variable.
     * @param optStorageClass The optional storage class of the variable (can be
     * `nullptr`).
     */
    explicit VariableDeclaration(std::string_view identifier,
                                 std::unique_ptr<Type> varType,
                                 std::unique_ptr<StorageClass> optStorageClass);

    /**
     * Constructor for the `VariableDeclaration` class with an optional
     * initializer and with an optional storage class.
     *
     * @param identifier The identifier of the variable.
     * @param optInitializer The optional initializer expression for the
     * variable (can be `nullptr`).
     * @param varType The type of the variable.
     * @param optStorageClass The optional storage class of the variable (can be
     * `nullptr`).
     */
    explicit VariableDeclaration(std::string_view identifier,
                                 std::unique_ptr<Expression> optInitializer,
                                 std::unique_ptr<Type> varType,
                                 std::unique_ptr<StorageClass> optStorageClass);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    void setIdentifier(std::string_view identifier);

    [[nodiscard]] Expression *getOptInitializer() const;

    void setOptInitializer(std::unique_ptr<Expression> newOptInitializer);

    [[nodiscard]] Type *getVarType() const;

    [[nodiscard]] StorageClass *getOptStorageClass() const;

  private:
    /**
     * The identifier of the variable.
     */
    std::string identifier;

    /**
     * The optional initializer expression of the variable (can be `nullptr`).
     */
    std::unique_ptr<Expression> optInitializer;

    /**
     * The type of the variable.
     */
    std::unique_ptr<Type> varType;

    /**
     * The optional storage class of the variable (can be `nullptr`).
     */
    std::unique_ptr<StorageClass> optStorageClass;
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
        std::unique_ptr<std::vector<std::string>> parameters,
        std::unique_ptr<Type> funType);

    /**
     * Constructor for the `FunctionDeclaration` class with an optional body
     * and without an optional storage class.
     *
     * @param identifier The identifier of the function.
     * @param parameters The parameter identifiers of the function.
     * @param optBody The optional body of the function (can be `nullptr`).
     * @param funType The function type of the function.
     */
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::unique_ptr<std::vector<std::string>> parameters,
        std::unique_ptr<Block> optBody, std::unique_ptr<Type> funType);

    /**
     * Constructor for the `FunctionDeclaration` class without an optional body
     * and with an optional storage class.
     *
     * @param identifier The identifier of the function.
     * @param parameters The parameter identifiers of the function.
     * @param funType The function type of the function.
     * @param optStorageClass The optional storage class of the function (can be
     * nullptr).
     */
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::unique_ptr<std::vector<std::string>> parameters,
        std::unique_ptr<Type> funType,
        std::unique_ptr<StorageClass> optStorageClass);

    /**
     * Constructor for the `FunctionDeclaration` class with an optional body
     * and with an optional storage class.
     *
     * @param identifier The identifier of the function.
     * @param parameters The parameter identifiers of the function.
     * @param optBody The optional body of the function (can be `nullptr`).
     * @param funType The function type of the function.
     * @param optStorageClass The optional storage class of the function (can be
     * `nullptr`).
     */
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::unique_ptr<std::vector<std::string>> parameters,
        std::unique_ptr<Block> optBody, std::unique_ptr<Type> funType,
        std::unique_ptr<StorageClass> optStorageClass);

    /**
     * Destructor for the `FunctionDeclaration` class.
     *
     * Declared here, defined in `declaration.cpp` to allow incomplete type
     * `Block` in the header.
     */
    ~FunctionDeclaration();

    /**
     * Delete the copy constructor for the function declaration class.
     */
    constexpr FunctionDeclaration(const FunctionDeclaration &) = delete;

    /**
     * Delete the copy assignment operator for the function declaration class.
     */
    constexpr FunctionDeclaration &
    operator=(const FunctionDeclaration &) = delete;

    /**
     * Default move constructor for the function declaration class.
     */
    constexpr FunctionDeclaration(FunctionDeclaration &&) = default;

    /**
     * Default move assignment operator for the function declaration class.
     */
    constexpr FunctionDeclaration &operator=(FunctionDeclaration &&) = default;

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    [[nodiscard]] const std::vector<std::string> &
    getParameterIdentifiers() const;

    [[nodiscard]] Type *getFunType() const;

    [[nodiscard]] Block *getOptBody() const;

    [[nodiscard]] StorageClass *getOptStorageClass() const;

    void setParameters(std::unique_ptr<std::vector<std::string>> parameters);

    void setOptBody(std::unique_ptr<Block> optBody);

    void setFunType(std::unique_ptr<Type> funType);

    void setOptStorageClass(std::unique_ptr<StorageClass> optStorageClass);

  private:
    /**
     * The identifier of the function.
     */
    std::string identifier;

    /**
     * The parameter identifiers of the function.
     */
    std::unique_ptr<std::vector<std::string>> parameters;

    /**
     * The optional body of the function (can be `nullptr`).
     */
    std::unique_ptr<Block> optBody;

    /**
     * The function type of the function.
     */
    std::unique_ptr<Type> funType;

    /**
     * The optional storage class of the function (can be `nullptr`).
     */
    std::unique_ptr<StorageClass> optStorageClass;
};
} // Namespace AST

#endif // FRONTEND_DECLARATION_H
