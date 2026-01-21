#include "pseudoToStackPass.h"
#include "backendSymbolTable.h"
#include <cassert>

namespace Assembly {
void PseudoToStackPass::replacePseudoWithStackAndAssociateStackSize(
    std::vector<std::unique_ptr<TopLevel>> &topLevels,
    const BackendSymbolTable &backendSymbolTable) {
    // Replace pseudo registers with stack operands in each instruction and
    // associate the stack size with each function.
    for (auto &topLevel : topLevels) {
        if (auto *functionDefinition =
                dynamic_cast<Assembly::FunctionDefinition *>(topLevel.get())) {
            // Reset offset and clear map for each function.
            this->offset = 0;
            this->pseudoToStackMap.clear();

            // Replace pseudo registers with stack operands in each instruction.
            for (auto &instruction : functionDefinition->getFunctionBody()) {
                replacePseudoWithStack(instruction, backendSymbolTable);
            }

            // Set the stack size for the function.
            functionDefinition->setStackSize(
                static_cast<size_t>(-this->offset));
        }
    }

    // Check that all pseudo registers have been replaced.
    for (const auto &topLevel : topLevels) {
        if (auto *functionDefinition =
                dynamic_cast<Assembly::FunctionDefinition *>(topLevel.get())) {
            checkPseudoRegistersInFunctionDefinitionReplaced(
                *functionDefinition);
        }
    }
}

std::unique_ptr<Assembly::Operand> PseudoToStackPass::replaceOperand(
    const Assembly::Operand *operand,
    const BackendSymbolTable &backendSymbolTable) {
    if (!operand) {
        throw std::logic_error("Setting null operand in replaceOperand");
    }

    if (auto pseudoReg =
            dynamic_cast<const Assembly::PseudoRegisterOperand *>(operand)) {
        const std::string &pseudoRegister = pseudoReg->getPseudoRegister();
        if (this->pseudoToStackMap.find(pseudoRegister) ==
            this->pseudoToStackMap.end()) {
            // If a pseudoregister is not in `pseudoToStackMap`, look it up in
            // the backend symbol table.
            auto backendSymbolTableItForAlloc =
                backendSymbolTable.find(pseudoRegister);
            if (backendSymbolTableItForAlloc != backendSymbolTable.end()) {
                auto &backendEntry = backendSymbolTableItForAlloc->second;
                // If it has a static storage duration, map it to a data operand
                // by the same name.
                if (auto objEntry =
                        dynamic_cast<ObjEntry *>(backendEntry.get())) {
                    if (objEntry->isStaticStorage()) {
                        return std::make_unique<Assembly::DataOperand>(
                            pseudoRegister);
                    }
                }
            }
            // Otherwise, assign it a new slot on the stack based on its type.
            // Look up the type in the backend symbol table to determine the
            // allocation size.
            int allocationSize =
                8; // Default to 8 bytes for temporary variables
            auto backendSymbolTableIt = backendSymbolTable.find(pseudoRegister);
            if (backendSymbolTableIt != backendSymbolTable.end()) {
                auto &backendEntry = backendSymbolTableIt->second;
                if (auto objEntry =
                        dynamic_cast<ObjEntry *>(backendEntry.get())) {
                    auto assemblyType = objEntry->getAssemblyType();
                    if (dynamic_cast<const Quadword *>(assemblyType)) {
                        allocationSize = 8; // 8 bytes for `Quadword`.
                    }
                    else {
                        allocationSize = 4; // 4 bytes for `Longword`.
                    }
                }
            }

            // Align the offset to an 8-byte boundary if allocating 8 bytes (for
            // type `Quadword`).
            if (allocationSize == 8) {
                // Compute the (negative) remainder when offset is divided by 8.
                int rem = this->offset % 8;
                // If it's not aligned, round down the (negative) offset to the
                // next 8-byte boundary.
                if (rem != 0) {
                    // Essentially, Subtracting `8 + rem` is the same as
                    // subtracting `8 - |rem|` where `rem` is negative, which
                    // moves the negative offset down to the next lower multiple
                    // of 8.
                    this->offset -= (8 + rem);
                }
            }

            // Update the offset to the next available stack slot.
            this->offset -= allocationSize;
            // Update the `pseudoToStackMap` with the new offset.
            this->pseudoToStackMap[pseudoRegister] = this->offset;
        }

        // Replace the pseudo register with a stack operand.
        return std::make_unique<Assembly::StackOperand>(
            this->pseudoToStackMap[pseudoRegister],
            std::make_unique<Assembly::BP>());
    }

    if (auto imm = dynamic_cast<const Assembly::ImmediateOperand *>(operand)) {
        return std::make_unique<Assembly::ImmediateOperand>(
            imm->getImmediateLong());
    }

    if (auto regOp = dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        auto reg = regOp->getRegister();
        if (dynamic_cast<const Assembly::AX *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::AX>());
        }
        if (dynamic_cast<const Assembly::CX *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::CX>());
        }
        if (dynamic_cast<const Assembly::DX *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::DX>());
        }
        if (dynamic_cast<const Assembly::DI *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::DI>());
        }
        if (dynamic_cast<const Assembly::SI *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::SI>());
        }
        if (dynamic_cast<const Assembly::R8 *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R8>());
        }
        if (dynamic_cast<const Assembly::R9 *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R9>());
        }
        if (dynamic_cast<const Assembly::R10 *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>());
        }
        if (dynamic_cast<const Assembly::R11 *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>());
        }
        if (dynamic_cast<const Assembly::SP *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::SP>());
        }
        if (dynamic_cast<const Assembly::BP *>(reg)) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::BP>());
        }
        throw std::logic_error("Unsupported register while replacing operand");
    }

    if (auto stackOp = dynamic_cast<const Assembly::StackOperand *>(operand)) {
        auto reservedReg = stackOp->getReservedRegister();
        if (dynamic_cast<const Assembly::SP *>(reservedReg)) {
            return std::make_unique<Assembly::StackOperand>(
                stackOp->getOffset(), std::make_unique<Assembly::SP>());
        }
        if (dynamic_cast<const Assembly::BP *>(reservedReg)) {
            return std::make_unique<Assembly::StackOperand>(
                stackOp->getOffset(), std::make_unique<Assembly::BP>());
        }
        throw std::logic_error(
            "Unsupported reserved register while replacing operand");
    }

    if (auto dataOp = dynamic_cast<const Assembly::DataOperand *>(operand)) {
        return std::make_unique<Assembly::DataOperand>(dataOp->getIdentifier());
    }

    throw std::logic_error("Unsupported operand type in replaceOperand");
}

void PseudoToStackPass::replacePseudoWithStack(
    std::unique_ptr<Assembly::Instruction> &instruction,
    const BackendSymbolTable &backendSymbolTable) {
    if (auto movInstruction =
            dynamic_cast<Assembly::MovInstruction *>(instruction.get())) {
        auto newSrc =
            replaceOperand(movInstruction->getSrc(), backendSymbolTable);
        auto newDst =
            replaceOperand(movInstruction->getDst(), backendSymbolTable);
        movInstruction->setSrc(std::move(newSrc));
        movInstruction->setDst(std::move(newDst));
    }
    else if (auto movsxInstruction = dynamic_cast<Assembly::MovsxInstruction *>(
                 instruction.get())) {
        auto newSrc =
            replaceOperand(movsxInstruction->getSrc(), backendSymbolTable);
        auto newDst =
            replaceOperand(movsxInstruction->getDst(), backendSymbolTable);
        movsxInstruction->setSrc(std::move(newSrc));
        movsxInstruction->setDst(std::move(newDst));
    }
    else if (auto unaryInstruction = dynamic_cast<Assembly::UnaryInstruction *>(
                 instruction.get())) {
        auto newOperand =
            replaceOperand(unaryInstruction->getOperand(), backendSymbolTable);
        unaryInstruction->setOperand(std::move(newOperand));
    }
    else if (auto binaryInstruction =
                 dynamic_cast<Assembly::BinaryInstruction *>(
                     instruction.get())) {
        auto newOperand1 = replaceOperand(binaryInstruction->getOperand1(),
                                          backendSymbolTable);
        auto newOperand2 = replaceOperand(binaryInstruction->getOperand2(),
                                          backendSymbolTable);
        binaryInstruction->setOperand1(std::move(newOperand1));
        binaryInstruction->setOperand2(std::move(newOperand2));
    }
    else if (auto cmpInstruction =
                 dynamic_cast<Assembly::CmpInstruction *>(instruction.get())) {
        auto newOperand1 =
            replaceOperand(cmpInstruction->getOperand1(), backendSymbolTable);
        auto newOperand2 =
            replaceOperand(cmpInstruction->getOperand2(), backendSymbolTable);
        cmpInstruction->setOperand1(std::move(newOperand1));
        cmpInstruction->setOperand2(std::move(newOperand2));
    }
    else if (auto idivInstruction =
                 dynamic_cast<Assembly::IdivInstruction *>(instruction.get())) {
        auto newOperand =
            replaceOperand(idivInstruction->getOperand(), backendSymbolTable);
        idivInstruction->setOperand(std::move(newOperand));
    }
    else if (auto setCCInstruction = dynamic_cast<Assembly::SetCCInstruction *>(
                 instruction.get())) {
        auto newOperand =
            replaceOperand(setCCInstruction->getOperand(), backendSymbolTable);
        setCCInstruction->setOperand(std::move(newOperand));
    }
    else if (auto pushInstruction =
                 dynamic_cast<Assembly::PushInstruction *>(instruction.get())) {
        auto newOperand =
            replaceOperand(pushInstruction->getOperand(), backendSymbolTable);
        pushInstruction->setOperand(std::move(newOperand));
    }
    else if (auto retInstruction =
                 dynamic_cast<Assembly::RetInstruction *>(instruction.get())) {
        (void)retInstruction;
    }
    else if (auto callInstruction =
                 dynamic_cast<Assembly::CallInstruction *>(instruction.get())) {
        (void)callInstruction;
    }
    else if (auto cdqInstruction =
                 dynamic_cast<Assembly::CdqInstruction *>(instruction.get())) {
        (void)cdqInstruction;
    }
    else if (auto jmpInstruction =
                 dynamic_cast<Assembly::JmpInstruction *>(instruction.get())) {
        (void)jmpInstruction;
    }
    else if (auto jmpCCInstruction = dynamic_cast<Assembly::JmpCCInstruction *>(
                 instruction.get())) {
        (void)jmpCCInstruction;
    }
    else if (auto labelInstruction = dynamic_cast<Assembly::LabelInstruction *>(
                 instruction.get())) {
        (void)labelInstruction;
    }
    else {
        throw std::logic_error("Unsupported instruction type while replacing "
                               "pseudo registers with stack operands");
    }
}

void PseudoToStackPass::checkPseudoRegistersInFunctionDefinitionReplaced(
    const Assembly::FunctionDefinition &functionDefinition) {
    for (const auto &instruction : functionDefinition.getFunctionBody()) {
        if (auto movInstruction =
                dynamic_cast<MovInstruction *>(instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                movInstruction->getSrc()));
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                movInstruction->getDst()));
            (void)movInstruction;
        }
        else if (auto movsxInstruction =
                     dynamic_cast<Assembly::MovsxInstruction *>(
                         instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                movsxInstruction->getSrc()));
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                movsxInstruction->getDst()));
            (void)movsxInstruction;
        }
        else if (auto unaryInstruction =
                     dynamic_cast<Assembly::UnaryInstruction *>(
                         instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                unaryInstruction->getOperand()));
            (void)unaryInstruction;
        }
        else if (auto binaryInstruction =
                     dynamic_cast<Assembly::BinaryInstruction *>(
                         instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                binaryInstruction->getOperand1()));
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                binaryInstruction->getOperand2()));
            (void)binaryInstruction;
        }
        else if (auto cmpInstruction = dynamic_cast<Assembly::CmpInstruction *>(
                     instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                cmpInstruction->getOperand1()));
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                cmpInstruction->getOperand2()));
            (void)cmpInstruction;
        }
        else if (auto idivInstruction =
                     dynamic_cast<Assembly::IdivInstruction *>(
                         instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                idivInstruction->getOperand()));
            (void)idivInstruction;
        }
        else if (auto setCCInstruction =
                     dynamic_cast<Assembly::SetCCInstruction *>(
                         instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                setCCInstruction->getOperand()));
            (void)setCCInstruction;
        }
        else if (auto pushInstruction =
                     dynamic_cast<Assembly::PushInstruction *>(
                         instruction.get())) {
            assert(!dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                pushInstruction->getOperand()));
            (void)pushInstruction;
        }
        else if (auto retInstruction = dynamic_cast<Assembly::RetInstruction *>(
                     instruction.get())) {
            (void)retInstruction;
        }
        else if (auto callInstruction =
                     dynamic_cast<Assembly::CallInstruction *>(
                         instruction.get())) {
            (void)callInstruction;
        }
        else if (auto cdqInstruction = dynamic_cast<Assembly::CdqInstruction *>(
                     instruction.get())) {
            (void)cdqInstruction;
        }
        else if (auto jmpInstruction = dynamic_cast<Assembly::JmpInstruction *>(
                     instruction.get())) {
            (void)jmpInstruction;
        }
        else if (auto jmpCCInstruction =
                     dynamic_cast<Assembly::JmpCCInstruction *>(
                         instruction.get())) {
            (void)jmpCCInstruction;
        }
        else if (auto labelInstruction =
                     dynamic_cast<Assembly::LabelInstruction *>(
                         instruction.get())) {
            (void)labelInstruction;
        }
        else {
            throw std::logic_error("Unsupported instruction type while "
                                   "checking whether pseudo registers have "
                                   "been replaced in function definition");
        }
    }
}
} // namespace Assembly
