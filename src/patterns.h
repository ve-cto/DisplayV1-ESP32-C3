#pragma once

#include <Arduino.h>

constexpr uint8_t DIGITS[] = {DIG_0, DIG_1, DIG_2, DIG_3, DIG_4, DIG_5, DIG_6, DIG_7, DIG_8, DIG_9};

constexpr uint32_t SEG_A_MASK  = 1UL << SEG_A;
constexpr uint32_t SEG_B_MASK  = 1UL << SEG_B;
constexpr uint32_t SEG_C_MASK  = 1UL << SEG_C;
constexpr uint32_t SEG_D_MASK  = 1UL << SEG_D;
constexpr uint32_t SEG_E_MASK  = 1UL << SEG_E;
constexpr uint32_t SEG_F_MASK  = 1UL << SEG_F;
constexpr uint32_t SEG_G_MASK  = 1UL << SEG_G;
constexpr uint32_t SEG_H_MASK  = 1UL << SEG_H;
constexpr uint32_t SEG_DP_MASK = 1UL << SEG_DP;

/* We can only assume the segments' order until we actually test the board.
*   _
* |   |
*   _
* |   |
*   _
*    .
*     ,
*
*   A
* B   F
*   G
* C   E
*   D
*    DP
*     H
*/

// OR all of the mask bits in
constexpr uint32_t PATTERN_CLEAR  = 0;
constexpr uint32_t PATTERN_FULL   = SEG_A_MASK | SEG_B_MASK | SEG_C_MASK | SEG_D_MASK | SEG_E_MASK | SEG_F_MASK | SEG_G_MASK | SEG_H_MASK | SEG_DP_MASK;
constexpr uint32_t PATTERN_0      = SEG_A_MASK | SEG_B_MASK | SEG_C_MASK | SEG_D_MASK | SEG_E_MASK | SEG_F_MASK;
constexpr uint32_t PATTERN_1      = SEG_F_MASK | SEG_E_MASK;
constexpr uint32_t PATTERN_2      = SEG_A_MASK | SEG_F_MASK | SEG_G_MASK | SEG_C_MASK | SEG_D_MASK;
constexpr uint32_t PATTERN_3      = SEG_A_MASK | SEG_F_MASK | SEG_F_MASK | SEG_E_MASK | SEG_D_MASK;
constexpr uint32_t PATTERN_4      = SEG_B_MASK | SEG_G_MASK | SEG_E_MASK | SEG_F_MASK;
constexpr uint32_t PATTERN_5      = SEG_A_MASK | SEG_B_MASK | SEG_G_MASK | SEG_E_MASK | SEG_D_MASK;
constexpr uint32_t PATTERN_6      = SEG_B_MASK | SEG_C_MASK | SEG_D_MASK | SEG_E_MASK | SEG_G_MASK;
constexpr uint32_t PATTERN_7      = SEG_A_MASK | SEG_F_MASK | SEG_E_MASK;
constexpr uint32_t PATTERN_8      = SEG_A_MASK | SEG_B_MASK | SEG_C_MASK | SEG_D_MASK | SEG_E_MASK | SEG_F_MASK | SEG_G_MASK;
constexpr uint32_t PATTERN_9      = SEG_G_MASK | SEG_B_MASK | SEG_A_MASK | SEG_F_MASK | SEG_E_MASK;
constexpr uint32_t PATTERN_DP     = SEG_DP_MASK;
constexpr uint32_t PATTERN_COMMA  = SEG_H_MASK;

constexpr uint32_t PATTERN_NUMBERS[] = {PATTERN_0, PATTERN_1, PATTERN_2, PATTERN_3, PATTERN_4, PATTERN_5, PATTERN_6, PATTERN_7, PATTERN_8, PATTERN_9};
