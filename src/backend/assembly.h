#ifndef BACKEND_ASSEMBLY_H
#define BACKEND_ASSEMBLY_H

#include "../frontend/semanticAnalysisPasses.h"
#include "../utils/constants.h"
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Assembly {
/**
 * Base class for representing assembly registers.
 */
class Register {
  public:
    /**
     * Default constructor for the assembly register class.
     */
    constexpr Register() = default;

    /**
     * Default virtual destructor for the assembly register class.
     */
    virtual ~Register() = default;

    /**
     * Delete the copy constructor for the assembly register class.
     */
    constexpr Register(const Register &) = delete;

    /**
     * Delete the copy assignment operator for the assembly register class.
     */
    constexpr Register &operator=(const Register &) = delete;

    /**
     * Default move constructor for the assembly register class.
     */
    constexpr Register(Register &&) = default;

    /**
     * Default move assignment operator for the assembly register class.
     */
    constexpr Register &operator=(Register &&) = default;
};

/**
 * Class for representing the AX assembly register.
 */
class AX : public Register {};

/**
 * Class for representing the CX assembly register.
 */
class CX : public Register {};

/**
 * Class for representing the DX assembly register.
 */
class DX : public Register {};

/**
 * Class for representing the DI assembly register.
 */
class DI : public Register {};

/**
 * Class for representing the SI assembly register.
 */
class SI : public Register {};

/**
 * Class for representing the R8 assembly register.
 */
class R8 : public Register {};

/**
 * Class for representing the R9 assembly register.
 */
class R9 : public Register {};

/**
 * Class for representing the R10 assembly register.
 */
class R10 : public Register {};

/**
 * Class for representing the R11 assembly register.
 */
class R11 : public Register {};

/**
 * Class for representing a reserved assembly register.
 */
class ReservedRegister : public Register {};

/**
 * Class for representing the SP reserved assembly register.
 */
class SP : public ReservedRegister {};

/**
 * Class for representing the BP reserved assembly register.
 */
class BP : public ReservedRegister {};

/**
 * Base class for representing an assembly operand.
 */
class Operand {
  public:
    /**
     * Default constructor for the assembly operand class.
     */
    constexpr Operand() = default;

    /**
     * Default virtual destructor for the assembly operand class.
     */
    virtual ~Operand() = default;

    /**
     * Delete the copy constructor for the assembly operand class.
     */
    constexpr Operand(const Operand &) = delete;

    /**
     * Delete the copy assignment operator for the assembly operand class.
     */
    constexpr Operand &operator=(const Operand &) = delete;

    /**
     * Default move constructor for the assembly operand class.
     */
    constexpr Operand(Operand &&) = default;

    /**
     * Default move assignment operator for the assembly operand class.
     */
    constexpr Operand &operator=(Operand &&) = default;

    /**
     * Get the immediate value of the operand.
     *
     * @return The immediate value of the operand.
     */
    [[nodiscard]] virtual unsigned long getImmediate() const;

    /**
     * Get the register of the operand.
     *
     * @return The register of the operand.
     */
    [[nodiscard]] virtual Register *getRegister() const;

    /**
     * Get the reserved register of the operand.
     *
     * @return The reserved register of the operand.
     */
    [[nodiscard]] virtual ReservedRegister *getReservedRegister() const;

    /**
     * Get the pseudo register of the operand.
     *
     * @return The pseudo register of the operand.
     */
    [[nodiscard]] virtual std::string getPseudoRegister() const;

    /**
     * Get the offset of the operand.
     *
     * @return The offset of the operand.
     */
    [[nodiscard]] virtual int getOffset() const;

    /**
     * Get the identifier of the operand.
     *
     * @return The identifier of the operand.
     */
    [[nodiscard]] virtual std::string getIdentifier() const;
};

/**
 * Class for representing an immediate operand.
 *
 * The immediate value is stored as an `unsigned long` to preserve all 64-bit
 * patterns.
 * The signedness of the immediate value is determined by the instruction that
 * uses it rather than the value itself.
 */
class ImmediateOperand : public Operand {
  private:
    /**
     * The immediate value of the operand (stored as unsigned to preserve bit
     * patterns).
     */
    unsigned long imm = 0;

  public:
    /**
     * Constructor for the immediate operand class.
     *
     * @param imm The immediate value of the operand.
     */
    explicit ImmediateOperand(int imm);

    /**
     * Constructor for the immediate operand class.
     *
     * @param imm The immediate value of the operand.
     */
    explicit ImmediateOperand(long imm);

    /**
     * Constructor for the immediate operand class.
     *
     * @param imm The immediate value of the operand.
     */
    explicit ImmediateOperand(unsigned int imm);

    /**
     * Constructor for the immediate operand class.
     *
     * @param imm The immediate value of the operand.
     */
    explicit ImmediateOperand(unsigned long imm);

    [[nodiscard]] unsigned long getImmediate() const override;
};

/**
 * Class for representing a register operand.
 */
class RegisterOperand : public Operand {
  private:
    /**
     * The register of the operand.
     */
    std::unique_ptr<Register> reg;

    /**
     * The mapping of register sizes to register names.
     */
    std::unordered_map<int, std::unordered_map<std::type_index, std::string>>
        regMappings = {{1, // 1-byte registers.
                        {{typeid(AX), "%al"},
                         {typeid(CX), "%cl"},
                         {typeid(DX), "%dl"},
                         {typeid(DI), "%dil"},
                         {typeid(SI), "%sil"},
                         {typeid(R8), "%r8b"},
                         {typeid(R9), "%r9b"},
                         {typeid(R10), "%r10b"},
                         {typeid(R11), "%r11b"},
                         {typeid(SP), "%spl"},
                         {typeid(BP), "%bpl"}}},
                       {LONGWORD_SIZE, // 4-byte registers.
                        {{typeid(AX), "%eax"},
                         {typeid(CX), "%ecx"},
                         {typeid(DX), "%edx"},
                         {typeid(DI), "%edi"},
                         {typeid(SI), "%esi"},
                         {typeid(R8), "%r8d"},
                         {typeid(R9), "%r9d"},
                         {typeid(R10), "%r10d"},
                         {typeid(R11), "%r11d"},
                         {typeid(SP), "%esp"},
                         {typeid(BP), "%ebp"}}},
                       {QUADWORD_SIZE, // 8-byte registers.
                        {{typeid(AX), "%rax"},
                         {typeid(CX), "%rcx"},
                         {typeid(DX), "%rdx"},
                         {typeid(DI), "%rdi"},
                         {typeid(SI), "%rsi"},
                         {typeid(R8), "%r8"},
                         {typeid(R9), "%r9"},
                         {typeid(R10), "%r10"},
                         {typeid(R11), "%r11"},
                         {typeid(SP), "%rsp"},
                         {typeid(BP), "%rbp"}}}};

  public:
    /**
     * Constructor for the register operand class.
     *
     * @param reg The register of the operand.
     */
    explicit RegisterOperand(std::unique_ptr<Register> reg);

    /**
     * Constructor for the register operand class.
     *
     * @param regInStr The string representation of the register.
     */
    explicit RegisterOperand(const std::string &regInStr);

    [[nodiscard]] Register *getRegister() const override;

    [[nodiscard]] std::string getRegisterInBytesInStr(int size) const;
};

/**
 * Class for representing a pseudo register operand.
 */
class PseudoRegisterOperand : public Operand {
  private:
    /**
     * The pseudo register of the operand.
     */
    std::string pseudoReg;

  public:
    /**
     * Constructor for the pseudo register operand class.
     *
     * @param pseudoReg The pseudo register of the operand.
     */
    explicit PseudoRegisterOperand(std::string pseudoReg);

    [[nodiscard]] std::string getPseudoRegister() const override;
};

/**
 * Class for representing a stack operand.
 */
class StackOperand : public Operand {
  private:
    /**
     * The offset of the operand.
     */
    int offset = 0;

    /**
     * The reserved register of the operand.
     */
    std::unique_ptr<ReservedRegister> reservedReg;

  public:
    /**
     * Constructor for the stack operand class.
     *
     * @param offset The offset of the operand.
     * @param reservedReg The reserved register of the operand.
     */
    explicit StackOperand(int offset,
                          std::unique_ptr<ReservedRegister> reservedReg);

    [[nodiscard]] int getOffset() const override;

    [[nodiscard]] ReservedRegister *getReservedRegister() const override;

    [[nodiscard]] std::string getReservedRegisterInStr() const;
};

/**
 * Class for representing a data operand.
 */
class DataOperand : public Operand {
  private:
    /**
     * The identifier of the operand.
     */
    std::string identifier;

  public:
    /**
     * Constructor for the data operand class.
     *
     * @param identifier The identifier of the operand.
     */
    explicit DataOperand(std::string identifier);

    [[nodiscard]] std::string getIdentifier() const override;
};

/**
 * Base class for representing a condition code.
 */
class CondCode {
  public:
    /**
     * Default constructor for the condition code class.
     */
    constexpr CondCode() = default;

    /**
     * Default virtual destructor for the condition code class.
     */
    virtual ~CondCode() = default;

    /**
     * Delete the copy constructor for the condition code class.
     */
    constexpr CondCode(const CondCode &) = delete;

    /**
     * Delete the copy assignment operator for the condition code class.
     */
    constexpr CondCode &operator=(const CondCode &) = delete;

    /**
     * Default move constructor for the condition code class.
     */
    constexpr CondCode(CondCode &&) = default;

    /**
     * Default move assignment operator for the condition code class.
     */
    constexpr CondCode &operator=(CondCode &&) = default;
};

/**
 * Class for representing the `E` (equal) condition code.
 */
class E : public CondCode {};

/**
 * Class for representing the `NE` (not equal) condition code.
 */
class NE : public CondCode {};

/**
 * Class for representing the `G` (greater) condition code.
 */
class G : public CondCode {};

/**
 * Class for representing the `GE` (greater or equal) condition code.
 */
class GE : public CondCode {};

/**
 * Class for representing the `L` (less) condition code.
 */
class L : public CondCode {};

/**
 * Class for representing the `LE` (less or equal) condition code.
 */
class LE : public CondCode {};

/**
 * Class for representing the `A` (above) condition code.
 */
class A : public CondCode {};

/**
 * Class for representing the `AE` (above or equal) condition code.
 */
class AE : public CondCode {};

/**
 * Class for representing the `B` (below) condition code.
 */
class B : public CondCode {};

/**
 * Class for representing the `BE` (below or equal) condition code.
 */
class BE : public CondCode {};

/**
 * Base class for representing an operator.
 */
class Operator {
  public:
    /**
     * Default constructor for the operator class.
     */
    constexpr Operator() = default;

    /**
     * Default virtual destructor for the operator class.
     */
    virtual ~Operator() = default;

    /**
     * Delete the copy constructor for the operator class.
     */
    constexpr Operator(const Operator &) = delete;

    /**
     * Delete the copy assignment operator for the operator class.
     */
    constexpr Operator &operator=(const Operator &) = delete;

    /**
     * Default move constructor for the operator class.
     */
    constexpr Operator(Operator &&) = default;

    /**
     * Default move assignment operator for the operator class.
     */
    constexpr Operator &operator=(Operator &&) = default;
};

/**
 * Base class for representing a unary operator.
 */
class UnaryOperator : public Operator {};

/**
 * Class for representing the negate unary operator.
 */
class NegateOperator : public UnaryOperator {};

/**
 * Class for representing the complement unary operator.
 */
class ComplementOperator : public UnaryOperator {};

/**
 * Class for representing the not unary operator.
 */
class NotOperator : public UnaryOperator {};

/**
 * Base class for representing a binary operator.
 */
class BinaryOperator : public Operator {};

/**
 * Class for representing the add binary operator.
 */
class AddOperator : public BinaryOperator {};

/**
 * Class for representing the subtract binary operator.
 */
class SubtractOperator : public BinaryOperator {};

/**
 * Class for representing the multiply binary operator.
 */
class MultiplyOperator : public BinaryOperator {};

/**
 * Base class for representing an assembly type.
 */
class AssemblyType {
  public:
    /**
     * Default constructor for the assembly type class.
     */
    constexpr AssemblyType() = default;

    /**
     * Default virtual destructor for the assembly type class.
     */
    virtual ~AssemblyType() = default;

    /**
     * Delete the copy constructor for the assembly type class.
     */
    constexpr AssemblyType(const AssemblyType &) = delete;

    /**
     * Delete the copy assignment operator for the assembly type class.
     */
    constexpr AssemblyType &operator=(const AssemblyType &) = delete;

    /**
     * Default move constructor for the assembly type class.
     */
    constexpr AssemblyType(AssemblyType &&) = default;

    /**
     * Default move assignment operator for the assembly type class.
     */
    constexpr AssemblyType &operator=(AssemblyType &&) = default;
};

/**
 * Class for representing the longword assembly type.
 */
class Longword : public AssemblyType {};

/**
 * Class for representing the quadword assembly type.
 */

class Quadword : public AssemblyType {};

/**
 * Base class for representing an instruction.
 */
class Instruction {
  public:
    /**
     * Default constructor for the instruction class.
     */
    constexpr Instruction() = default;

    /**
     * Default virtual destructor for the instruction class.
     */
    virtual ~Instruction() = default;

    /**
     * Delete the copy constructor for the instruction class.
     */
    constexpr Instruction(const Instruction &) = delete;

    /**
     * Delete the copy assignment operator for the instruction class.
     */
    constexpr Instruction &operator=(const Instruction &) = delete;

    /**
     * Default move constructor for the instruction class.
     */
    constexpr Instruction(Instruction &&) = default;

    /**
     * Default move assignment operator for the instruction class.
     */
    constexpr Instruction &operator=(Instruction &&) = default;
};

/**
 * Class for representing the mov instruction.
 */
class MovInstruction : public Instruction {
  private:
    /**
     * The type of the instruction.
     */
    std::unique_ptr<AssemblyType> type;

    /**
     * The source and destination operands of the instruction.
     */
    std::unique_ptr<Operand> src, dst;

  public:
    /**
     * Constructor for the mov instruction class.
     *
     * @param type The type of the instruction.
     * @param src The source operand of the instruction.
     * @param dst The destination operand of the instruction.
     */
    explicit MovInstruction(std::unique_ptr<AssemblyType> type,
                            std::unique_ptr<Operand> src,
                            std::unique_ptr<Operand> dst);

    [[nodiscard]] const AssemblyType *getType() const;

    [[nodiscard]] const Operand *getSrc() const;

    [[nodiscard]] const Operand *getDst() const;

    void setType(std::unique_ptr<AssemblyType> newType);

    void setSrc(std::unique_ptr<Operand> newSrc);

    void setDst(std::unique_ptr<Operand> newDst);
};

/**
 * Class for representing the movsx instruction.
 */
class MovsxInstruction : public Instruction {
  private:
    /**
     * The source and destination operands of the instruction.
     */
    std::unique_ptr<Operand> src, dst;

  public:
    /**
     * Constructor for the movsx instruction class.
     *
     * @param src The source operand of the instruction.
     * @param dst The destination operand of the instruction.
     */
    explicit MovsxInstruction(std::unique_ptr<Operand> src,
                              std::unique_ptr<Operand> dst);

    [[nodiscard]] const Operand *getSrc() const;

    [[nodiscard]] const Operand *getDst() const;

    void setSrc(std::unique_ptr<Operand> newSrc);

    void setDst(std::unique_ptr<Operand> newDst);
};

/**
 * Class for representing the movzeroextend instruction.
 */
class MovZeroExtendInstruction : public Instruction {
  private:
    /**
     * The source and destination operands of the instruction.
     */
    std::unique_ptr<Operand> src, dst;

  public:
    /**
     * Constructor for the movzeroextend instruction class.
     *
     * @param src The source operand of the instruction.
     * @param dst The destination operand of the instruction.
     */
    explicit MovZeroExtendInstruction(std::unique_ptr<Operand> src,
                                      std::unique_ptr<Operand> dst);

    [[nodiscard]] const Operand *getSrc() const;

    [[nodiscard]] const Operand *getDst() const;

    void setSrc(std::unique_ptr<Operand> newSrc);

    void setDst(std::unique_ptr<Operand> newDst);
};

/**
 * Class for representing the unary instruction.
 */
class UnaryInstruction : public Instruction {
  private:
    /**
     * The unary operator of the instruction.
     */
    std::unique_ptr<UnaryOperator> unaryOperator;

    /**
     * The type of the instruction.
     */
    std::unique_ptr<AssemblyType> type;

    /**
     * The operand of the instruction.
     */
    std::unique_ptr<Operand> operand;

  public:
    /**
     * Constructor for the unary instruction class.
     *
     * @param unaryOperator The unary operator of the instruction.
     * @param type The type of the instruction.
     * @param operand The operand of the instruction.
     */
    explicit UnaryInstruction(std::unique_ptr<UnaryOperator> unaryOperator,
                              std::unique_ptr<AssemblyType> type,
                              std::unique_ptr<Operand> operand);

    [[nodiscard]] const UnaryOperator *getUnaryOperator() const;

    [[nodiscard]] const AssemblyType *getType() const;

    [[nodiscard]] const Operand *getOperand() const;

    void setUnaryOperator(std::unique_ptr<UnaryOperator> newUnaryOperator);

    void setType(std::unique_ptr<AssemblyType> newType);

    void setOperand(std::unique_ptr<Operand> newOperand);
};

/**
 * Class for representing the binary instruction.
 */
class BinaryInstruction : public Instruction {
  private:
    /**
     * The binary operator of the instruction.
     */
    std::unique_ptr<BinaryOperator> binaryOperator;

    /**
     * The type of the instruction.
     */
    std::unique_ptr<AssemblyType> type;

    /**
     * The first and second operands of the instruction.
     */
    std::unique_ptr<Operand> operand1, operand2;

  public:
    /**
     * Constructor for the binary instruction class.
     *
     * @param binaryOperator The binary operator of the instruction.
     * @param type The type of the instruction.
     * @param operand1 The first operand of the instruction.
     * @param operand2 The second operand of the instruction.
     */
    explicit BinaryInstruction(std::unique_ptr<BinaryOperator> binaryOperator,
                               std::unique_ptr<AssemblyType> type,
                               std::unique_ptr<Operand> operand1,
                               std::unique_ptr<Operand> operand2);

    [[nodiscard]] const BinaryOperator *getBinaryOperator() const;

    [[nodiscard]] const AssemblyType *getType() const;

    [[nodiscard]] const Operand *getOperand1() const;

    [[nodiscard]] const Operand *getOperand2() const;

    void setBinaryOperator(std::unique_ptr<BinaryOperator> newBinaryOperator);

    void setType(std::unique_ptr<AssemblyType> newType);

    void setOperand1(std::unique_ptr<Operand> newOperand1);

    void setOperand2(std::unique_ptr<Operand> newOperand2);
};

/**
 * Class for representing the cmp instruction.
 */
class CmpInstruction : public Instruction {
  private:
    /**
     * The type of the instruction.
     */
    std::unique_ptr<AssemblyType> type;

    /**
     * The first and second operands of the instruction.
     */
    std::unique_ptr<Operand> operand1, operand2;

  public:
    /**
     * Constructor for the cmp instruction class.
     *
     * @param type The type of the instruction.
     * @param operand1 The first operand of the instruction.
     * @param operand2 The second operand of the instruction.
     */
    explicit CmpInstruction(std::unique_ptr<AssemblyType> type,
                            std::unique_ptr<Operand> operand1,
                            std::unique_ptr<Operand> operand2);

    [[nodiscard]] const AssemblyType *getType() const;

    [[nodiscard]] const Operand *getOperand1() const;

    [[nodiscard]] const Operand *getOperand2() const;

    void setType(std::unique_ptr<AssemblyType> newType);

    void setOperand1(std::unique_ptr<Operand> newOperand1);

    void setOperand2(std::unique_ptr<Operand> newOperand2);
};

/**
 * Class for representing the idiv instruction.
 */
class IdivInstruction : public Instruction {
  private:
    /**
     * The type of the instruction.
     */
    std::unique_ptr<AssemblyType> type;

    /**
     * The operand of the instruction.
     */
    std::unique_ptr<Operand> operand;

  public:
    /**
     * Constructor for the idiv instruction class.
     *
     * @param type The type of the instruction.
     * @param operand The operand of the instruction.
     */
    explicit IdivInstruction(std::unique_ptr<AssemblyType> type,
                             std::unique_ptr<Operand> operand);

    [[nodiscard]] const AssemblyType *getType() const;

    [[nodiscard]] const Operand *getOperand() const;

    void setType(std::unique_ptr<AssemblyType> newType);

    void setOperand(std::unique_ptr<Operand> newOperand);
};

/**
 * Class for representing the div instruction.
 */
class DivInstruction : public Instruction {
  private:
    /**
     * The type of the instruction.
     */
    std::unique_ptr<AssemblyType> type;

    /**
     * The operand of the instruction.
     */
    std::unique_ptr<Operand> operand;

  public:
    /**
     * Constructor for the div instruction class.
     *
     * @param type The type of the instruction.
     * @param operand The operand of the instruction.
     */
    explicit DivInstruction(std::unique_ptr<AssemblyType> type,
                            std::unique_ptr<Operand> operand);

    [[nodiscard]] const AssemblyType *getType() const;

    [[nodiscard]] const Operand *getOperand() const;

    void setType(std::unique_ptr<AssemblyType> newType);

    void setOperand(std::unique_ptr<Operand> newOperand);
};

/**
 * Class for representing the cdq instruction.
 */
class CdqInstruction : public Instruction {
  private:
    /**
     * The type of the instruction.
     */
    std::unique_ptr<AssemblyType> type;

  public:
    /**
     * Constructor for the cdq instruction class.
     *
     * @param type The type of the instruction.
     */
    explicit CdqInstruction(std::unique_ptr<AssemblyType> type);

    [[nodiscard]] const AssemblyType *getType() const;

    void setType(std::unique_ptr<AssemblyType> newType);
};

/**
 * Class for representing the jmp instruction.
 */
class JmpInstruction : public Instruction {
  private:
    /**
     * The label of the instruction.
     */
    std::string label;

  public:
    /**
     * Constructor for the jmp instruction class.
     *
     * @param label The label of the instruction.
     */
    explicit JmpInstruction(std::string label);

    [[nodiscard]] std::string getLabel() const;

    void setLabel(std::string newLabel);
};

/**
 * Class for representing the jmpcc instruction.
 */
class JmpCCInstruction : public Instruction {
  private:
    /**
     * The condition code of the instruction.
     */
    std::unique_ptr<CondCode> condCode;

    /**
     * The label of the instruction.
     */
    std::string label;

  public:
    /**
     * Constructor for the jmpcc instruction class.
     *
     * @param condCode The condition code of the instruction.
     * @param label The label of the instruction.
     */
    explicit JmpCCInstruction(std::unique_ptr<CondCode> condCode,
                              std::string label);

    [[nodiscard]] const CondCode *getCondCode() const;

    [[nodiscard]] std::string getLabel() const;

    void setCondCode(std::unique_ptr<CondCode> newCondCode);

    void setLabel(std::string newLabel);
};

/**
 * Class for representing the setcc instruction.
 */
class SetCCInstruction : public Instruction {
  private:
    /**
     * The condition code of the instruction.
     */
    std::unique_ptr<CondCode> condCode;

    /**
     * The operand of the instruction.
     */
    std::unique_ptr<Operand> operand;

  public:
    /**
     * Constructor for the setcc instruction class.
     *
     * @param condCode The condition code of the instruction.
     * @param operand The operand of the instruction.
     */
    explicit SetCCInstruction(std::unique_ptr<CondCode> condCode,
                              std::unique_ptr<Operand> operand);

    [[nodiscard]] const CondCode *getCondCode() const;

    [[nodiscard]] const Operand *getOperand() const;

    void setCondCode(std::unique_ptr<CondCode> newCondCode);

    void setOperand(std::unique_ptr<Operand> newOperand);
};

/**
 * Class for representing the label instruction.
 */
class LabelInstruction : public Instruction {
  private:
    /**
     * The label of the instruction.
     */
    std::string label;

  public:
    /**
     * Constructor for the label instruction class.
     *
     * @param label The label of the instruction.
     */
    explicit LabelInstruction(std::string label);

    [[nodiscard]] std::string getLabel() const;

    void setLabel(std::string newLabel);
};

/**
 * Class for representing the push instruction.
 */
class PushInstruction : public Instruction {
  private:
    /**
     * The operand of the instruction.
     */
    std::unique_ptr<Operand> operand;

  public:
    /**
     * Constructor for the push instruction class.
     *
     * @param operand The operand of the instruction.
     */
    explicit PushInstruction(std::unique_ptr<Operand> operand);

    [[nodiscard]] const Operand *getOperand() const;

    void setOperand(std::unique_ptr<Operand> newOperand);
};

/**
 * Class for representing the call instruction.
 */
class CallInstruction : public Instruction {
  private:
    /**
     * The function identifier of the instruction.
     */
    std::string functionIdentifier;

  public:
    /**
     * Constructor for the call instruction class.
     *
     * @param functionIdentifier The function identifier of the instruction.
     */
    explicit CallInstruction(std::string functionIdentifier);

    [[nodiscard]] std::string getFunctionIdentifier() const;
};

/**
 * Class for representing the ret instruction.
 */
class RetInstruction : public Instruction {};

/**
 * Base class for representing a top-level construct.
 */
class TopLevel {
  public:
    /**
     * Default constructor for the top-level class.
     */
    constexpr TopLevel() = default;

    /**
     * Default virtual destructor for the top-level class.
     */
    virtual ~TopLevel() = default;

    /**
     * Delete the copy constructor for the top-level class.
     */
    constexpr TopLevel(const TopLevel &) = delete;

    /**
     * Delete the copy assignment operator for the top-level class.
     */
    constexpr TopLevel &operator=(const TopLevel &) = delete;

    /**
     * Default move constructor for the top-level class.
     */
    constexpr TopLevel(TopLevel &&) = default;

    /**
     * Default move assignment operator for the top-level class.
     */
    constexpr TopLevel &operator=(TopLevel &&) = default;
};

/**
 * Class for representing a function definition.
 */
class FunctionDefinition : public TopLevel {
  private:
    /**
     * The function identifier of the function definition.
     */
    std::string functionIdentifier;

    /**
     * Boolean indicating whether the function definition is global.
     */
    bool global = false;

    /**
     * The function body of the function definition.
     */
    std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> functionBody;

    /**
     * The stack size of the function definition.
     */
    size_t stackSize = 0;

  public:
    /**
     * Constructor for the function definition class.
     *
     * @param functionIdentifier The function identifier of the function
     * definition.
     * @param global Boolean indicating whether the function definition is
     * global.
     * @param functionBody The function body of the function definition.
     * @param stackSize The stack size of the function definition.
     */
    explicit FunctionDefinition(
        std::string functionIdentifier, bool global,
        std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> functionBody,
        size_t stackSize);

    [[nodiscard]] std::string getFunctionIdentifier() const;

    [[nodiscard]] bool isGlobal() const;

    [[nodiscard]] const std::vector<std::unique_ptr<Instruction>> &
    getFunctionBody() const;

    [[nodiscard]] std::vector<std::unique_ptr<Instruction>> &getFunctionBody();

    void
    setFunctionBody(std::unique_ptr<std::vector<std::unique_ptr<Instruction>>>
                        newFunctionBody);

    [[nodiscard]] size_t getStackSize() const;

    void setStackSize(size_t newStackSize);
};

/**
 * Class for representing a static variable.
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
    bool global = false;

    /**
     * The alignment of the static variable.
     */
    int alignment = 0;

    /**
     * The static initialization of the static variable.
     */
    std::unique_ptr<AST::StaticInit> staticInit;

  public:
    /**
     * Constructor for the static variable class.
     *
     * @param identifier The identifier of the static variable.
     * @param global Boolean indicating whether the static variable is global.
     * @param alignment The alignment of the static variable.
     * @param staticInit The static initialization of the static variable.
     */
    explicit StaticVariable(std::string identifier, bool global, int alignment,
                            std::unique_ptr<AST::StaticInit> staticInit);

    [[nodiscard]] std::string getIdentifier() const;

    [[nodiscard]] bool isGlobal() const;

    [[nodiscard]] int getAlignment() const;

    void setAlignment(int newAlignment);

    [[nodiscard]] const AST::StaticInit *getStaticInit() const;

    void setStaticInit(std::unique_ptr<AST::StaticInit> newStaticInit);
};

/**
 * Class for representing a program.
 */
class Program {
  private:
    /**
     * The top-levels of the program.
     */
    std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels;

  public:
    /**
     * Constructor for the program class.
     *
     * @param topLevels The top-levels of the program.
     */
    explicit Program(
        std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> topLevels);

    [[nodiscard]] const std::vector<std::unique_ptr<TopLevel>> &
    getTopLevels() const;

    [[nodiscard]] std::vector<std::unique_ptr<TopLevel>> &getTopLevels();

    void setTopLevels(
        std::unique_ptr<std::vector<std::unique_ptr<TopLevel>>> newTopLevels);
};
} // namespace Assembly

#endif // BACKEND_ASSEMBLY_H
