#ifndef PRETTY_PRINTERS_H
#define PRETTY_PRINTERS_H

#include "assembly.h"
#include "ir.h"

#include <iostream>

class PrettyPrinters {
  public:
    static void printIRProgram(std::shared_ptr<IR::Program> irProgram);
    static void
    printAssemblyProgram(std::shared_ptr<Assembly::Program> assemblyProgram);
};

#endif // PRETTY_PRINTERS_H
