#ifndef PIPELINE_STAGES_EXECUTORS_H
#define PIPELINE_STAGES_EXECUTORS_H

#include "assembly.h"
#include "assemblyGenerator.cpp"
#include "compiler.h"
#include "fixupPass.h"
#include "ir.h"
#include "irGenerator.cpp"
#include "lexer.h"
#include "parser.h"
#include "printVisitor.h"
#include "pseudoToStackPass.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

class PipelineStagesExecutors {
  public:
    static std::vector<Token> lexerExecutor(const std::string &sourceFile);
    static std::shared_ptr<AST::Program>
    parserExecutor(const std::vector<Token> &tokens);
    static std::shared_ptr<IR::Program>
    irGeneratorExecutor(std::shared_ptr<AST::Program> astProgram);
    static std::shared_ptr<Assembly::Program>
    codegenExecutor(std::shared_ptr<IR::Program> irProgram);
    static void
    codeEmissionExecutor(std::shared_ptr<Assembly::Program> assemblyProgram,
                         const std::string &assemblyFile);
};

#endif // PIPELINE_STAGES_EXECUTORS_H
