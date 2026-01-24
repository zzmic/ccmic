#ifndef MIDEND_IR_H
#define MIDEND_IR_H

#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/type.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace IR {
/**
 * Base class for representing an operator in the IR.
 *
 * An IR operator can be either a unary or binary operator.
 */
class Operator {
  public:
    /**
     * Default constructor for the IR operator class.
     */
    constexpr Operator() = default;

    /**
     * Default virtual destructor for the IR operator class.
     */
    virtual ~Operator() = default;

    /**
     * Delete the copy constructor for the IR operator class.
     */
    constexpr Operator(const Operator &) = delete;

    /**
     * Delete the copy assignment operator for the IR operator class.
     */
    constexpr Operator &operator=(const Operator &) = delete;

    /**
     * Default move constructor for the IR operator class.
     */
    constexpr Operator(Operator &&) = default;

    /**
     * Default move assignment operator for the IR operator class.
     */
    constexpr Operator &operator=(Operator &&) = default;
};

/**
 * Base class for representing a unary operator in the IR.
 */
class UnaryOperator : public Operator {};

/**
 * Class representing the negate unary operator in the IR.
 */
class NegateOperator : public UnaryOperator {};

/**
 * Class representing the complement unary operator in the IR.
 */
class ComplementOperator : public UnaryOperator {};

/**
 * Class representing the not unary operator in the IR.
 */
class NotOperator : public UnaryOperator {};

/**
 * Base class for representing a binary operator in the IR.
 *
 * Note: The logical AND and logical OR operators in the AST are NOT binary
 * operators in the IR.
 */
class BinaryOperator : public Operator {};

/**
 * Class representing the addition binary operator in the IR.
 */
class AddOperator : public BinaryOperator {};

/**
 * Class representing the subtraction binary operator in the IR.
 */
class SubtractOperator : public BinaryOperator {};

/**
 * Class representing the multiplication binary operator in the IR.
 */
class MultiplyOperator : public BinaryOperator {};

/**
 * Class representing the division binary operator in the IR.
 */
class DivideOperator : public BinaryOperator {};

/**
 * Class representing the remainder binary operator in the IR.
 */
class RemainderOperator : public BinaryOperator {};

/**
 * Class representing the equal binary operator in the IR.
 */
class EqualOperator : public BinaryOperator {};

/**
 * Class representing the not equal binary operator in the IR.
 */
class NotEqualOperator : public BinaryOperator {};

/**
 * Class representing the less than binary operator in the IR.
 */
class LessThanOperator : public BinaryOperator {};

/**
 * Class representing the less than or equal binary operator in the IR.
 */
class LessThanOrEqualOperator : public BinaryOperator {};

/**
 * Class representing the greater than binary operator in the IR.
 */
class GreaterThanOperator : public BinaryOperator {};

/**
 * Class representing the greater than or equal binary operator in the IR.
 */
class GreaterThanOrEqualOperator : public BinaryOperator {};

/**
 * Base class for representing a value in the IR.
 */
class Value {
  public:
    /**
     * Default constructor for the IR value class.
     */
    constexpr Value() = default;

    /**
     * Default virtual destructor for the IR value class.
     */
    virtual ~Value() = default;

    /**
     * Delete the copy constructor for the IR value class.
     */
    constexpr Value(const Value &) = delete;

    /**
     * Delete the copy assignment operator for the IR value class.
     */
    constexpr Value &operator=(const Value &) = delete;

    /**
     * Default move constructor for the IR value class.
     */
    constexpr Value(Value &&) = default;

    /**
     * Default move assignment operator for the IR value class.
     */
    constexpr Value &operator=(Value &&) = default;
};

/**
 * Class representing a constant value in the IR.
 */
class ConstantValue : public Value {
  private:
    /**
     * The AST constant encapsulated by this IR constant value.
     */
    std::unique_ptr<AST::Constant> astConstant;

  public:
    /**
     * Constructor for creating an IR constant value from an AST constant.
     *
     * @param astConstant The AST constant to encapsulate.
     * @throws std::logic_error if `astConstant` is null.
     */
    explicit ConstantValue(std::unique_ptr<AST::Constant> astConstant);

    [[nodiscard]] const AST::Constant *getASTConstant() const;

    void setASTConstant(std::unique_ptr<AST::Constant> newAstConstant);
};

/**
 * Class representing a variable value in the IR.
 */
class VariableValue : public Value {
  private:
    /**
     * The identifier of the variable.
     */
    std::string identifier;

  public:
    /**
     * Constructor for creating an IR variable value from a variable
     * identifier.
     *
     * @param identifier The identifier of the variable.
     */
    explicit VariableValue(std::string_view identifier);

    [[nodiscard]] const std::string &getIdentifier() const;

    void setIdentifier(std::string_view newIdentifier);
};

/**
 * Base class for representing an instruction in the IR.
 */
class Instruction {
  public:
    /**
     * Default constructor for the IR instruction class.
     */
    constexpr Instruction() = default;

    /**
     * Default virtual destructor for the IR instruction class.
     */
    virtual ~Instruction() = default;

    /**
     * Delete the copy constructor for the IR instruction class.
     */
    constexpr Instruction(const Instruction &) = delete;

    /**
     * Delete the copy assignment operator for the IR instruction class.
     */
    constexpr Instruction &operator=(const Instruction &) = delete;

    /**
     * Default move constructor for the IR instruction class.
     */
    constexpr Instruction(Instruction &&) = default;

    /**
     * Default move assignment operator for the IR instruction class.
     */
    constexpr Instruction &operator=(Instruction &&) = default;
};

/**
 * Class representing a return instruction in the IR.
 */
class ReturnInstruction : public Instruction {
  private:
    /**
     * The return value of the instruction.
     */
    std::unique_ptr<Value> returnValue;

  public:
    /**
     * Constructor for creating a return instruction with a return value.
     *
     * @param returnValue The return value of the instruction.
     * @throws std::logic_error if `returnValue` is null.
     */
    explicit ReturnInstruction(std::unique_ptr<Value> returnValue);

    [[nodiscard]] Value *getReturnValue() const;

    void setReturnValue(std::unique_ptr<Value> newReturnValue);
};

/**
 * Class representing a sign-extend instruction in the IR.
 */
class SignExtendInstruction : public Instruction {
  private:
    /**
     * The source and destination values of the instruction.
     */
    std::unique_ptr<Value> src, dst;

  public:
    /**
     * Constructor for creating a sign-extend instruction with source and
     * destination values.
     *
     * @param src The source value of the instruction.
     * @param dst The destination value of the instruction.
     * @throws std::logic_error if `src` or `dst` is null.
     */
    explicit SignExtendInstruction(std::unique_ptr<Value> src,
                                   std::unique_ptr<Value> dst);

    [[nodiscard]] Value *getSrc() const;

    [[nodiscard]] Value *getDst() const;

    void setSrc(std::unique_ptr<Value> newSrc);

    void setDst(std::unique_ptr<Value> newDst);
};

/**
 * Class representing a truncate instruction in the IR.
 */
class TruncateInstruction : public Instruction {
  private:
    /**
     * The source and destination values of the instruction.
     */
    std::unique_ptr<Value> src, dst;

  public:
    /**
     * Constructor for creating a truncate instruction with source and
     * destination values.
     *
     * @param src The source value of the instruction.
     * @param dst The destination value of the instruction.
     * @throws std::logic_error if `src` or `dst` is null.
     */
    explicit TruncateInstruction(std::unique_ptr<Value> src,
                                 std::unique_ptr<Value> dst);

    [[nodiscard]] Value *getSrc() const;

    [[nodiscard]] Value *getDst() const;

    void setSrc(std::unique_ptr<Value> newSrc);

    void setDst(std::unique_ptr<Value> newDst);
};

class ZeroExtendInstruction : public Instruction {
  private:
    /**
     * The source and destination values of the instruction.
     */
    std::unique_ptr<Value> src, dst;

  public:
    /**
     * Constructor for creating a zero-extend instruction with source and
     * destination values.
     *
     * @param src The source value of the instruction.
     * @param dst The destination value of the instruction.
     * @throws std::logic_error if `src` or `dst` is null.
     */
    explicit ZeroExtendInstruction(std::unique_ptr<Value> src,
                                   std::unique_ptr<Value> dst);

    [[nodiscard]] Value *getSrc() const;

    [[nodiscard]] Value *getDst() const;

    void setSrc(std::unique_ptr<Value> newSrc);

    void setDst(std::unique_ptr<Value> newDst);
};

/**
 * Class representing a unary instruction in the IR.
 */
class UnaryInstruction : public Instruction {
  private:
    /**
     * The unary operator of the instruction.
     */
    std::unique_ptr<UnaryOperator> unaryOperator;
    /**
     * The source and destination values of the instruction.
     */
    std::unique_ptr<Value> src, dst;

  public:
    /**
     * Constructor for creating a unary instruction with a unary operator,
     * source value, and destination value.
     *
     * @param unaryOperator The unary operator of the instruction.
     * @param src The source value of the instruction.
     * @param dst The destination value of the instruction.
     * @throws std::logic_error if `unaryOperator`, `src`, or `dst` is null.
     */
    explicit UnaryInstruction(std::unique_ptr<UnaryOperator> unaryOperator,
                              std::unique_ptr<Value> src,
                              std::unique_ptr<Value> dst);

    [[nodiscard]] UnaryOperator *getUnaryOperator() const;

    [[nodiscard]] Value *getSrc() const;

    [[nodiscard]] Value *getDst() const;

    void setUnaryOperator(std::unique_ptr<UnaryOperator> newUnaryOperator);

    void setSrc(std::unique_ptr<Value> newSrc);

    void setDst(std::unique_ptr<Value> newDst);
};

/**
 * Class representing a binary instruction in the IR.
 */
class BinaryInstruction : public Instruction {
  private:
    /**
     * The binary operator of the instruction.
     */
    std::unique_ptr<BinaryOperator> binaryOperator;
    /**
     * The source and destination values of the instruction.
     */
    std::unique_ptr<Value> src1, src2, dst;

  public:
    /**
     * Constructor for creating a binary instruction with a binary operator,
     * two source values, and a destination value.
     *
     * @param binaryOperator The binary operator of the instruction.
     * @param src1 The first source value of the instruction.
     * @param src2 The second source value of the instruction.
     * @param dst The destination value of the instruction.
     * @throws std::logic_error if `binaryOperator`, `src1`, `src2`, or `dst`
     * is null.
     */
    explicit BinaryInstruction(std::unique_ptr<BinaryOperator> binaryOperator,
                               std::unique_ptr<Value> src1,
                               std::unique_ptr<Value> src2,
                               std::unique_ptr<Value> dst);

    [[nodiscard]] BinaryOperator *getBinaryOperator() const;

    [[nodiscard]] Value *getSrc1() const;

    [[nodiscard]] Value *getSrc2() const;

    [[nodiscard]] Value *getDst() const;

    void setBinaryOperator(std::unique_ptr<BinaryOperator> newBinaryOperator);

    void setSrc1(std::unique_ptr<Value> newSrc1);

    void setSrc2(std::unique_ptr<Value> newSrc2);

    void setDst(std::unique_ptr<Value> newDst);
};

/**
 * Class representing a copy instruction in the IR.
 */
class CopyInstruction : public Instruction {
  private:
    /**
     * The source and destination values of the instruction.
     */
    std::unique_ptr<Value> src, dst;

  public:
    /**
     * Constructor for creating a copy instruction with source and
     * destination values.
     *
     * @param src The source value of the instruction.
     * @param dst The destination value of the instruction.
     * @throws std::logic_error if `src` or `dst` is null.
     */
    explicit CopyInstruction(std::unique_ptr<Value> src,
                             std::unique_ptr<Value> dst);

    [[nodiscard]] Value *getSrc() const;

    [[nodiscard]] Value *getDst() const;

    void setSrc(std::unique_ptr<Value> newSrc);

    void setDst(std::unique_ptr<Value> newDst);
};

/**
 * Class representing a jump instruction in the IR.
 */
class JumpInstruction : public Instruction {
  private:
    /**
     * The target label of the jump instruction.
     */
    std::string target;

  public:
    /**
     * Constructor for creating a jump instruction with a target label.
     *
     * @param target The target label of the jump instruction.
     */
    explicit JumpInstruction(std::string_view target);

    [[nodiscard]] const std::string &getTarget() const;

    void setTarget(std::string_view newTarget);
};

/**
 * Class representing a conditional jump instruction in the IR.
 */
class JumpIfZeroInstruction : public Instruction {
  private:
    /**
     * The condition value of the jump instruction.
     */
    std::unique_ptr<Value> condition;
    /**
     * The target label of the jump instruction.
     */
    std::string target;

  public:
    /**
     * Constructor for creating a conditional jump instruction with a condition
     * value and target label.
     *
     * @param condition The condition value of the jump instruction.
     * @param target The target label of the jump instruction.
     * @throws std::logic_error if `condition` is null.
     */
    explicit JumpIfZeroInstruction(std::unique_ptr<Value> condition,
                                   std::string_view target);

    [[nodiscard]] Value *getCondition() const;

    [[nodiscard]] const std::string &getTarget() const;

    void setCondition(std::unique_ptr<Value> newCondition);

    void setTarget(std::string_view newTarget);
};

/**
 * Class representing a conditional jump instruction in the IR.
 */
class JumpIfNotZeroInstruction : public Instruction {
  private:
    /**
     * The condition value of the jump instruction.
     */
    std::unique_ptr<Value> condition;
    /**
     * The target label of the jump instruction.
     */
    std::string target;

  public:
    /**
     * Constructor for creating a conditional jump instruction with a condition
     * value and target label.
     *
     * @param condition The condition value of the jump instruction.
     * @param target The target label of the jump instruction.
     * @throws std::logic_error if `condition` is null.
     */
    explicit JumpIfNotZeroInstruction(std::unique_ptr<Value> condition,
                                      std::string_view target);

    [[nodiscard]] Value *getCondition() const;

    [[nodiscard]] const std::string &getTarget() const;

    void setCondition(std::unique_ptr<Value> newCondition);

    void setTarget(std::string_view newTarget);
};

/**
 * Class representing a label instruction in the IR.
 */
class LabelInstruction : public Instruction {
  private:
    /**
     * The label of the instruction.
     */
    std::string label;

  public:
    /**
     * Constructor for creating a label instruction with a label.
     *
     * @param label The label of the instruction.
     */
    explicit LabelInstruction(std::string_view label);

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view newLabel);
};

/**
 * Class representing a function call instruction in the IR.
 */
class FunctionCallInstruction : public Instruction {
  private:
    /**
     * The function identifier of the instruction.
     */
    std::string functionIdentifier;
    /**
     * The argument values of the instruction.
     */
    std::unique_ptr<std::vector<std::unique_ptr<Value>>> args;
    /**
     * The destination value of the instruction.
     */
    std::unique_ptr<Value> dst;

  public:
    /**
     * Constructor for creating a function call instruction with a function
     * identifier, argument values, and a destination value.
     *
     * @param functionIdentifier The function identifier of the instruction.
     * @param args The argument values of the instruction.
     * @param dst The destination value of the instruction.
     * @throws std::logic_error if `args` or `dst` is null.
     */
    explicit FunctionCallInstruction(
        std::string_view functionIdentifier,
        std::unique_ptr<std::vector<std::unique_ptr<Value>>> args,
        std::unique_ptr<Value> dst);

    [[nodiscard]] const std::string &getFunctionIdentifier() const;

    [[nodiscard]] const std::vector<std::unique_ptr<Value>> &getArgs() const;

    [[nodiscard]] Value *getDst() const;

    void setFunctionIdentifier(std::string_view newFunctionIdentifier);

    void setArgs(std::unique_ptr<std::vector<std::unique_ptr<Value>>> newArgs);

    void setDst(std::unique_ptr<Value> newDst);
};

/**
 * Base class for representing a top-level construct in the IR.
 */
class TopLevel {
  public:
    /**
     * Default constructor for the IR top-level class.
     */
    constexpr TopLevel() = default;

    /**
     * Default virtual destructor for the IR top-level class.
     */
    virtual ~TopLevel() = default;

    /**
     * Delete the copy constructor for the IR top-level class.
     */
    constexpr TopLevel(const TopLevel &) = delete;

    /**
     * Delete the copy assignment operator for the IR top-level class.
     */
    constexpr TopLevel &operator=(const TopLevel &) = delete;

    /**
     * Default move constructor for the IR top-level class.
     */
    constexpr TopLevel(TopLevel &&) = default;

    /**
     * Default move assignment operator for the IR top-level class.
     */
    constexpr TopLevel &operator=(TopLevel &&) = default;
};

/**
 * Class representing a function definition in the IR.
 */
class FunctionDefinition : public TopLevel {
  private:
    /**
     * The function identifier of the function definition.
     */
    std::string functionIdentifier;
    /**
     * Boolean indicating whether the function is global.
     */
    bool global;
    /**
     * The parameter identifiers of the function definition.
     */
    std::unique_ptr<std::vector<std::string>> parameters;
    /**
     * The function body of the function definition.
     */
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> functionBody;

  public:
    /**
     * Constructor for creating a function definition with a function
     * identifier, global flag, parameter identifiers, and function body.
     *
     * @param functionIdentifier The function identifier of the function
     * definition.
     * @param global Boolean indicating whether the function is global.
     * @param parameters The parameter identifiers of the function definition.
     * @param functionBody The function body of the function definition.
     * @throws std::logic_error if `parameters` or `functionBody` is null.
     */
    explicit FunctionDefinition(
        std::string_view functionIdentifier, bool global,
        std::unique_ptr<std::vector<std::string>> parameters,
        std::unique_ptr<std::vector<std::unique_ptr<Instruction>>>
            functionBody);

    [[nodiscard]] const std::string &getFunctionIdentifier() const;

    [[nodiscard]] bool isGlobal() const;

    [[nodiscard]] const std::vector<std::string> &
    getParameterIdentifiers() const;

    [[nodiscard]] const std::vector<std::unique_ptr<Instruction>> &
    getFunctionBody() const;

    void
    setFunctionBody(std::unique_ptr<std::vector<std::unique_ptr<Instruction>>>
                        newFunctionBody);
};

/**
 * Class representing a static variable in the IR.
 */
class StaticVariable : public TopLevel {
  private:
    /**
     * The identifier of the static variable.
     */
    std::string identifier;
    /**
     * Boolean indicating whether the static variable is global.
     */
    bool global;
    /**
     * The type of the static variable.
     */
    std::unique_ptr<AST::Type> type;
    /**
     * The static initialization of the static variable.
     */
    std::unique_ptr<AST::StaticInit> staticInit;

  public:
    /**
     * Constructor for creating a static variable with an identifier, global
     * flag, type, and static initialization.
     *
     * @param identifier The identifier of the static variable.
     * @param global Boolean indicating whether the static variable is global.
     * @param type The type of the static variable.
     * @param staticInit The static initialization of the static variable.
     * @throws std::logic_error if `type` or `staticInit` is null.
     */
    explicit StaticVariable(std::string_view identifier, bool global,
                            std::unique_ptr<AST::Type> type,
                            std::unique_ptr<AST::StaticInit> staticInit);

    [[nodiscard]] const std::string &getIdentifier() const;

    [[nodiscard]] bool isGlobal() const;

    [[nodiscard]] const AST::Type *getType() const;

    [[nodiscard]] const AST::StaticInit *getStaticInit() const;
};

/**
 * Class representing a program in the IR.
 */
class Program {
  private:
    /**
     * The top-level constructs of the program.
     */
    std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels;

  public:
    /**
     * Constructor for creating a program with top-level constructs.
     *
     * @param topLevels The top-level constructs of the program.
     * @throws std::logic_error if `topLevels` is null.
     */
    explicit Program(
        std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels);

    [[nodiscard]] const std::vector<std::unique_ptr<TopLevel>> &
    getTopLevels() const;

    [[nodiscard]] std::vector<std::unique_ptr<TopLevel>> &getTopLevels();
};
} // namespace IR

#endif // MIDEND_IR_H
