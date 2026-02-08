#include "pipelineStagesExecutors.h"
#include "../backend/assembly.h"
#include "../backend/assemblyGenerator.h"
#include "../backend/backendSymbolTable.h"
#include "../backend/fixupPass.h"
#include "../backend/pseudoToStackPass.h"
#include "../frontend/frontendSymbolTable.h"
#include "../frontend/lexer.h"
#include "../frontend/parser.h"
#include "../frontend/printVisitor.h"
#include "../frontend/program.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../midend/ir.h"
#include "../midend/irGenerator.h"
#include "../midend/irOptimizationPasses.h"
#include "../utils/constants.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

std::vector<Token>
PipelineStagesExecutors::lexerExecutor(std::string_view sourceFileName) {
    std::ifstream sourceFileInputStream(std::string{sourceFileName});
    if (sourceFileInputStream.fail()) {
        std::stringstream msg;
        msg << "Unable to open source file: " << sourceFileName;
        throw std::ios_base::failure(msg.str());
    }

    // Read the entire source file into a string.
    // The first iterator is an iterator that reads characters from the input
    // stream.
    // The second iterator is the (default-constructed) end-of-stream iterator.
    // The string range-constructor reads characters from the input stream until
    // the end-of-stream iterator is reached.
    auto input =
        std::string((std::istreambuf_iterator<char>(sourceFileInputStream)),
                    std::istreambuf_iterator<char>());
    sourceFileInputStream.close();

    std::vector<Token> tokens;
    try {
        tokens = lexer(input);
        printTokens(tokens);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Lexical error in lexerExecutor in PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }

    return tokens;
}

std::unique_ptr<AST::Program>
PipelineStagesExecutors::parserExecutor(const std::vector<Token> &tokens) {
    std::unique_ptr<AST::Program> program;
    try {
        AST::Parser parser(tokens);
        // Parse the tokens to generate the AST program.
        program = parser.parse();
        AST::PrintVisitor printVisitor;
        std::cout << "\n";
        // Visit and print the AST program after parsing.
        program->accept(printVisitor);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Parsing error in parserExecutor in PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }

    return program;
}

int PipelineStagesExecutors::semanticAnalysisExecutor(
    AST::Program &astProgram, AST::FrontendSymbolTable &frontendSymbolTable) {
    AST::IdentifierResolutionPass IdentifierResolutionPass;
    AST::TypeCheckingPass typeCheckingPass(frontendSymbolTable);
    AST::LoopLabelingPass loopLabelingPass;
    auto variableResolutionCounter = 0;

    try {
        // Perform the identifier-resolution pass.
        variableResolutionCounter =
            IdentifierResolutionPass.resolveProgram(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Identifier resolution error in semanticAnalysisExecutor in "
               "PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }
    try {
        // Perform the type-checking pass.
        typeCheckingPass.typeCheckProgram(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Type-checking error in semanticAnalysisExecutor in "
               "PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }
    try {
        // Perform the loop-labeling pass.
        loopLabelingPass.labelLoops(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Loop-labeling error in semanticAnalysisExecutor in "
               "PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }
    try {
        // Visit and print the AST program after semantic analysis.
        AST::PrintVisitor printVisitor;
        std::cout << "\n";
        astProgram.accept(printVisitor);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Printing AST error (in semantic analysis) in "
               "semanticAnalysisExecutor in PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }

    return variableResolutionCounter;
}

std::pair<std::unique_ptr<IR::Program>,
          std::unique_ptr<std::vector<std::unique_ptr<IR::StaticVariable>>>>
PipelineStagesExecutors::irGeneratorExecutor(
    const AST::Program &astProgram, int variableResolutionCounter,
    AST::FrontendSymbolTable &frontendSymbolTable) {
    (void)variableResolutionCounter;
    (void)frontendSymbolTable;
    std::cout << "\n";
    std::pair<std::unique_ptr<IR::Program>,
              std::unique_ptr<std::vector<std::unique_ptr<IR::StaticVariable>>>>
        irProgramAndIRStaticVariables;
    try {
        IR::IRGenerator irGenerator(variableResolutionCounter,
                                    frontendSymbolTable);
        // Generate the IR program from the AST program.
        irProgramAndIRStaticVariables = irGenerator.generateIR(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "IR generation error in irGeneratorExecutor in "
               "PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }
    return irProgramAndIRStaticVariables;
}

void PipelineStagesExecutors::irOptimizationExecutor(
    IR::Program &irProgram, bool foldConstantsPass, bool propagateCopiesPass,
    bool eliminateUnreachableCodePass, bool eliminateDeadStoresPass) {
    for (auto &topLevel : irProgram.getTopLevels()) {
        if (auto *functionDefinition =
                dynamic_cast<IR::FunctionDefinition *>(topLevel.get())) {
            // Extract the function body from the function definition, optimize
            // the function body, and set the optimized function body back to
            // the function definition.
            const auto &functionBody = functionDefinition->getFunctionBody();
            auto optimizedFunctionBody = IR::IROptimizer::irOptimize(
                functionBody, foldConstantsPass, propagateCopiesPass,
                eliminateUnreachableCodePass, eliminateDeadStoresPass);
            functionDefinition->setFunctionBody(
                std::move(optimizedFunctionBody));
        }
    }
}

std::unique_ptr<Assembly::Program> PipelineStagesExecutors::codegenExecutor(
    const IR::Program &irProgram,
    const std::vector<std::unique_ptr<IR::StaticVariable>> &irStaticVariables,
    const AST::FrontendSymbolTable &frontendSymbolTable) {
    (void)irStaticVariables;
    std::unique_ptr<Assembly::Program> assemblyProgram;
    try {
        // Convert the frontend symbol table to backend symbol table before
        // assembly generation so all variables (including temporaries) are
        // available.
        Assembly::BackendSymbolTable backendSymbolTable;
        Assembly::convertFrontendToBackendSymbolTable(frontendSymbolTable,
                                                      backendSymbolTable);

        Assembly::AssemblyGenerator assemblyGenerator(irStaticVariables,
                                                      frontendSymbolTable);
        // Generate the assembly program from the IR program.
        assemblyProgram = assemblyGenerator.generateAssembly(irProgram);

        Assembly::PseudoToStackPass pseudoToStackPass;
        // Associate the stack size with each top-level element.
        auto &topLevels = assemblyProgram->getTopLevels();
        pseudoToStackPass.replacePseudoWithStackAndAssociateStackSize(
            topLevels, backendSymbolTable);

        // Fix up the assembly program.
        Assembly::FixupPass::fixup(topLevels);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Code generation error in codegenExecutor in "
               "PipelineStagesExecutors: "
            << e.what();
        throw std::runtime_error(msg.str());
    }

    // Finally, return the assembly program.
    return assemblyProgram;
}

void PipelineStagesExecutors::codeEmissionExecutor(
    const Assembly::Program &assemblyProgram,
    std::string_view assemblyFileName) {
    std::ofstream assemblyFileStream(std::string{assemblyFileName});
    if (!assemblyFileStream.is_open()) {
        std::stringstream msg;
        msg << "Error: Unable to open output file " << assemblyFileName
            << " in codeEmissionExecutor in PipelineStagesExecutors";
        throw std::ios_base::failure(msg.str());
    }

    const auto &assyTopLevels = assemblyProgram.getTopLevels();
    for (const auto &topLevel : assyTopLevels) {
        if (auto *functionDefinition =
                dynamic_cast<Assembly::FunctionDefinition *>(topLevel.get())) {
            emitAssyFunctionDefinition(*functionDefinition, assemblyFileStream);
        }
        else if (auto *staticVariable =
                     dynamic_cast<Assembly::StaticVariable *>(topLevel.get())) {
            emitAssyStaticVariable(*staticVariable, assemblyFileStream);
        }
    }

// If the underlying OS is Linux, add the following to enable an important
// security hardening measure: it indicates that the code does not require an
// executable stack.
#ifdef __linux__
    assemblyFileStream << ".section .note.GNU-stack,\"\",@progbits\n";
#endif

    assemblyFileStream.close();
}

void PipelineStagesExecutors::emitAssyFunctionDefinition(
    const Assembly::FunctionDefinition &functionDefinition,
    std::ofstream &assemblyFileStream) {
    auto functionName = functionDefinition.getFunctionIdentifier();
    prependUnderscoreToIdentifierIfMacOS(functionName);
    auto global = functionDefinition.isGlobal();
    auto globalDirective = "    .globl " + functionName + "\n";
    if (!global) {
        globalDirective = "";
    }

    // Emit the function prologue (before emitting the function body).
    assemblyFileStream << globalDirective;
    assemblyFileStream << "    .text\n";
    assemblyFileStream << functionName << ":\n";
    assemblyFileStream << "    pushq %rbp\n";
    assemblyFileStream << "    movq %rsp, %rbp\n";

    for (const auto &instruction : functionDefinition.getFunctionBody()) {
        emitAssyInstruction(*instruction, assemblyFileStream);
    }
}

void PipelineStagesExecutors::emitAssyStaticVariable(
    const Assembly::StaticVariable &staticVariable,
    std::ofstream &assemblyFileStream) {
    auto alignment = staticVariable.getAlignment();
    auto alignmentInStr = std::to_string(alignment);
    auto alignDirective = ".align " + alignmentInStr;
// If the underlying OS is macOS, use the `.balign 4` directive instead of the
// `.align 4` directive.
#ifdef __APPLE__
    alignDirective = ".balign " + alignmentInStr;
#endif

    auto variableIdentifier = staticVariable.getIdentifier();
    prependUnderscoreToIdentifierIfMacOS(variableIdentifier);

    auto global = staticVariable.isGlobal();
    auto globalDirective = ".globl " + variableIdentifier + "\n";
    if (!global) {
        globalDirective = "";
    }

    bool isZeroInit = false;
    const auto *staticInit = staticVariable.getStaticInit();
    if (const auto *intInit = dynamic_cast<const AST::IntInit *>(staticInit)) {
        isZeroInit = (std::get<int>(intInit->getValue()) == 0);
    }
    else if (const auto *longInit =
                 dynamic_cast<const AST::LongInit *>(staticInit)) {
        isZeroInit = (std::get<long>(longInit->getValue()) == 0L);
    }
    else if (const auto *uintInit =
                 dynamic_cast<const AST::UIntInit *>(staticInit)) {
        isZeroInit = (std::get<unsigned int>(uintInit->getValue()) == 0U);
    }
    else if (const auto *ulongInit =
                 dynamic_cast<const AST::ULongInit *>(staticInit)) {
        isZeroInit = (std::get<unsigned long>(ulongInit->getValue()) == 0UL);
    }
    else {
        const auto &r = *staticInit;
        throw std::logic_error(
            "Unsupported static init type while printing "
            "assembly static variable in emitAssyStaticVariable "
            "in PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }

    assemblyFileStream << "\n";
    if (!isZeroInit) {
        assemblyFileStream << globalDirective;
        assemblyFileStream << "    .data\n";
        assemblyFileStream << "    " << alignDirective << "\n";
        assemblyFileStream << variableIdentifier << ":\n";
        if (const auto *intInit =
                dynamic_cast<const AST::IntInit *>(staticInit)) {
            assemblyFileStream << "    .long "
                               << std::get<int>(intInit->getValue()) << "\n";
        }
        else if (const auto *longInit =
                     dynamic_cast<const AST::LongInit *>(staticInit)) {
            assemblyFileStream << "    .quad "
                               << std::get<long>(longInit->getValue()) << "\n";
        }
        else if (const auto *uintInit =
                     dynamic_cast<const AST::UIntInit *>(staticInit)) {
            assemblyFileStream << "    .long "
                               << std::get<unsigned int>(uintInit->getValue())
                               << "\n";
        }
        else if (const auto *ulongInit =
                     dynamic_cast<const AST::ULongInit *>(staticInit)) {
            assemblyFileStream << "    .quad "
                               << std::get<unsigned long>(ulongInit->getValue())
                               << "\n";
        }
    }
    else if (isZeroInit) {
        assemblyFileStream << globalDirective;
        assemblyFileStream << "    .bss\n";
        assemblyFileStream << "    " << alignDirective << "\n";
        assemblyFileStream << variableIdentifier << ":\n";
        if (dynamic_cast<const AST::IntInit *>(staticInit) ||
            dynamic_cast<const AST::UIntInit *>(staticInit)) {
            assemblyFileStream << "    .zero 4\n";
        }
        else if (dynamic_cast<const AST::LongInit *>(staticInit) ||
                 dynamic_cast<const AST::ULongInit *>(staticInit)) {
            assemblyFileStream << "    .zero 8\n";
        }
    }
}

void PipelineStagesExecutors::emitAssyInstruction(
    const Assembly::Instruction &instruction,
    std::ofstream &assemblyFileStream) {
    if (const auto *movInstruction =
            dynamic_cast<const Assembly::MovInstruction *>(&instruction)) {
        emitAssyMovInstruction(*movInstruction, assemblyFileStream);
    }
    else if (dynamic_cast<const Assembly::RetInstruction *>(&instruction)) {
        emitAssyRetInstruction(assemblyFileStream);
    }
    else if (const auto *pushInstruction =
                 dynamic_cast<const Assembly::PushInstruction *>(
                     &instruction)) {
        emitAssyPushInstruction(*pushInstruction, assemblyFileStream);
    }
    else if (const auto *callInstruction =
                 dynamic_cast<const Assembly::CallInstruction *>(
                     &instruction)) {
        emitAssyCallInstruction(*callInstruction, assemblyFileStream);
    }
    else if (const auto *unaryInstruction =
                 dynamic_cast<const Assembly::UnaryInstruction *>(
                     &instruction)) {
        emitAssyUnaryInstruction(*unaryInstruction, assemblyFileStream);
    }
    else if (const auto *binaryInstruction =
                 dynamic_cast<const Assembly::BinaryInstruction *>(
                     &instruction)) {
        emitAssyBinaryInstruction(*binaryInstruction, assemblyFileStream);
    }
    else if (const auto *cmpInstruction =
                 dynamic_cast<const Assembly::CmpInstruction *>(&instruction)) {
        emitAssyCmpInstruction(*cmpInstruction, assemblyFileStream);
    }
    else if (const auto *idivInstruction =
                 dynamic_cast<const Assembly::IdivInstruction *>(
                     &instruction)) {
        emitAssyIdivInstruction(*idivInstruction, assemblyFileStream);
    }
    else if (const auto *divInstruction =
                 dynamic_cast<const Assembly::DivInstruction *>(&instruction)) {
        emitAssyDivInstruction(*divInstruction, assemblyFileStream);
    }
    else if (const auto *movsxInstruction =
                 dynamic_cast<const Assembly::MovsxInstruction *>(
                     &instruction)) {
        emitAssyMovsxInstruction(*movsxInstruction, assemblyFileStream);
    }
    else if (const auto *cdqInstruction =
                 dynamic_cast<const Assembly::CdqInstruction *>(&instruction)) {
        emitAssyCdqInstruction(*cdqInstruction, assemblyFileStream);
    }
    else if (const auto *jmpInstruction =
                 dynamic_cast<const Assembly::JmpInstruction *>(&instruction)) {
        emitAssyJmpInstruction(*jmpInstruction, assemblyFileStream);
    }
    else if (const auto *jmpCCInstruction =
                 dynamic_cast<const Assembly::JmpCCInstruction *>(
                     &instruction)) {
        emitAssyJmpCCInstruction(*jmpCCInstruction, assemblyFileStream);
    }
    else if (const auto *setCCInstruction =
                 dynamic_cast<const Assembly::SetCCInstruction *>(
                     &instruction)) {
        emitAssySetCCInstruction(*setCCInstruction, assemblyFileStream);
    }
    else if (const auto *labelInstruction =
                 dynamic_cast<const Assembly::LabelInstruction *>(
                     &instruction)) {
        emitAssyLabelInstruction(*labelInstruction, assemblyFileStream);
    }
    else {
        const auto &r = *&instruction;
        throw std::logic_error(
            "Unsupported instruction type while printing assembly instruction "
            "in emitAssyInstruction in PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }
}

void PipelineStagesExecutors::emitAssyMovInstruction(
    const Assembly::MovInstruction &movInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *type = movInstruction.getType();

    std::string instructionName;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type)) {
        instructionName = "movl";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type)) {
        instructionName = "movq";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly mov instruction in "
            "emitAssyMovInstruction in PipelineStagesExecutors");
    }

    const auto *src = movInstruction.getSrc();
    std::string srcStr;
    if (const auto *srcReg =
            dynamic_cast<const Assembly::RegisterOperand *>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(registerSize);
    }
    else if (const auto *srcImm =
                 dynamic_cast<const Assembly::ImmediateOperand *>(src)) {
        srcStr =
            "$" + std::to_string(static_cast<long>(srcImm->getImmediate()));
    }
    else if (const auto *srcStack =
                 dynamic_cast<const Assembly::StackOperand *>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *srcData =
                 dynamic_cast<const Assembly::DataOperand *>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        const auto &r = *src;
        throw std::logic_error(
            "Unsupported source type while printing assembly mov instruction "
            "in emitAssyMovInstruction in PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }

    const auto *dst = movInstruction.getDst();
    std::string dstStr;
    if (const auto *dstReg =
            dynamic_cast<const Assembly::RegisterOperand *>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(registerSize);
    }
    else if (const auto *dstStack =
                 dynamic_cast<const Assembly::StackOperand *>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *dstData =
                 dynamic_cast<const Assembly::DataOperand *>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        const auto &r = *dst;
        throw std::logic_error(
            "Unsupported destination type while printing assembly mov "
            "instruction in emitAssyMovInstruction in "
            "PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }

    assemblyFileStream << "    " << instructionName << " " << srcStr << ", "
                       << dstStr << "\n";
}

void PipelineStagesExecutors::emitAssyMovsxInstruction(
    const Assembly::MovsxInstruction &movsxInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *src = movsxInstruction.getSrc();
    std::string srcStr;
    if (const auto *srcReg =
            dynamic_cast<const Assembly::RegisterOperand *>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(4);
    }
    else if (const auto *srcImm =
                 dynamic_cast<const Assembly::ImmediateOperand *>(src)) {
        srcStr =
            "$" + std::to_string(static_cast<long>(srcImm->getImmediate()));
    }
    else if (const auto *srcStack =
                 dynamic_cast<const Assembly::StackOperand *>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *srcData =
                 dynamic_cast<const Assembly::DataOperand *>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Unsupported source type while printing assembly movsx instruction "
            "in emitAssyMovsxInstruction in PipelineStagesExecutors");
    }

    const auto *dst = movsxInstruction.getDst();
    std::string dstStr;
    if (const auto *dstReg =
            dynamic_cast<const Assembly::RegisterOperand *>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(QUADWORD_SIZE);
    }
    else if (const auto *dstStack =
                 dynamic_cast<const Assembly::StackOperand *>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (const auto *dstData =
                 dynamic_cast<const Assembly::DataOperand *>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Unsupported destination type while printing assembly movsx "
            "instruction in emitAssyMovsxInstruction in "
            "PipelineStagesExecutors");
    }

    assemblyFileStream << "    movslq " << srcStr << ", " << dstStr << "\n";
}

void PipelineStagesExecutors::emitAssyRetInstruction(
    std::ofstream &assemblyFileStream) {
    // Emit the function epilogue before emitting the return
    // instruction.
    assemblyFileStream << "    movq %rbp, %rsp\n";
    assemblyFileStream << "    popq %rbp\n";
    assemblyFileStream << "    ret\n";
}

void PipelineStagesExecutors::emitAssyPushInstruction(
    const Assembly::PushInstruction &pushInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *operand = pushInstruction.getOperand();

    if (const auto *stackOperand =
            dynamic_cast<const Assembly::StackOperand *>(operand)) {
        assemblyFileStream << "    pushq " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *regOperand =
                 dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        assemblyFileStream << "    pushq "
                           << regOperand->getRegisterInBytesInStr(QUADWORD_SIZE)
                           << "\n";
    }
    else if (const auto *immOperand =
                 dynamic_cast<const Assembly::ImmediateOperand *>(operand)) {
        assemblyFileStream << "    pushq $"
                           << static_cast<long>(immOperand->getImmediate())
                           << "\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << "    pushq " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Unsupported operand type while printing assembly push instruction "
            "in emitAssyPushInstruction in PipelineStagesExecutors");
    }
}

void PipelineStagesExecutors::emitAssyCallInstruction(
    const Assembly::CallInstruction &callInstruction,
    std::ofstream &assemblyFileStream) {
    auto functionName = callInstruction.getFunctionIdentifier();
    prependUnderscoreToIdentifierIfMacOS(functionName);
    assemblyFileStream << "    call " << functionName;
// If the underlying OS is Linux, add the `@PLT` suffix (PLT modifier) to the
// operand.
#ifdef __linux__
    assemblyFileStream << "@PLT";
#endif
    assemblyFileStream << "\n";
}

void PipelineStagesExecutors::emitAssyUnaryInstruction(
    const Assembly::UnaryInstruction &unaryInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *unaryOperator = unaryInstruction.getUnaryOperator();
    const auto *type = unaryInstruction.getType();

    std::string instructionName;
    if (dynamic_cast<const Assembly::NegateOperator *>(unaryOperator)) {
        instructionName = "neg";
    }
    else if ((dynamic_cast<const Assembly::ComplementOperator *>(
                 unaryOperator)) ||
             (dynamic_cast<const Assembly::NotOperator *>(unaryOperator))) {
        instructionName = "not";
    }
    else {
        throw std::logic_error(
            "Unsupported unary operator while printing assembly unary "
            "instruction in emitAssyUnaryInstruction in "
            "PipelineStagesExecutors");
    }

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type)) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type)) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly unary instruction in "
            "emitAssyUnaryInstruction in PipelineStagesExecutors");
    }

    assemblyFileStream << "    " << instructionName << typeSuffix;

    const auto *operand = unaryInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        assemblyFileStream << " "
                           << regOperand->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        assemblyFileStream << " " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly unary "
            "instruction in emitAssyUnaryInstruction in "
            "PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }
}

void PipelineStagesExecutors::emitAssyBinaryInstruction(
    const Assembly::BinaryInstruction &binaryInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *binaryOperator = binaryInstruction.getBinaryOperator();
    const auto *type = binaryInstruction.getType();

    std::string instructionName;
    if (dynamic_cast<const Assembly::AddOperator *>(binaryOperator)) {
        instructionName = "add";
    }
    else if (dynamic_cast<const Assembly::SubtractOperator *>(binaryOperator)) {
        instructionName = "sub";
    }
    else if (dynamic_cast<const Assembly::MultiplyOperator *>(binaryOperator)) {
        instructionName = "imul";
    }
    else {
        throw std::logic_error("Unsupported binary operator while printing "
                               "assembly binary instruction");
    }

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type)) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type)) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly binary instruction in "
            "emitAssyBinaryInstruction in PipelineStagesExecutors");
    }

    assemblyFileStream << "    " << instructionName << typeSuffix;

    const auto *operand1 = binaryInstruction.getOperand1();
    if (const auto *operand1Imm =
            dynamic_cast<const Assembly::ImmediateOperand *>(operand1)) {
        assemblyFileStream << " $"
                           << static_cast<long>(operand1Imm->getImmediate())
                           << ",";
    }
    else if (const auto *operand1Reg =
                 dynamic_cast<const Assembly::RegisterOperand *>(operand1)) {
        assemblyFileStream << " "
                           << operand1Reg->getRegisterInBytesInStr(registerSize)
                           << ",";
    }
    else if (const auto *operand1Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand1)) {
        assemblyFileStream << " " << operand1Stack->getOffset() << "("
                           << operand1Stack->getReservedRegisterInStr() << "),";
    }
    else if (const auto *operand1Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip),";
    }

    const auto *operand2 = binaryInstruction.getOperand2();
    if (const auto *operand2Reg =
            dynamic_cast<const Assembly::RegisterOperand *>(operand2)) {
        assemblyFileStream << " "
                           << operand2Reg->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (const auto *operand2Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand2)) {
        assemblyFileStream << " " << operand2Stack->getOffset() << "("
                           << operand2Stack->getReservedRegisterInStr()
                           << ")\n";
    }
    else if (const auto *operand2Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand2;
        throw std::logic_error(
            "Unsupported operand type while printing assembly binary "
            "instruction in emitAssyBinaryInstruction in "
            "PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }
}

void PipelineStagesExecutors::emitAssyCmpInstruction(
    const Assembly::CmpInstruction &cmpInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *type = cmpInstruction.getType();

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type)) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type)) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly cmp instruction in "
            "emitAssyCmpInstruction in PipelineStagesExecutors");
    }

    assemblyFileStream << "    cmp" << typeSuffix;

    const auto *operand1 = cmpInstruction.getOperand1();
    if (const auto *operand1Imm =
            dynamic_cast<const Assembly::ImmediateOperand *>(operand1)) {
        assemblyFileStream << " $"
                           << static_cast<long>(operand1Imm->getImmediate());
    }
    else if (const auto *operand1Reg =
                 dynamic_cast<const Assembly::RegisterOperand *>(operand1)) {
        assemblyFileStream << " "
                           << operand1Reg->getRegisterInBytesInStr(
                                  registerSize);
    }
    else if (const auto *operand1Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand1)) {
        assemblyFileStream << " " << operand1Stack->getOffset() << "("
                           << operand1Stack->getReservedRegisterInStr() << ")";
    }
    else if (const auto *operand1Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)";
    }

    assemblyFileStream << ",";

    const auto *operand2 = cmpInstruction.getOperand2();
    if (const auto *operand2Reg =
            dynamic_cast<const Assembly::RegisterOperand *>(operand2)) {
        assemblyFileStream << " "
                           << operand2Reg->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (const auto *operand2Stack =
                 dynamic_cast<const Assembly::StackOperand *>(operand2)) {
        assemblyFileStream << " " << operand2Stack->getOffset() << "("
                           << operand2Stack->getReservedRegisterInStr()
                           << ")\n";
    }
    else if (const auto *operand2Data =
                 dynamic_cast<const Assembly::DataOperand *>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand2;
        throw std::logic_error(
            "Unsupported operand type while printing assembly cmp instruction "
            "in emitAssyCmpInstruction in PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }
}

void PipelineStagesExecutors::emitAssyIdivInstruction(
    const Assembly::IdivInstruction &idivInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *type = idivInstruction.getType();

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type)) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type)) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly idiv instruction in "
            "emitAssyIdivInstruction in PipelineStagesExecutors");
    }

    assemblyFileStream << "    idiv" << typeSuffix;

    const auto *operand = idivInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        assemblyFileStream << " "
                           << regOperand->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        assemblyFileStream << " " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly idiv instruction "
            "in emitAssyIdivInstruction in PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }
}

void PipelineStagesExecutors::emitAssyDivInstruction(
    const Assembly::DivInstruction &divInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *type = divInstruction.getType();

    std::string typeSuffix;
    int registerSize = 0;
    if (dynamic_cast<const Assembly::Longword *>(type)) {
        typeSuffix = "l";
        registerSize = LONGWORD_SIZE;
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type)) {
        typeSuffix = "q";
        registerSize = QUADWORD_SIZE;
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly div instruction in "
            "emitAssyDivInstruction in PipelineStagesExecutors");
    }

    assemblyFileStream << "    div" << typeSuffix;

    const auto *operand = divInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        assemblyFileStream << " "
                           << regOperand->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        assemblyFileStream << " " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly div instruction "
            "in emitAssyDivInstruction in PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }
}

void PipelineStagesExecutors::emitAssyCdqInstruction(
    const Assembly::CdqInstruction &cdqInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *type = cdqInstruction.getType();

    if (dynamic_cast<const Assembly::Longword *>(type)) {
        assemblyFileStream << "    cdq\n";
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type)) {
        assemblyFileStream << "    cqo\n";
    }
    else {
        throw std::logic_error(
            "Unsupported type while printing assembly cdq instruction in "
            "emitAssyCdqInstruction in PipelineStagesExecutors");
    }
}

void PipelineStagesExecutors::emitAssyJmpInstruction(
    const Assembly::JmpInstruction &jmpInstruction,
    std::ofstream &assemblyFileStream) {
    auto label = jmpInstruction.getLabel();
    assemblyFileStream << "    jmp .L" << label << "\n";
}

void PipelineStagesExecutors::emitAssyJmpCCInstruction(
    const Assembly::JmpCCInstruction &jmpCCInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *condCode = jmpCCInstruction.getCondCode();
    if (dynamic_cast<const Assembly::E *>(condCode)) {
        assemblyFileStream << "    je";
    }
    else if (dynamic_cast<const Assembly::NE *>(condCode)) {
        assemblyFileStream << "    jne";
    }
    else if (dynamic_cast<const Assembly::G *>(condCode)) {
        assemblyFileStream << "    jg";
    }
    else if (dynamic_cast<const Assembly::GE *>(condCode)) {
        assemblyFileStream << "    jge";
    }
    else if (dynamic_cast<const Assembly::L *>(condCode)) {
        assemblyFileStream << "    jl";
    }
    else if (dynamic_cast<const Assembly::LE *>(condCode)) {
        assemblyFileStream << "    jle";
    }
    else if (dynamic_cast<const Assembly::A *>(condCode)) {
        assemblyFileStream << "    ja";
    }
    else if (dynamic_cast<const Assembly::AE *>(condCode)) {
        assemblyFileStream << "    jae";
    }
    else if (dynamic_cast<const Assembly::B *>(condCode)) {
        assemblyFileStream << "    jb";
    }
    else if (dynamic_cast<const Assembly::BE *>(condCode)) {
        assemblyFileStream << "    jbe";
    }
    else {
        throw std::logic_error(
            "Unsupported conditional code while printing assembly jmpcc "
            "instruction in emitAssyJmpCCInstruction in "
            "PipelineStagesExecutors");
    }

    auto label = jmpCCInstruction.getLabel();
    assemblyFileStream << " .L" << label << "\n";
}

void PipelineStagesExecutors::emitAssySetCCInstruction(
    const Assembly::SetCCInstruction &setCCInstruction,
    std::ofstream &assemblyFileStream) {
    const auto *condCode = setCCInstruction.getCondCode();
    if (dynamic_cast<const Assembly::E *>(condCode)) {
        assemblyFileStream << "    sete";
    }
    else if (dynamic_cast<const Assembly::NE *>(condCode)) {
        assemblyFileStream << "    setne";
    }
    else if (dynamic_cast<const Assembly::G *>(condCode)) {
        assemblyFileStream << "    setg";
    }
    else if (dynamic_cast<const Assembly::GE *>(condCode)) {
        assemblyFileStream << "    setge";
    }
    else if (dynamic_cast<const Assembly::L *>(condCode)) {
        assemblyFileStream << "    setl";
    }
    else if (dynamic_cast<const Assembly::LE *>(condCode)) {
        assemblyFileStream << "    setle";
    }
    else if (dynamic_cast<const Assembly::A *>(condCode)) {
        assemblyFileStream << "    seta";
    }
    else if (dynamic_cast<const Assembly::AE *>(condCode)) {
        assemblyFileStream << "    setae";
    }
    else if (dynamic_cast<const Assembly::B *>(condCode)) {
        assemblyFileStream << "    setb";
    }
    else if (dynamic_cast<const Assembly::BE *>(condCode)) {
        assemblyFileStream << "    setbe";
    }
    else {
        throw std::logic_error(
            "Unsupported conditional code while printing assembly setcc "
            "instruction in emitAssySetCCInstruction in "
            "PipelineStagesExecutors");
    }

    const auto *operand = setCCInstruction.getOperand();
    if (const auto *regOperand =
            dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        assemblyFileStream << " " << regOperand->getRegisterInBytesInStr(1)
                           << "\n";
    }
    else if (const auto *stackOperand =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        assemblyFileStream << " " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (const auto *dataOperand =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        const auto &r = *operand;
        throw std::logic_error(
            "Unsupported operand type while printing assembly setcc "
            "instruction in emitAssySetCCInstruction in "
            "PipelineStagesExecutors: " +
            std::string(typeid(r).name()));
    }
}

void PipelineStagesExecutors::emitAssyLabelInstruction(
    const Assembly::LabelInstruction &labelInstruction,
    std::ofstream &assemblyFileStream) {
    auto label = labelInstruction.getLabel();
    assemblyFileStream << ".L" << label << ":\n";
}

void PipelineStagesExecutors::prependUnderscoreToIdentifierIfMacOS(
    [[maybe_unused]] std::string &identifier) {
// If the underlying OS is macOS, prepend an underscore to the function name.
// Otherwise, leave the function name as is.
#ifdef __APPLE__
    identifier = "_" + identifier;
#endif
}
