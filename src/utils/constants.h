#ifndef UTILS_CONSTANTS_H
#define UTILS_CONSTANTS_H

#include <cstddef>

/**
 * Byte size for quadword types and registers.
 */
static constexpr int QUADWORD_SIZE = 8;

/**
 * Byte size for longword types and registers.
 */
static constexpr int LONGWORD_SIZE = 4;

/**
 * Number of argument registers available for function parameters.
 */
static constexpr std::size_t NUM_ARGUMENT_REGISTERS = 6;

/**
 * Stack alignment mask for 16-byte alignment (16 - 1).
 */
static constexpr std::size_t STACK_ALIGNMENT_MASK = 15;

#endif // UTILS_CONSTANTS_H
