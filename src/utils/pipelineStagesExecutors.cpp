#include "pipelineStagesExecutors.h"
#include "../frontend/frontendSymbolTable.h"
#include <fstream>
#include <iostream>
#include <sstream>

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
        msg << "Lexical error: " << e.what();
        throw std::runtime_error(msg.str());
    }

    return tokens;
}

std::shared_ptr<AST::Program>
PipelineStagesExecutors::parserExecutor(const std::vector<Token> &tokens) {
    std::shared_ptr<AST::Program> program;
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
        msg << "Parsing error: " << e.what();
        throw std::runtime_error(msg.str());
    }

    return program;
}

int PipelineStagesExecutors::semanticAnalysisExecutor(
    const std::shared_ptr<AST::Program> &astProgram,
    AST::FrontendSymbolTable &frontendSymbolTable) {
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
        msg << "Identifier resolution error: " << e.what();
        throw std::runtime_error(msg.str());
    }
    try {
        // Perform the type-checking pass.
        typeCheckingPass.typeCheckProgram(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Type-checking error: " << e.what();
        throw std::runtime_error(msg.str());
    }
    try {
        // Perform the loop-labeling pass.
        loopLabelingPass.labelLoops(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Loop-labeling error: " << e.what();
        throw std::runtime_error(msg.str());
    }
    try {
        // Visit and print the AST program after semantic analysis.
        AST::PrintVisitor printVisitor;
        std::cout << "\n";
        astProgram->accept(printVisitor);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Printing AST error (in semantic analysis): " << e.what();
        throw std::runtime_error(msg.str());
    }

    return variableResolutionCounter;
}

std::pair<std::shared_ptr<IR::Program>,
          std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
PipelineStagesExecutors::irGeneratorExecutor(
    const std::shared_ptr<AST::Program> &astProgram,
    int variableResolutionCounter,
    AST::FrontendSymbolTable &frontendSymbolTable) {
    std::cout << "\n";
    std::pair<std::shared_ptr<IR::Program>,
              std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
        irProgramAndIRStaticVariables;
    try {
        IR::IRGenerator irGenerator(variableResolutionCounter,
                                    frontendSymbolTable);
        // Generate the IR program from the AST program.
        irProgramAndIRStaticVariables = irGenerator.generateIR(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "IR generation error: " << e.what();
        throw std::runtime_error(msg.str());
    }
    return irProgramAndIRStaticVariables;
}

void PipelineStagesExecutors::irOptimizationExecutor(
    const std::shared_ptr<IR::Program> &irProgram, bool foldConstantsPass,
    bool propagateCopiesPass, bool eliminateUnreachableCodePass,
    bool eliminateDeadStoresPass) {
    auto topLevels = irProgram->getTopLevels();
    for (auto topLevel : *topLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<IR::FunctionDefinition>(topLevel)) {
            // Extract the function body from the function definition, optimize
            // the function body, and set the optimized function body back to
            // the function definition.
            auto functionBody = functionDefinition->getFunctionBody();
            auto optimizedFunctionBody = IR::IROptimizer::irOptimize(
                functionBody, foldConstantsPass, propagateCopiesPass,
                eliminateUnreachableCodePass, eliminateDeadStoresPass);
            functionDefinition->setFunctionBody(optimizedFunctionBody);
        }
    }
}

std::shared_ptr<Assembly::Program> PipelineStagesExecutors::codegenExecutor(
    const std::shared_ptr<IR::Program> &irProgram,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
        &irStaticVariables,
    const AST::FrontendSymbolTable &frontendSymbolTable) {
    std::shared_ptr<Assembly::Program> assemblyProgram;
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
        auto topLevels = assemblyProgram->getTopLevels();
        pseudoToStackPass.replacePseudoWithStackAndAssociateStackSize(
            topLevels, backendSymbolTable);

        Assembly::FixupPass fixupPass;
        // Fix up the assembly program.
        fixupPass.fixup(topLevels);

        // Set the top-level elements of the assembly program after all the
        // passes.
        assemblyProgram->setTopLevels(topLevels);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Code generation error: " << e.what();
        throw std::runtime_error(msg.str());
    }

    // Finally, return the assembly program.
    return assemblyProgram;
}

void PipelineStagesExecutors::codeEmissionExecutor(
    const std::shared_ptr<Assembly::Program> &assemblyProgram,
    std::string_view assemblyFileName) {
    std::ofstream assemblyFileStream(std::string{assemblyFileName});
    if (!assemblyFileStream.is_open()) {
        std::stringstream msg;
        msg << "Error: Unable to open output file " << assemblyFileName;
        throw std::ios_base::failure(msg.str());
    }

    auto assyTopLevels = assemblyProgram->getTopLevels();
    for (auto topLevel : *assyTopLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<Assembly::FunctionDefinition>(
                    topLevel)) {
            emitAssyFunctionDefinition(functionDefinition, assemblyFileStream);
        }
        else if (auto staticVariable =
                     std::dynamic_pointer_cast<Assembly::StaticVariable>(
                         topLevel)) {
            emitAssyStaticVariable(staticVariable, assemblyFileStream);
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
    const std::shared_ptr<Assembly::FunctionDefinition> &functionDefinition,
    std::ofstream &assemblyFileStream) {
    auto functionName = functionDefinition->getFunctionIdentifier();
    prependUnderscoreToIdentifierIfMacOS(functionName);
    auto global = functionDefinition->isGlobal();
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

    for (auto instruction : *functionDefinition->getFunctionBody()) {
        emitAssyInstruction(instruction, assemblyFileStream);
    }
}

void PipelineStagesExecutors::emitAssyStaticVariable(
    const std::shared_ptr<Assembly::StaticVariable> &staticVariable,
    std::ofstream &assemblyFileStream) {
    auto alignment = staticVariable->getAlignment();
    auto alignmentInStr = std::to_string(alignment);
    auto alignDirective = ".align " + alignmentInStr;
// If the underlying OS is macOS, use the `.balign 4` directive instead of the
// `.align 4` directive.
#ifdef __APPLE__
    alignDirective = ".balign " + alignmentInStr;
#endif

    auto variableIdentifier = staticVariable->getIdentifier();
    prependUnderscoreToIdentifierIfMacOS(variableIdentifier);

    auto global = staticVariable->isGlobal();
    auto globalDirective = ".globl " + variableIdentifier + "\n";
    if (!global) {
        globalDirective = "";
    }

    bool isZeroInit = false;
    auto staticInit = staticVariable->getStaticInit();
    if (auto intInit = std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
        isZeroInit = (std::get<int>(intInit->getValue()) == 0);
    }
    else if (auto longInit =
                 std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
        isZeroInit = (std::get<long>(longInit->getValue()) == 0L);
    }
    else {
        throw std::logic_error(
            "Unknown static init type while printing assembly static variable");
    }

    assemblyFileStream << "\n";
    if (!isZeroInit) {
        assemblyFileStream << globalDirective;
        assemblyFileStream << "    .data\n";
        assemblyFileStream << "    " << alignDirective << "\n";
        assemblyFileStream << variableIdentifier << ":\n";
        if (auto intInit =
                std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
            if (std::get<int>(intInit->getValue()) == 0) {
                assemblyFileStream << "    .zero 4\n";
            }
            else {
                assemblyFileStream << "    .long "
                                   << std::get<int>(intInit->getValue())
                                   << "\n";
            }
        }
        else if (auto longInit =
                     std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
            if (std::get<long>(longInit->getValue()) == 0) {
                assemblyFileStream << "    .zero 8\n";
            }
            else {
                assemblyFileStream << "    .quad "
                                   << std::get<long>(longInit->getValue())
                                   << "\n";
            }
        }
    }
    else if (isZeroInit) {
        assemblyFileStream << globalDirective;
        assemblyFileStream << "    .bss\n";
        assemblyFileStream << "    " << alignDirective << "\n";
        assemblyFileStream << variableIdentifier << ":\n";
        if (auto intInit =
                std::dynamic_pointer_cast<AST::IntInit>(staticInit)) {
            assemblyFileStream << "    .zero 4\n";
        }
        else if (auto longInit =
                     std::dynamic_pointer_cast<AST::LongInit>(staticInit)) {
            assemblyFileStream << "    .zero 8\n";
        }
    }
}

void PipelineStagesExecutors::emitAssyInstruction(
    const std::shared_ptr<Assembly::Instruction> &instruction,
    std::ofstream &assemblyFileStream) {
    if (auto movInstruction =
            std::dynamic_pointer_cast<Assembly::MovInstruction>(instruction)) {
        emitAssyMovInstruction(movInstruction, assemblyFileStream);
    }
    else if (auto retInstruction =
                 std::dynamic_pointer_cast<Assembly::RetInstruction>(
                     instruction)) {
        emitAssyRetInstruction(assemblyFileStream);
    }
    else if (auto pushInstruction =
                 std::dynamic_pointer_cast<Assembly::PushInstruction>(
                     instruction)) {
        emitAssyPushInstruction(pushInstruction, assemblyFileStream);
    }
    else if (auto callInstruction =
                 std::dynamic_pointer_cast<Assembly::CallInstruction>(
                     instruction)) {
        emitAssyCallInstruction(callInstruction, assemblyFileStream);
    }
    else if (auto unaryInstruction =
                 std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                     instruction)) {
        emitAssyUnaryInstruction(unaryInstruction, assemblyFileStream);
    }
    else if (auto binaryInstruction =
                 std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                     instruction)) {
        emitAssyBinaryInstruction(binaryInstruction, assemblyFileStream);
    }
    else if (auto cmpInstruction =
                 std::dynamic_pointer_cast<Assembly::CmpInstruction>(
                     instruction)) {
        emitAssyCmpInstruction(cmpInstruction, assemblyFileStream);
    }
    else if (auto idivInstruction =
                 std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                     instruction)) {
        emitAssyIdivInstruction(idivInstruction, assemblyFileStream);
    }
    else if (auto movsxInstruction =
                 std::dynamic_pointer_cast<Assembly::MovsxInstruction>(
                     instruction)) {
        emitAssyMovsxInstruction(movsxInstruction, assemblyFileStream);
    }
    else if (auto cdqInstruction =
                 std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                     instruction)) {
        emitAssyCdqInstruction(cdqInstruction, assemblyFileStream);
    }
    else if (auto jmpInstruction =
                 std::dynamic_pointer_cast<Assembly::JmpInstruction>(
                     instruction)) {
        emitAssyJmpInstruction(jmpInstruction, assemblyFileStream);
    }
    else if (auto jmpCCInstruction =
                 std::dynamic_pointer_cast<Assembly::JmpCCInstruction>(
                     instruction)) {
        emitAssyJmpCCInstruction(jmpCCInstruction, assemblyFileStream);
    }
    else if (auto setCCInstruction =
                 std::dynamic_pointer_cast<Assembly::SetCCInstruction>(
                     instruction)) {
        emitAssySetCCInstruction(setCCInstruction, assemblyFileStream);
    }
    else if (auto labelInstruction =
                 std::dynamic_pointer_cast<Assembly::LabelInstruction>(
                     instruction)) {
        emitAssyLabelInstruction(labelInstruction, assemblyFileStream);
    }
    else {
        throw std::logic_error(
            "Invalid instruction type while printing assembly instruction");
    }
}

void PipelineStagesExecutors::emitAssyMovInstruction(
    const std::shared_ptr<Assembly::MovInstruction> &movInstruction,
    std::ofstream &assemblyFileStream) {
    auto type = movInstruction->getType();

    std::string instructionName;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        instructionName = "movl";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        instructionName = "movq";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly mov instruction");
    }

    auto src = movInstruction->getSrc();
    std::string srcStr;
    if (auto srcReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(registerSize);
    }
    else if (auto srcImm =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src)) {
        // Use long values for quadword instructions and int values for longword
        // instructions in order to avoid (potential) overflow issues.
        if (auto quadword =
                std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
            srcStr = "$" + std::to_string(srcImm->getImmediateLong());
        }
        else {
            srcStr = "$" + std::to_string(srcImm->getImmediate());
        }
    }
    else if (auto srcStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (auto srcData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Invalid source type while printing assembly mov instruction");
    }

    auto dst = movInstruction->getDst();
    std::string dstStr;
    if (auto dstReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(registerSize);
    }
    else if (auto dstStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (auto dstData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error(
            "Invalid destination type while printing assembly mov instruction");
    }

    assemblyFileStream << "    " << instructionName << " " << srcStr << ", "
                       << dstStr << "\n";
}

void PipelineStagesExecutors::emitAssyMovsxInstruction(
    const std::shared_ptr<Assembly::MovsxInstruction> &movsxInstruction,
    std::ofstream &assemblyFileStream) {
    auto src = movsxInstruction->getSrc();
    std::string srcStr;
    if (auto srcReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
        srcStr = srcReg->getRegisterInBytesInStr(4);
    }
    else if (auto srcImm =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src)) {
        srcStr = "$" + std::to_string(srcImm->getImmediateLong());
    }
    else if (auto srcStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(src)) {
        srcStr = std::to_string(srcStack->getOffset()) + "(" +
                 srcStack->getReservedRegisterInStr() + ")";
    }
    else if (auto srcData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        srcStr = identifier + "(%rip)";
    }
    else {
        std::stringstream msg;
        msg << "Invalid source type while printing assembly movsx instruction";
        if (src == nullptr) {
            msg << ": Source operand is nullptr";
        }
        else {
            msg << ": Source operand is not nullptr but unknown type";
        }
        throw std::logic_error(msg.str());
    }

    auto dst = movsxInstruction->getDst();
    std::string dstStr;
    if (auto dstReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
        dstStr = dstReg->getRegisterInBytesInStr(8);
    }
    else if (auto dstStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(dst)) {
        dstStr = std::to_string(dstStack->getOffset()) + "(" +
                 dstStack->getReservedRegisterInStr() + ")";
    }
    else if (auto dstData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        dstStr = identifier + "(%rip)";
    }
    else {
        throw std::logic_error("Invalid destination type while printing "
                               "assembly movsx instruction");
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
    const std::shared_ptr<Assembly::PushInstruction> &pushInstruction,
    std::ofstream &assemblyFileStream) {
    auto operand = pushInstruction->getOperand();

    if (auto stackOperand =
            std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        assemblyFileStream << "    pushq " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto regOperand =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand)) {
        assemblyFileStream << "    pushq "
                           << regOperand->getRegisterInBytesInStr(8) << "\n";
    }
    else if (auto immOperand =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                     operand)) {
        assemblyFileStream << "    pushq $" << immOperand->getImmediateLong()
                           << "\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << "    pushq " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly push instruction");
    }
}

void PipelineStagesExecutors::emitAssyCallInstruction(
    const std::shared_ptr<Assembly::CallInstruction> &callInstruction,
    std::ofstream &assemblyFileStream) {
    auto functionName = callInstruction->getFunctionIdentifier();
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
    const std::shared_ptr<Assembly::UnaryInstruction> &unaryInstruction,
    std::ofstream &assemblyFileStream) {
    auto unaryOperator = unaryInstruction->getUnaryOperator();
    auto type = unaryInstruction->getType();

    std::string instructionName;
    if (auto negateOperator =
            std::dynamic_pointer_cast<Assembly::NegateOperator>(
                unaryOperator)) {
        instructionName = "neg";
    }
    else if (auto complementOperator =
                 std::dynamic_pointer_cast<Assembly::ComplementOperator>(
                     unaryOperator)) {
        instructionName = "not";
    }
    else if (auto notOperator =
                 std::dynamic_pointer_cast<Assembly::NotOperator>(
                     unaryOperator)) {
        instructionName = "not";
    }
    else {
        throw std::logic_error(
            "Invalid unary operator while printing assembly unary instruction");
    }

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly unary instruction");
    }

    assemblyFileStream << "    " << instructionName << typeSuffix;

    auto operand = unaryInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        assemblyFileStream << " "
                           << regOperand->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        assemblyFileStream << " " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly unary instruction");
    }
}

void PipelineStagesExecutors::emitAssyBinaryInstruction(
    const std::shared_ptr<Assembly::BinaryInstruction> &binaryInstruction,
    std::ofstream &assemblyFileStream) {
    auto binaryOperator = binaryInstruction->getBinaryOperator();
    auto type = binaryInstruction->getType();

    std::string instructionName;
    if (auto addOperator =
            std::dynamic_pointer_cast<Assembly::AddOperator>(binaryOperator)) {
        instructionName = "add";
    }
    else if (auto subtractOperator =
                 std::dynamic_pointer_cast<Assembly::SubtractOperator>(
                     binaryOperator)) {
        instructionName = "sub";
    }
    else if (auto multiplyOperator =
                 std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                     binaryOperator)) {
        instructionName = "imul";
    }
    else {
        throw std::logic_error("Invalid binary operator while printing "
                               "assembly binary instruction");
    }

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly binary instruction");
    }

    assemblyFileStream << "    " << instructionName << typeSuffix;

    auto operand1 = binaryInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        if (auto quadword =
                std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
            assemblyFileStream << " $" << operand1Imm->getImmediateLong()
                               << ",";
        }
        else {
            assemblyFileStream << " $" << operand1Imm->getImmediate() << ",";
        }
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        assemblyFileStream << " "
                           << operand1Reg->getRegisterInBytesInStr(registerSize)
                           << ",";
    }
    else if (auto operand1Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand1)) {
        assemblyFileStream << " " << operand1Stack->getOffset() << "("
                           << operand1Stack->getReservedRegisterInStr() << "),";
    }
    else if (auto operand1Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip),";
    }

    auto operand2 = binaryInstruction->getOperand2();
    if (auto operand2Reg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand2)) {
        assemblyFileStream << " "
                           << operand2Reg->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (auto operand2Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand2)) {
        assemblyFileStream << " " << operand2Stack->getOffset() << "("
                           << operand2Stack->getReservedRegisterInStr()
                           << ")\n";
    }
    else if (auto operand2Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly binary instruction");
    }
}

void PipelineStagesExecutors::emitAssyCmpInstruction(
    const std::shared_ptr<Assembly::CmpInstruction> &cmpInstruction,
    std::ofstream &assemblyFileStream) {
    auto type = cmpInstruction->getType();

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly cmp instruction");
    }

    assemblyFileStream << "    cmp" << typeSuffix;

    auto operand1 = cmpInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        if (auto quadword =
                std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
            assemblyFileStream << " $" << operand1Imm->getImmediateLong();
        }
        else {
            assemblyFileStream << " $" << operand1Imm->getImmediate();
        }
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        assemblyFileStream << " "
                           << operand1Reg->getRegisterInBytesInStr(
                                  registerSize);
    }
    else if (auto operand1Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand1)) {
        assemblyFileStream << " " << operand1Stack->getOffset() << "("
                           << operand1Stack->getReservedRegisterInStr() << ")";
    }
    else if (auto operand1Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand1)) {
        auto identifier = operand1Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)";
    }

    assemblyFileStream << ",";

    auto operand2 = cmpInstruction->getOperand2();
    if (auto operand2Reg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand2)) {
        assemblyFileStream << " "
                           << operand2Reg->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (auto operand2Stack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand2)) {
        assemblyFileStream << " " << operand2Stack->getOffset() << "("
                           << operand2Stack->getReservedRegisterInStr()
                           << ")\n";
    }
    else if (auto operand2Data =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand2)) {
        auto identifier = operand2Data->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly cmp instruction");
    }
}

void PipelineStagesExecutors::emitAssyIdivInstruction(
    const std::shared_ptr<Assembly::IdivInstruction> &idivInstruction,
    std::ofstream &assemblyFileStream) {
    auto type = idivInstruction->getType();

    std::string typeSuffix;
    int registerSize;
    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        typeSuffix = "l";
        registerSize = 4;
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        typeSuffix = "q";
        registerSize = 8;
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly idiv instruction");
    }

    assemblyFileStream << "    idiv" << typeSuffix;

    auto operand = idivInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        assemblyFileStream << " "
                           << regOperand->getRegisterInBytesInStr(registerSize)
                           << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        assemblyFileStream << " " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly idiv instruction");
    }
}

void PipelineStagesExecutors::emitAssyCdqInstruction(
    const std::shared_ptr<Assembly::CdqInstruction> &cdqInstruction,
    std::ofstream &assemblyFileStream) {
    auto type = cdqInstruction->getType();

    if (auto longword = std::dynamic_pointer_cast<Assembly::Longword>(type)) {
        assemblyFileStream << "    cdq\n";
    }
    else if (auto quadword =
                 std::dynamic_pointer_cast<Assembly::Quadword>(type)) {
        assemblyFileStream << "    cqo\n";
    }
    else {
        throw std::logic_error(
            "Invalid type while printing assembly cdq instruction");
    }
}

void PipelineStagesExecutors::emitAssyJmpInstruction(
    const std::shared_ptr<Assembly::JmpInstruction> &jmpInstruction,
    std::ofstream &assemblyFileStream) {
    auto label = jmpInstruction->getLabel();
    assemblyFileStream << "    jmp .L" << label << "\n";
}

void PipelineStagesExecutors::emitAssyJmpCCInstruction(
    const std::shared_ptr<Assembly::JmpCCInstruction> &jmpCCInstruction,
    std::ofstream &assemblyFileStream) {
    auto condCode = jmpCCInstruction->getCondCode();
    if (auto e = std::dynamic_pointer_cast<Assembly::E>(condCode)) {
        assemblyFileStream << "    je";
    }
    else if (auto ne = std::dynamic_pointer_cast<Assembly::NE>(condCode)) {
        assemblyFileStream << "    jne";
    }
    else if (auto g = std::dynamic_pointer_cast<Assembly::G>(condCode)) {
        assemblyFileStream << "    jg";
    }
    else if (auto ge = std::dynamic_pointer_cast<Assembly::GE>(condCode)) {
        assemblyFileStream << "    jge";
    }
    else if (auto l = std::dynamic_pointer_cast<Assembly::L>(condCode)) {
        assemblyFileStream << "    jl";
    }
    else if (auto le = std::dynamic_pointer_cast<Assembly::LE>(condCode)) {
        assemblyFileStream << "    jle";
    }
    else {
        throw std::logic_error("Invalid conditional code while printing "
                               "assembly jmpcc instruction");
    }

    auto label = jmpCCInstruction->getLabel();
    assemblyFileStream << " .L" << label << "\n";
}

void PipelineStagesExecutors::emitAssySetCCInstruction(
    const std::shared_ptr<Assembly::SetCCInstruction> &setCCInstruction,
    std::ofstream &assemblyFileStream) {
    auto condCode = setCCInstruction->getCondCode();
    if (auto e = std::dynamic_pointer_cast<Assembly::E>(condCode)) {
        assemblyFileStream << "    sete";
    }
    else if (auto ne = std::dynamic_pointer_cast<Assembly::NE>(condCode)) {
        assemblyFileStream << "    setne";
    }
    else if (auto g = std::dynamic_pointer_cast<Assembly::G>(condCode)) {
        assemblyFileStream << "    setg";
    }
    else if (auto ge = std::dynamic_pointer_cast<Assembly::GE>(condCode)) {
        assemblyFileStream << "    setge";
    }
    else if (auto l = std::dynamic_pointer_cast<Assembly::L>(condCode)) {
        assemblyFileStream << "    setl";
    }
    else if (auto le = std::dynamic_pointer_cast<Assembly::LE>(condCode)) {
        assemblyFileStream << "    setle";
    }
    else {
        throw std::logic_error("Invalid conditional code while printing "
                               "assembly setcc instruction");
    }

    auto operand = setCCInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        assemblyFileStream << " " << regOperand->getRegisterInBytesInStr(1)
                           << "\n";
    }
    else if (auto stackOperand =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(operand)) {
        assemblyFileStream << " " << stackOperand->getOffset() << "("
                           << stackOperand->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << " " << identifier << "(%rip)\n";
    }
    else {
        throw std::logic_error(
            "Invalid operand type while printing assembly setcc instruction");
    }
}

void PipelineStagesExecutors::emitAssyLabelInstruction(
    const std::shared_ptr<Assembly::LabelInstruction> &labelInstruction,
    std::ofstream &assemblyFileStream) {
    auto label = labelInstruction->getLabel();
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
