#include "pipelineStagesExecutors.h"

std::vector<Token>
PipelineStagesExecutors::lexerExecutor(std::string_view sourceFile) {
    // Instantiate an input file stream to read the source file.
    std::ifstream sourceFileInputStream(std::string{sourceFile});
    if (sourceFileInputStream.fail()) {
        std::stringstream msg;
        msg << "Unable to open source file: " << sourceFile;
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
        tokens = lexer(std::move(input));
        printTokens(tokens);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Lexical error: " << e.what();
        throw std::runtime_error(msg.str());
    }

    return tokens;
}

// Function to parse the tokens, generate the AST, visit the AST, and print the
// AST.
std::shared_ptr<AST::Program>
PipelineStagesExecutors::parserExecutor(const std::vector<Token> &tokens) {
    std::shared_ptr<AST::Program> program;
    try {
        AST::Parser parser(tokens);
        program = parser.parse();
        AST::PrintVisitor printVisitor;
        std::cout << "\n";
        program->accept(printVisitor);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Parsing error: " << e.what();
        throw std::runtime_error(msg.str());
    }

    return program;
}

// Function to perform semantic-analysis passes on the AST program.
std::pair<
    int, std::unordered_map<
             std::string, std::pair<std::shared_ptr<AST::Type>,
                                    std::shared_ptr<AST::IdentifierAttribute>>>>
PipelineStagesExecutors::semanticAnalysisExecutor(
    const std::shared_ptr<AST::Program> &astProgram) {
    AST::IdentifierResolutionPass IdentifierResolutionPass;
    AST::TypeCheckingPass typeCheckingPass;
    AST::LoopLabelingPass loopLabelingPass;
    auto variableResolutionCounter = 0;
    std::unordered_map<std::string,
                       std::pair<std::shared_ptr<AST::Type>,
                                 std::shared_ptr<AST::IdentifierAttribute>>>
        symbols;

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
        symbols = typeCheckingPass.typeCheckProgram(astProgram);
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
        // Print the AST after semantic analysis.
        AST::PrintVisitor printVisitor;
        std::cout << "\n";
        astProgram->accept(printVisitor);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "Printing AST error (in semantic analysis): " << e.what();
        throw std::runtime_error(msg.str());
    }

    // Return the variable resolution counter and the symbol table
    // altogether.
    return {variableResolutionCounter, symbols};
}

// Function to generate the IR from the AST program.
std::pair<std::shared_ptr<IR::Program>,
          std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
PipelineStagesExecutors::irGeneratorExecutor(
    const std::shared_ptr<AST::Program> &astProgram,
    int variableResolutionCounter,
    const std::unordered_map<
        std::string, std::pair<std::shared_ptr<AST::Type>,
                               std::shared_ptr<AST::IdentifierAttribute>>>
        &symbols) {
    std::cout << "\n";

    std::pair<std::shared_ptr<IR::Program>,
              std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
        irProgramAndIRStaticVariables;
    try {
        IR::IRGenerator irGenerator(variableResolutionCounter, symbols);
        irProgramAndIRStaticVariables = irGenerator.generateIR(astProgram);
    } catch (const std::runtime_error &e) {
        std::stringstream msg;
        msg << "IR generation error: " << e.what();
        throw std::runtime_error(msg.str());
    }
    return irProgramAndIRStaticVariables;
}

// Function to perform optimization passes on the IR program.
void PipelineStagesExecutors::irOptimizationExecutor(
    const std::shared_ptr<IR::Program> &irProgram, bool foldConstantsPass,
    bool propagateCopiesPass, bool eliminateUnreachableCodePass,
    bool eliminateDeadStoresPass) {
    auto topLevels = irProgram->getTopLevels();
    for (auto topLevel : *topLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<IR::FunctionDefinition>(topLevel)) {
            auto functionBody = functionDefinition->getFunctionBody();
            auto optimizedFunctionBody = IR::IROptimizer::irOptimize(
                functionBody, foldConstantsPass, propagateCopiesPass,
                eliminateUnreachableCodePass, eliminateDeadStoresPass);
            functionDefinition->setFunctionBody(optimizedFunctionBody);
        }
    }
}

// Function to generate (but not yet emit) the assembly program from the AST
// program.
std::shared_ptr<Assembly::Program> PipelineStagesExecutors::codegenExecutor(
    const std::shared_ptr<IR::Program> &irProgram,
    const std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
        &irStaticVariables,
    const std::unordered_map<
        std::string, std::pair<std::shared_ptr<AST::Type>,
                               std::shared_ptr<AST::IdentifierAttribute>>>
        &symbols) {
    std::shared_ptr<Assembly::Program> assemblyProgram;
    try {
        // Instantiate an assembly generator object and generate the assembly.
        Assembly::AssemblyGenerator assemblyGenerator(irStaticVariables,
                                                      symbols);
        assemblyProgram = assemblyGenerator.generateAssembly(irProgram);

        // Instantiate a pseudo-to-stack pass object and associate the stack
        // size with each top-level element.
        Assembly::PseudoToStackPass pseudoToStackPass(symbols);
        auto topLevels = assemblyProgram->getTopLevels();
        pseudoToStackPass.replacePseudoWithStackAndAssociateStackSize(
            topLevels);

        // Instantiate a fixup pass object and fixup the assembly program.
        Assembly::FixupPass fixupPass;
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

// Function to emit the generated assembly code to the assembly file.
void PipelineStagesExecutors::codeEmissionExecutor(
    const std::shared_ptr<Assembly::Program> &assemblyProgram,
    std::string_view assemblyFile) {
    std::ofstream assemblyFileStream(std::string{assemblyFile});
    if (!assemblyFileStream.is_open()) {
        std::stringstream msg;
        msg << "Error: Unable to open output file " << assemblyFile;
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
    auto alignDirective = ".align 4";
// If the underlying OS is macOS, use the `.balign 4` directive instead of the
// `.align 4` directive.
#ifdef __APPLE__
    alignDirective = ".balign 4";
#endif
    auto variableIdentifier = staticVariable->getIdentifier();
    prependUnderscoreToIdentifierIfMacOS(variableIdentifier);
    auto global = staticVariable->isGlobal();
    auto globalDirective = ".globl " + variableIdentifier + "\n";
    if (!global) {
        globalDirective = "";
    }
    auto initialValue = staticVariable->getInitialValue();

    assemblyFileStream << "\n";
    if (initialValue != 0) {
        assemblyFileStream << globalDirective;
        assemblyFileStream << "    .data\n";
        assemblyFileStream << "    " << alignDirective << "\n";
        assemblyFileStream << variableIdentifier << ":\n";
        assemblyFileStream << "    .long " << initialValue << "\n";
    }
    else if (initialValue == 0) {
        assemblyFileStream << globalDirective;
        assemblyFileStream << "    .bss\n";
        assemblyFileStream << "    " << alignDirective << "\n";
        assemblyFileStream << variableIdentifier << ":\n";
        assemblyFileStream << "    .zero 4\n";
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
    else if (auto allocateStackInstruction =
                 std::dynamic_pointer_cast<Assembly::AllocateStackInstruction>(
                     instruction)) {
        emitAssyAllocateStackInstruction(allocateStackInstruction,
                                         assemblyFileStream);
    }
    else if (auto deallocateStackInstruction = std::dynamic_pointer_cast<
                 Assembly::DeallocateStackInstruction>(instruction)) {
        emitAssyDeallocateStackInstruction(deallocateStackInstruction,
                                           assemblyFileStream);
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
    else if (auto cdqInstruction =
                 std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                     instruction)) {
        emitAssyCdqInstruction(assemblyFileStream);
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
}

void PipelineStagesExecutors::emitAssyMovInstruction(
    const std::shared_ptr<Assembly::MovInstruction> &movInstruction,
    std::ofstream &assemblyFileStream) {
    auto src = movInstruction->getSrc();
    if (auto srcReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
        assemblyFileStream << "    movl " << srcReg->getRegisterInBytesInStr(4);
    }
    else if (auto srcImm =
                 std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src)) {
        assemblyFileStream << "    movl $" << srcImm->getImmediate();
    }
    else if (auto srcStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(src)) {
        assemblyFileStream << "    movl " << srcStack->getOffset() << "("
                           << srcStack->getReservedRegisterInStr() << ")";
    }
    else if (auto srcData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(src)) {
        auto identifier = srcData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << "    movl " << identifier << "(%rip)";
    }

    auto dst = movInstruction->getDst();
    if (auto dstReg =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
        assemblyFileStream << ", " << dstReg->getRegisterInBytesInStr(4)
                           << "\n";
    }
    else if (auto dstStack =
                 std::dynamic_pointer_cast<Assembly::StackOperand>(dst)) {
        assemblyFileStream << ", " << dstStack->getOffset() << "("
                           << dstStack->getReservedRegisterInStr() << ")\n";
    }
    else if (auto dstData =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(dst)) {
        auto identifier = dstData->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << ", " << identifier << "(%rip)\n";
    }
}

void PipelineStagesExecutors::emitAssyRetInstruction(
    std::ofstream &assemblyFileStream) {
    // Emit the function epilogue before emitting the return
    // instruction.
    assemblyFileStream << "    movq %rbp, %rsp\n";
    assemblyFileStream << "    popq %rbp\n";
    assemblyFileStream << "    ret\n";
}

void PipelineStagesExecutors::emitAssyAllocateStackInstruction(
    const std::shared_ptr<Assembly::AllocateStackInstruction>
        &allocateStackInstruction,
    std::ofstream &assemblyFileStream) {
    assemblyFileStream
        << "    subq $"
        << allocateStackInstruction->getAddressGivenOffsetFromRBP()
        << ", %rsp\n";
}

void PipelineStagesExecutors::emitAssyDeallocateStackInstruction(
    const std::shared_ptr<Assembly::DeallocateStackInstruction>
        &deallocateStackInstruction,
    std::ofstream &assemblyFileStream) {
    assemblyFileStream
        << "    addq $"
        << deallocateStackInstruction->getAddressGivenOffsetFromRBP()
        << ", %rsp\n";
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
        assemblyFileStream << "    pushq $" << immOperand->getImmediate()
                           << "\n";
    }
    else if (auto dataOperand =
                 std::dynamic_pointer_cast<Assembly::DataOperand>(operand)) {
        auto identifier = dataOperand->getIdentifier();
        prependUnderscoreToIdentifierIfMacOS(identifier);
        assemblyFileStream << "    pushq " << identifier << "(%rip)\n";
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
    if (auto negateOperator =
            std::dynamic_pointer_cast<Assembly::NegateOperator>(
                unaryOperator)) {
        assemblyFileStream << "    negl";
    }
    else if (auto complementOperator =
                 std::dynamic_pointer_cast<Assembly::ComplementOperator>(
                     unaryOperator)) {
        assemblyFileStream << "    notl";
    }
    else if (auto notOperator =
                 std::dynamic_pointer_cast<Assembly::NotOperator>(
                     unaryOperator)) {
        assemblyFileStream << "    notl";
    }

    auto operand = unaryInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        assemblyFileStream << " " << regOperand->getRegisterInBytesInStr(4)
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
}

void PipelineStagesExecutors::emitAssyBinaryInstruction(
    const std::shared_ptr<Assembly::BinaryInstruction> &binaryInstruction,
    std::ofstream &assemblyFileStream) {
    auto binaryOperator = binaryInstruction->getBinaryOperator();
    if (auto addOperator =
            std::dynamic_pointer_cast<Assembly::AddOperator>(binaryOperator)) {
        assemblyFileStream << "    addl";
    }
    else if (auto subtractOperator =
                 std::dynamic_pointer_cast<Assembly::SubtractOperator>(
                     binaryOperator)) {
        assemblyFileStream << "    subl";
    }
    else if (auto multiplyOperator =
                 std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                     binaryOperator)) {
        assemblyFileStream << "    imull";
    }

    auto operand1 = binaryInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        assemblyFileStream << " $" << operand1Imm->getImmediate() << ",";
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        assemblyFileStream << " " << operand1Reg->getRegisterInBytesInStr(4)
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
        assemblyFileStream << " " << operand2Reg->getRegisterInBytesInStr(4)
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
}

void PipelineStagesExecutors::emitAssyCmpInstruction(
    const std::shared_ptr<Assembly::CmpInstruction> &cmpInstruction,
    std::ofstream &assemblyFileStream) {
    assemblyFileStream << "    cmpl";

    auto operand1 = cmpInstruction->getOperand1();
    if (auto operand1Imm =
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(operand1)) {
        assemblyFileStream << " $" << operand1Imm->getImmediate();
    }
    else if (auto operand1Reg =
                 std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                     operand1)) {
        assemblyFileStream << " " << operand1Reg->getRegisterInBytesInStr(4);
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
        assemblyFileStream << " " << operand2Reg->getRegisterInBytesInStr(4)
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
}

void PipelineStagesExecutors::emitAssyIdivInstruction(
    const std::shared_ptr<Assembly::IdivInstruction> &idivInstruction,
    std::ofstream &assemblyFileStream) {
    assemblyFileStream << "    idivl";

    auto operand = idivInstruction->getOperand();
    if (auto regOperand =
            std::dynamic_pointer_cast<Assembly::RegisterOperand>(operand)) {
        assemblyFileStream << " " << regOperand->getRegisterInBytesInStr(4)
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
}

void PipelineStagesExecutors::emitAssyCdqInstruction(
    std::ofstream &assemblyFileStream) {
    assemblyFileStream << "    cdq\n";
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
    identifier = "_" + std::move(identifier);
#endif
}
