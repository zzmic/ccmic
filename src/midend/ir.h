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
     * Default virtual destructor for the IR operator class.
     */
    virtual ~Operator() = default;
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
     * Default virtual destructor for the IR value class.
     */
    virtual ~Value() = default;
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
    explicit ConstantValue(std::unique_ptr<AST::Constant> astConstant)
        : astConstant(std::move(astConstant)) {
        if (!this->astConstant) {
            throw std::logic_error(
                "Creating ConstantValue with null astConstant");
        }
    }

    /**
     * Default destructor for the IR constant value class.
     */
    ~ConstantValue() = default;

    [[nodiscard]] const AST::Constant *getASTConstant() const {
        return astConstant.get();
    }

    void setASTConstant(std::unique_ptr<AST::Constant> newAstConstant) {
        if (!newAstConstant) {
            throw std::logic_error("Setting ConstantValue astConstant to null");
        }
        this->astConstant = std::move(newAstConstant);
    }
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
    explicit VariableValue(std::string_view identifier)
        : identifier(identifier) {}

    /**
     * Default destructor for the IR variable value class.
     */
    ~VariableValue() = default;

    [[nodiscard]] const std::string &getIdentifier() const {
        return identifier;
    }

    void setIdentifier(std::string_view newIdentifier) {
        this->identifier = newIdentifier;
    }
};

/**
 * Base class for representing an instruction in the IR.
 */
class Instruction {
  public:
    /**
     * Default virtual destructor for the IR instruction class.
     */
    virtual ~Instruction() = default;
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
    explicit ReturnInstruction(std::unique_ptr<Value> returnValue)
        : returnValue(std::move(returnValue)) {
        if (!this->returnValue) {
            throw std::logic_error(
                "Creating ReturnInstruction with null returnValue");
        }
    }

    [[nodiscard]] Value *getReturnValue() const { return returnValue.get(); }

    void setReturnValue(std::unique_ptr<Value> newReturnValue) {
        if (!newReturnValue) {
            throw std::logic_error(
                "Setting ReturnInstruction returnValue to null");
        }
        this->returnValue = std::move(newReturnValue);
    }
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
    SignExtendInstruction(std::unique_ptr<Value> src,
                          std::unique_ptr<Value> dst)
        : src(std::move(src)), dst(std::move(dst)) {
        if (!this->src) {
            throw std::logic_error("Creating SignExtendInstruction with null "
                                   "src");
        }
        if (!this->dst) {
            throw std::logic_error("Creating SignExtendInstruction with null "
                                   "dst");
        }
    }

    [[nodiscard]] Value *getSrc() const { return src.get(); }

    [[nodiscard]] Value *getDst() const { return dst.get(); }

    void setSrc(std::unique_ptr<Value> newSrc) {
        if (!newSrc) {
            throw std::logic_error("Setting SignExtendInstruction src to null");
        }
        this->src = std::move(newSrc);
    }

    void setDst(std::unique_ptr<Value> newDst) {
        if (!newDst) {
            throw std::logic_error("Setting SignExtendInstruction dst to null");
        }
        this->dst = std::move(newDst);
    }
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
    TruncateInstruction(std::unique_ptr<Value> src, std::unique_ptr<Value> dst)
        : src(std::move(src)), dst(std::move(dst)) {
        if (!this->src) {
            throw std::logic_error(
                "Creating TruncateInstruction with null src");
        }
        if (!this->dst) {
            throw std::logic_error(
                "Creating TruncateInstruction with null dst");
        }
    }

    [[nodiscard]] Value *getSrc() const { return src.get(); }

    [[nodiscard]] Value *getDst() const { return dst.get(); }

    void setSrc(std::unique_ptr<Value> newSrc) {
        if (!newSrc) {
            throw std::logic_error("Setting TruncateInstruction src to null");
        }
        this->src = std::move(newSrc);
    }

    void setDst(std::unique_ptr<Value> newDst) {
        if (!newDst) {
            throw std::logic_error("Setting TruncateInstruction dst to null");
        }
        this->dst = std::move(newDst);
    }
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
    UnaryInstruction(std::unique_ptr<UnaryOperator> unaryOperator,
                     std::unique_ptr<Value> src, std::unique_ptr<Value> dst)
        : unaryOperator(std::move(unaryOperator)), src(std::move(src)),
          dst(std::move(dst)) {
        if (!this->unaryOperator) {
            throw std::logic_error(
                "Creating UnaryInstruction with null unaryOperator");
        }
        if (!this->src) {
            throw std::logic_error("Creating UnaryInstruction with null src");
        }
        if (!this->dst) {
            throw std::logic_error("Creating UnaryInstruction with null dst");
        }
    }

    [[nodiscard]] UnaryOperator *getUnaryOperator() const {
        return unaryOperator.get();
    }

    [[nodiscard]] Value *getSrc() const { return src.get(); }

    [[nodiscard]] Value *getDst() const { return dst.get(); }

    void setUnaryOperator(std::unique_ptr<UnaryOperator> newUnaryOperator) {
        if (!newUnaryOperator) {
            throw std::logic_error(
                "Setting UnaryInstruction unaryOperator to null");
        }
        this->unaryOperator = std::move(newUnaryOperator);
    }

    void setSrc(std::unique_ptr<Value> newSrc) {
        if (!newSrc) {
            throw std::logic_error("Setting UnaryInstruction src to null");
        }
        this->src = std::move(newSrc);
    }

    void setDst(std::unique_ptr<Value> newDst) {
        if (!newDst) {
            throw std::logic_error("Setting UnaryInstruction dst to null");
        }
        this->dst = std::move(newDst);
    }
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
    BinaryInstruction(std::unique_ptr<BinaryOperator> binaryOperator,
                      std::unique_ptr<Value> src1, std::unique_ptr<Value> src2,
                      std::unique_ptr<Value> dst)
        : binaryOperator(std::move(binaryOperator)), src1(std::move(src1)),
          src2(std::move(src2)), dst(std::move(dst)) {
        if (!this->binaryOperator) {
            throw std::logic_error(
                "Creating BinaryInstruction with null binaryOperator");
        }
        if (!this->src1) {
            throw std::logic_error("Creating BinaryInstruction with null src1");
        }
        if (!this->src2) {
            throw std::logic_error("Creating BinaryInstruction with null src2");
        }
        if (!this->dst) {
            throw std::logic_error("Creating BinaryInstruction with null dst");
        }
    }

    [[nodiscard]] BinaryOperator *getBinaryOperator() const {
        return binaryOperator.get();
    }

    [[nodiscard]] Value *getSrc1() const { return src1.get(); }

    [[nodiscard]] Value *getSrc2() const { return src2.get(); }

    [[nodiscard]] Value *getDst() const { return dst.get(); }

    void setBinaryOperator(std::unique_ptr<BinaryOperator> newBinaryOperator) {
        if (!newBinaryOperator) {
            throw std::logic_error(
                "Setting BinaryInstruction binaryOperator to null");
        }
        this->binaryOperator = std::move(newBinaryOperator);
    }

    void setSrc1(std::unique_ptr<Value> newSrc1) {
        if (!newSrc1) {
            throw std::logic_error("Setting BinaryInstruction src1 to null");
        }
        this->src1 = std::move(newSrc1);
    }

    void setSrc2(std::unique_ptr<Value> newSrc2) {
        if (!newSrc2) {
            throw std::logic_error("Setting BinaryInstruction src2 to null");
        }
        this->src2 = std::move(newSrc2);
    }

    void setDst(std::unique_ptr<Value> newDst) {
        if (!newDst) {
            throw std::logic_error("Setting BinaryInstruction dst to null");
        }
        this->dst = std::move(newDst);
    }
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
    CopyInstruction(std::unique_ptr<Value> src, std::unique_ptr<Value> dst)
        : src(std::move(src)), dst(std::move(dst)) {
        if (!this->src) {
            throw std::logic_error("Creating CopyInstruction with null src");
        }
        if (!this->dst) {
            throw std::logic_error("Creating CopyInstruction with null dst");
        }
    }

    [[nodiscard]] Value *getSrc() const { return src.get(); }

    [[nodiscard]] Value *getDst() const { return dst.get(); }

    void setSrc(std::unique_ptr<Value> newSrc) {
        if (!newSrc) {
            throw std::logic_error("Setting CopyInstruction src to null");
        }
        this->src = std::move(newSrc);
    }

    void setDst(std::unique_ptr<Value> newDst) {
        if (!newDst) {
            throw std::logic_error("Setting CopyInstruction dst to null");
        }
        this->dst = std::move(newDst);
    }
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
    explicit JumpInstruction(std::string_view target) : target(target) {}

    [[nodiscard]] const std::string &getTarget() const { return target; }

    void setTarget(std::string_view newTarget) { this->target = newTarget; }
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
    JumpIfZeroInstruction(std::unique_ptr<Value> condition,
                          std::string_view target)
        : condition(std::move(condition)), target(target) {
        if (!this->condition) {
            throw std::logic_error(
                "Creating JumpIfZeroInstruction with null condition");
        }
    }

    [[nodiscard]] Value *getCondition() const { return condition.get(); }

    [[nodiscard]] const std::string &getTarget() const { return target; }

    void setCondition(std::unique_ptr<Value> newCondition) {
        if (!newCondition) {
            throw std::logic_error(
                "Setting JumpIfZeroInstruction condition to null");
        }
        this->condition = std::move(newCondition);
    }

    void setTarget(std::string_view newTarget) { this->target = newTarget; }
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
    JumpIfNotZeroInstruction(std::unique_ptr<Value> condition,
                             std::string_view target)
        : condition(std::move(condition)), target(target) {
        if (!this->condition) {
            throw std::logic_error(
                "Creating JumpIfNotZeroInstruction with null condition");
        }
    }

    [[nodiscard]] Value *getCondition() const { return condition.get(); }

    [[nodiscard]] const std::string &getTarget() const { return target; }

    void setCondition(std::unique_ptr<Value> newCondition) {
        if (!newCondition) {
            throw std::logic_error(
                "Setting JumpIfNotZeroInstruction condition to null");
        }
        this->condition = std::move(newCondition);
    }

    void setTarget(std::string_view newTarget) { this->target = newTarget; }
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
    explicit LabelInstruction(std::string_view label) : label(label) {}

    [[nodiscard]] const std::string &getLabel() const { return label; }

    void setLabel(std::string_view newLabel) { this->label = newLabel; }
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
    FunctionCallInstruction(
        std::string_view functionIdentifier,
        std::unique_ptr<std::vector<std::unique_ptr<Value>>> args,
        std::unique_ptr<Value> dst)
        : functionIdentifier(functionIdentifier), args(std::move(args)),
          dst(std::move(dst)) {
        if (!this->args) {
            throw std::logic_error(
                "Creating FunctionCallInstruction with null args");
        }
        if (!this->dst) {
            throw std::logic_error(
                "Creating FunctionCallInstruction with null dst");
        }
    }

    [[nodiscard]] const std::string &getFunctionIdentifier() const {
        return functionIdentifier;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Value>> &getArgs() const {
        return *args;
    }

    [[nodiscard]] Value *getDst() const { return dst.get(); }

    void setFunctionIdentifier(std::string_view newFunctionIdentifier) {
        this->functionIdentifier = newFunctionIdentifier;
    }

    void setArgs(std::unique_ptr<std::vector<std::unique_ptr<Value>>> newArgs) {
        if (!newArgs) {
            throw std::logic_error(
                "Setting FunctionCallInstruction args to null");
        }
        this->args = std::move(newArgs);
    }

    void setDst(std::unique_ptr<Value> newDst) {
        if (!newDst) {
            throw std::logic_error(
                "Setting FunctionCallInstruction dst to null");
        }
        this->dst = std::move(newDst);
    }
};

/**
 * Base class for representing a top-level construct in the IR.
 */
class TopLevel {
  public:
    /**
     * Default virtual destructor for the IR top-level class.
     */
    virtual ~TopLevel() = default;
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
    FunctionDefinition(
        std::string_view functionIdentifier, bool global,
        std::unique_ptr<std::vector<std::string>> parameters,
        std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> functionBody)
        : functionIdentifier(functionIdentifier), global(global),
          parameters(std::move(parameters)),
          functionBody(std::move(functionBody)) {
        if (!this->parameters) {
            throw std::logic_error(
                "Creating FunctionDefinition with null parameters");
        }
        if (!this->functionBody) {
            throw std::logic_error(
                "Creating FunctionDefinition with null functionBody");
        }
    }

    [[nodiscard]] const std::string &getFunctionIdentifier() const {
        return functionIdentifier;
    }

    [[nodiscard]] bool isGlobal() const { return global; }

    [[nodiscard]] const std::vector<std::string> &
    getParameterIdentifiers() const {
        return *parameters;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Instruction>> &
    getFunctionBody() const {
        return *functionBody;
    }

    void
    setFunctionBody(std::unique_ptr<std::vector<std::unique_ptr<Instruction>>>
                        newFunctionBody) {
        if (!newFunctionBody) {
            throw std::logic_error(
                "Setting FunctionDefinition functionBody to null");
        }
        this->functionBody = std::move(newFunctionBody);
    }
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
    StaticVariable(std::string_view identifier, bool global,
                   std::unique_ptr<AST::Type> type,
                   std::unique_ptr<AST::StaticInit> staticInit)
        : identifier(identifier), global(global), type(std::move(type)),
          staticInit(std::move(staticInit)) {
        if (!this->type) {
            throw std::logic_error("Creating StaticVariable with null type");
        }
        if (!this->staticInit) {
            throw std::logic_error(
                "Creating StaticVariable with null staticInit");
        }
    }

    [[nodiscard]] const std::string &getIdentifier() const {
        return identifier;
    }

    [[nodiscard]] bool isGlobal() const { return global; }

    [[nodiscard]] const AST::Type *getType() const { return type.get(); }

    [[nodiscard]] const AST::StaticInit *getStaticInit() const {
        return staticInit.get();
    }
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
        std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels)
        : topLevels(std::move(topLevels)) {
        if (!this->topLevels) {
            throw std::logic_error("Creating Program with null topLevels");
        }
    }

    [[nodiscard]] const std::vector<std::unique_ptr<TopLevel>> &
    getTopLevels() const {
        return *topLevels;
    }

    [[nodiscard]] std::vector<std::unique_ptr<TopLevel>> &getTopLevels() {
        return *topLevels;
    }
};
} // namespace IR

#endif // MIDEND_IR_H
