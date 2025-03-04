/* RISC-V opcode list
   Copyright 2011-2015 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (waterman@cs.berkeley.edu) at UC Berkeley.
   Based on MIPS target.

   PULP family support contributed by Eric Flamand (eflamand@iis.ee.ethz.ch) at ETH-Zurich

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "opcode/riscv.h"
#include <stdio.h>

/* Register names used by gas and objdump.  */

const char * const riscv_gpr_names_numeric[32] =
{
  "x0",   "x1",   "x2",   "x3",   "x4",   "x5",   "x6",   "x7",
  "x8",   "x9",   "x10",  "x11",  "x12",  "x13",  "x14",  "x15",
  "x16",  "x17",  "x18",  "x19",  "x20",  "x21",  "x22",  "x23",
  "x24",  "x25",  "x26",  "x27",  "x28",  "x29",  "x30",  "x31"
};

const char * const riscv_gpr_names_abi[32] = {
  "zero", "ra", "sp",  "gp",  "tp", "t0",  "t1",  "t2",
  "s0",   "s1", "a0",  "a1",  "a2", "a3",  "a4",  "a5",
  "a6",   "a7", "s2",  "s3",  "s4", "s5",  "s6",  "s7",
  "s8",   "s9", "s10", "s11", "t3", "t4",  "t5",  "t6"
};

const char * const riscv_fpr_names_numeric[32] =
{
  "f0",   "f1",   "f2",   "f3",   "f4",   "f5",   "f6",   "f7",
  "f8",   "f9",   "f10",  "f11",  "f12",  "f13",  "f14",  "f15",
  "f16",  "f17",  "f18",  "f19",  "f20",  "f21",  "f22",  "f23",
  "f24",  "f25",  "f26",  "f27",  "f28",  "f29",  "f30",  "f31"
};

const char * const riscv_fpr_names_abi[32] = {
  "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
  "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
  "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
  "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
};

/* The order of overloaded instructions matters.  Label arguments and
   register arguments look the same. Instructions that can have either
   for arguments must apear in the correct order in this table for the
   assembler to pick the right one. In other words, entries with
   immediate operands must apear after the same instruction with
   registers.

   Because of the lookup algorithm used, entries with the same opcode
   name must be contiguous.  */

#define WR_xd INSN_WRITE_GPR_D
#define WR_fd INSN_WRITE_FPR_D
#define RD_xs1 INSN_READ_GPR_S
#define RD_xs2 INSN_READ_GPR_T
#define RD_xs3 INSN_READ_GPR_R
#define RD_fs1 INSN_READ_FPR_S
#define RD_fs2 INSN_READ_FPR_T
#define RD_fs3 INSN_READ_FPR_R

#define MASK_RS1 (OP_MASK_RS1 << OP_SH_RS1)
#define MASK_RS2 (OP_MASK_RS2 << OP_SH_RS2)
#define MASK_RD (OP_MASK_RD << OP_SH_RD)
#define MASK_CRS2 (OP_MASK_CRS2 << OP_SH_CRS2)
#define MASK_IMM ENCODE_ITYPE_IMM(-1U)
#define MASK_RVC_IMM ENCODE_RVC_IMM(-1U)
#define MASK_UIMM ENCODE_UTYPE_IMM(-1U)
#define MASK_RM (OP_MASK_RM << OP_SH_RM)
#define MASK_PRED (OP_MASK_PRED << OP_SH_PRED)
#define MASK_SUCC (OP_MASK_SUCC << OP_SH_SUCC)
#define MASK_AQ (OP_MASK_AQ << OP_SH_AQ)
#define MASK_RL (OP_MASK_RL << OP_SH_RL)
#define MASK_AQRL (MASK_AQ | MASK_RL)

static int match_opcode(const struct riscv_opcode *op, insn_t insn)
{
  return ((insn ^ op->match) & op->mask) == 0;
}

static int match_never(const struct riscv_opcode *op ATTRIBUTE_UNUSED,
		       insn_t insn ATTRIBUTE_UNUSED)
{
  return 0;
}

static int match_rs1_eq_rs2(const struct riscv_opcode *op, insn_t insn)
{
  int rs1 = (insn & MASK_RS1) >> OP_SH_RS1;
  int rs2 = (insn & MASK_RS2) >> OP_SH_RS2;
  return match_opcode (op, insn) && rs1 == rs2;
}

static int match_rd_nonzero(const struct riscv_opcode *op, insn_t insn)
{
  return match_opcode (op, insn) && ((insn & MASK_RD) != 0);
}

static int match_c_add(const struct riscv_opcode *op, insn_t insn)
{
  return match_rd_nonzero (op, insn) && ((insn & MASK_CRS2) != 0);
}

static int match_c_lui(const struct riscv_opcode *op, insn_t insn)
{
  return match_rd_nonzero (op, insn) && (((insn & MASK_RD) >> OP_SH_RD) != 2);
}

const struct riscv_opcode riscv_builtin_opcodes[] =
{
/* name,      isa,   operands, match, mask, match_func, pinfo */
{"unimp",     "C",   "",  0, 0xffffU,  match_opcode, 0 },
{"unimp",     "I",   "",  MATCH_CSRRW | (CSR_CYCLE << OP_SH_CSR), 0xffffffffU,  match_opcode, 0 }, /* csrw cycle, x0 */
{"ebreak",    "C",   "",  MATCH_C_EBREAK, MASK_C_EBREAK, match_opcode, INSN_ALIAS },
{"ebreak",    "I",   "",    MATCH_EBREAK, MASK_EBREAK, match_opcode,   0 },
{"sbreak",    "C",   "",  MATCH_C_EBREAK, MASK_C_EBREAK, match_opcode, INSN_ALIAS },
{"sbreak",    "I",   "",    MATCH_EBREAK, MASK_EBREAK, match_opcode,   INSN_ALIAS },
{"ret",       "C",   "",  MATCH_C_JR | (X_RA << OP_SH_RD), MASK_C_JR | MASK_RD, match_opcode, INSN_ALIAS },
{"ret",       "I",   "",  MATCH_JALR | (X_RA << OP_SH_RS1), MASK_JALR | MASK_RD | MASK_RS1 | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"jr",        "C",   "d",  MATCH_C_JR, MASK_C_JR, match_rd_nonzero, INSN_ALIAS },
{"jr",        "I",   "s",  MATCH_JALR, MASK_JALR | MASK_RD | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"jr",        "I",   "s,j",  MATCH_JALR, MASK_JALR | MASK_RD, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"jalr",      "C",   "d",  MATCH_C_JALR, MASK_C_JALR, match_rd_nonzero, INSN_ALIAS },
{"jalr",      "I",   "s",  MATCH_JALR | (X_RA << OP_SH_RD), MASK_JALR | MASK_RD | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"jalr",      "I",   "s,j",  MATCH_JALR | (X_RA << OP_SH_RD), MASK_JALR | MASK_RD, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"jalr",      "I",   "d,s",  MATCH_JALR, MASK_JALR | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"jalr",      "I",   "d,s,j",  MATCH_JALR, MASK_JALR, match_opcode,   WR_xd|RD_xs1 },
{"j",         "C",   "Ca",  MATCH_C_J, MASK_C_J, match_opcode, INSN_ALIAS },
{"j",         "I",   "a",  MATCH_JAL, MASK_JAL | MASK_RD, match_opcode,   INSN_ALIAS },
{"jal",       "32C", "Ca",  MATCH_C_JAL, MASK_C_JAL, match_opcode, INSN_ALIAS },
{"jal",       "I",   "a",  MATCH_JAL | (X_RA << OP_SH_RD), MASK_JAL | MASK_RD, match_opcode,   INSN_ALIAS|WR_xd },
{"jal",       "I",   "d,a",  MATCH_JAL, MASK_JAL, match_opcode,   WR_xd },
{"call",      "I",   "c", (X_T1 << OP_SH_RS1) | (X_RA << OP_SH_RD), (int) M_CALL,  match_never, INSN_MACRO },
{"call",      "I",   "d,c", (X_T1 << OP_SH_RS1), (int) M_CALL,  match_never, INSN_MACRO },
{"tail",      "I",   "c", (X_T1 << OP_SH_RS1), (int) M_CALL,  match_never, INSN_MACRO },
{"jump",      "I",   "c,s", 0, (int) M_CALL,  match_never, INSN_MACRO },
{"nop",       "C",   "",  MATCH_C_ADDI, 0xffff, match_opcode, INSN_ALIAS },
{"nop",       "I",   "",         MATCH_ADDI, MASK_ADDI | MASK_RD | MASK_RS1 | MASK_IMM, match_opcode,  INSN_ALIAS },
{"lui",       "C",   "d,Cu",  MATCH_C_LUI, MASK_C_LUI, match_c_lui, INSN_ALIAS },
{"lui",       "I",   "d,u",  MATCH_LUI, MASK_LUI, match_opcode,   WR_xd },
{"li",        "C",   "d,Cv",  MATCH_C_LUI, MASK_C_LUI, match_c_lui, INSN_ALIAS },
{"li",        "C",   "d,Cj",  MATCH_C_LI, MASK_C_LI, match_rd_nonzero, INSN_ALIAS },
{"li",        "C",   "d,0",  MATCH_C_LI, MASK_C_LI | MASK_RVC_IMM, match_rd_nonzero, INSN_ALIAS },
{"li",        "I",   "d,j",      MATCH_ADDI, MASK_ADDI | MASK_RS1, match_opcode,  INSN_ALIAS|WR_xd }, /* addi */
{"li",        "I",   "d,I",  0,    (int) M_LI,  match_never, INSN_MACRO },
{"mv",        "C",   "d,CV",  MATCH_C_MV, MASK_C_MV, match_c_add, INSN_ALIAS },
{"mv",        "I",   "d,s",  MATCH_ADDI, MASK_ADDI | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"move",      "C",   "d,CV",  MATCH_C_MV, MASK_C_MV, match_c_add, INSN_ALIAS },
{"move",      "I",   "d,s",  MATCH_ADDI, MASK_ADDI | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"andi",      "C",   "Cs,Cw,Cj",  MATCH_C_ANDI, MASK_C_ANDI, match_opcode, INSN_ALIAS },
{"andi",      "I",   "d,s,j",  MATCH_ANDI, MASK_ANDI, match_opcode,   WR_xd|RD_xs1 },
{"and",       "C",   "Cs,Cw,Ct",  MATCH_C_AND, MASK_C_AND, match_opcode, INSN_ALIAS },
{"and",       "C",   "Cs,Ct,Cw",  MATCH_C_AND, MASK_C_AND, match_opcode, INSN_ALIAS },
{"and",       "C",   "Cs,Cw,Cj",  MATCH_C_ANDI, MASK_C_ANDI, match_opcode, INSN_ALIAS },
{"and",       "I",   "d,s,t",  MATCH_AND, MASK_AND, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"and",       "I",   "d,s,j",  MATCH_ANDI, MASK_ANDI, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"beqz",      "C",   "Cs,Cp",  MATCH_C_BEQZ, MASK_C_BEQZ, match_opcode, INSN_ALIAS },
{"beqz",      "I",   "s,p",  MATCH_BEQ, MASK_BEQ | MASK_RS2, match_opcode,   INSN_ALIAS|RD_xs1 },
{"beq",       "I",   "s,t,p",  MATCH_BEQ, MASK_BEQ, match_opcode,   RD_xs1|RD_xs2 },
{"blez",      "I",   "t,p",  MATCH_BGE, MASK_BGE | MASK_RS1, match_opcode,   INSN_ALIAS|RD_xs2 },
{"bgez",      "I",   "s,p",  MATCH_BGE, MASK_BGE | MASK_RS2, match_opcode,   INSN_ALIAS|RD_xs1 },
{"ble",       "I",   "t,s,p",  MATCH_BGE, MASK_BGE, match_opcode,   INSN_ALIAS|RD_xs1|RD_xs2 },
{"bleu",      "I",   "t,s,p",  MATCH_BGEU, MASK_BGEU, match_opcode,   INSN_ALIAS|RD_xs1|RD_xs2 },
{"bge",       "I",   "s,t,p",  MATCH_BGE, MASK_BGE, match_opcode,   RD_xs1|RD_xs2 },
{"bgeu",      "I",   "s,t,p",  MATCH_BGEU, MASK_BGEU, match_opcode,   RD_xs1|RD_xs2 },
{"bltz",      "I",   "s,p",  MATCH_BLT, MASK_BLT | MASK_RS2, match_opcode,   INSN_ALIAS|RD_xs1 },
{"bgtz",      "I",   "t,p",  MATCH_BLT, MASK_BLT | MASK_RS1, match_opcode,   INSN_ALIAS|RD_xs2 },
{"blt",       "I",   "s,t,p",  MATCH_BLT, MASK_BLT, match_opcode,   RD_xs1|RD_xs2 },
{"bltu",      "I",   "s,t,p",  MATCH_BLTU, MASK_BLTU, match_opcode,   RD_xs1|RD_xs2 },
{"bgt",       "I",   "t,s,p",  MATCH_BLT, MASK_BLT, match_opcode,   INSN_ALIAS|RD_xs1|RD_xs2 },
{"bgtu",      "I",   "t,s,p",  MATCH_BLTU, MASK_BLTU, match_opcode,   INSN_ALIAS|RD_xs1|RD_xs2 },
{"bnez",      "C",   "Cs,Cp",  MATCH_C_BNEZ, MASK_C_BNEZ, match_opcode, INSN_ALIAS },
{"bnez",      "I",   "s,p",  MATCH_BNE, MASK_BNE | MASK_RS2, match_opcode,   INSN_ALIAS|RD_xs1 },
{"bne",       "I",   "s,t,p",  MATCH_BNE, MASK_BNE, match_opcode,   RD_xs1|RD_xs2 },
{"addi",      "C",   "Ct,Cc,CK", MATCH_C_ADDI4SPN, MASK_C_ADDI4SPN, match_opcode, INSN_ALIAS },
{"addi",      "C",   "d,CU,Cj",  MATCH_C_ADDI, MASK_C_ADDI, match_rd_nonzero, INSN_ALIAS },
{"addi",      "C",   "Cc,Cc,CL", MATCH_C_ADDI16SP, MASK_C_ADDI16SP, match_opcode, INSN_ALIAS },
{"addi",      "I",   "d,s,j",  MATCH_ADDI, MASK_ADDI, match_opcode,  WR_xd|RD_xs1 },
{"add",       "C",   "d,CU,CV",  MATCH_C_ADD, MASK_C_ADD, match_c_add, INSN_ALIAS },
{"add",       "C",   "d,CV,CU",  MATCH_C_ADD, MASK_C_ADD, match_c_add, INSN_ALIAS },
{"add",       "C",   "d,CU,Cj",  MATCH_C_ADDI, MASK_C_ADDI, match_rd_nonzero, INSN_ALIAS },
{"add",       "C",   "Ct,Cc,CK", MATCH_C_ADDI4SPN, MASK_C_ADDI4SPN, match_opcode, INSN_ALIAS },
{"add",       "C",   "Cc,Cc,CL", MATCH_C_ADDI16SP, MASK_C_ADDI16SP, match_opcode, INSN_ALIAS },
{"add",       "I",   "d,s,t",  MATCH_ADD, MASK_ADD, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"add",       "I",   "d,s,t,0",MATCH_ADD, MASK_ADD, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"add",       "I",   "d,s,j",  MATCH_ADDI, MASK_ADDI, match_opcode,  INSN_ALIAS|WR_xd|RD_xs1 },
{"la",        "I",   "d,A",  0,    (int) M_LA,  match_never, INSN_MACRO },
{"lla",       "I",   "d,A",  0,    (int) M_LLA,  match_never, INSN_MACRO },
{"la.tls.gd", "I",   "d,A",  0,    (int) M_LA_TLS_GD,  match_never, INSN_MACRO },
{"la.tls.ie", "I",   "d,A",  0,    (int) M_LA_TLS_IE,  match_never, INSN_MACRO },
{"neg",       "I",   "d,t",  MATCH_SUB, MASK_SUB | MASK_RS1, match_opcode,   INSN_ALIAS|WR_xd|RD_xs2 }, /* sub 0 */
{"slli",      "C",   "d,CU,C>",  MATCH_C_SLLI, MASK_C_SLLI, match_rd_nonzero, INSN_ALIAS },
{"slli",      "I",   "d,s,>",   MATCH_SLLI, MASK_SLLI, match_opcode,   WR_xd|RD_xs1 },
{"sll",       "C",   "d,CU,C>",  MATCH_C_SLLI, MASK_C_SLLI, match_rd_nonzero, INSN_ALIAS },
{"sll",       "I",   "d,s,t",   MATCH_SLL, MASK_SLL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"sll",       "I",   "d,s,>",   MATCH_SLLI, MASK_SLLI, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"srli",      "C",   "Cs,Cw,C>",  MATCH_C_SRLI, MASK_C_SRLI, match_rd_nonzero, INSN_ALIAS },
{"srli",      "I",   "d,s,>",   MATCH_SRLI, MASK_SRLI, match_opcode,   WR_xd|RD_xs1 },
{"srl",       "C",   "Cs,Cw,C>",  MATCH_C_SRLI, MASK_C_SRLI, match_rd_nonzero, INSN_ALIAS },
{"srl",       "I",   "d,s,t",   MATCH_SRL, MASK_SRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"srl",       "I",   "d,s,>",   MATCH_SRLI, MASK_SRLI, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"srai",      "C",   "Cs,Cw,C>",  MATCH_C_SRAI, MASK_C_SRAI, match_rd_nonzero, INSN_ALIAS },
{"srai",      "I",   "d,s,>",   MATCH_SRAI, MASK_SRAI, match_opcode,   WR_xd|RD_xs1 },
{"sra",       "C",   "Cs,Cw,C>",  MATCH_C_SRAI, MASK_C_SRAI, match_rd_nonzero, INSN_ALIAS },
{"sra",       "I",   "d,s,t",   MATCH_SRA, MASK_SRA, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"sra",       "I",   "d,s,>",   MATCH_SRAI, MASK_SRAI, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"sub",       "C",   "Cs,Cw,Ct",  MATCH_C_SUB, MASK_C_SUB, match_opcode, INSN_ALIAS },
{"sub",       "I",   "d,s,t",  MATCH_SUB, MASK_SUB, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lb",        "I",   "d,o(s)",  MATCH_LB, MASK_LB, match_opcode,   WR_xd|RD_xs1 },
{"lb",        "I",   "d,A",  0, (int) M_LB, match_never, INSN_MACRO },
{"lbu",       "I",   "d,o(s)",  MATCH_LBU, MASK_LBU, match_opcode,   WR_xd|RD_xs1 },
{"lbu",       "I",   "d,A",  0, (int) M_LBU, match_never, INSN_MACRO },
{"lh",        "I",   "d,o(s)",  MATCH_LH, MASK_LH, match_opcode,   WR_xd|RD_xs1 },
{"lh",        "I",   "d,A",  0, (int) M_LH, match_never, INSN_MACRO },
{"lhu",       "I",   "d,o(s)",  MATCH_LHU, MASK_LHU, match_opcode,   WR_xd|RD_xs1 },
{"lhu",       "I",   "d,A",  0, (int) M_LHU, match_never, INSN_MACRO },
{"lw",        "C",   "d,Cm(Cc)",  MATCH_C_LWSP, MASK_C_LWSP, match_rd_nonzero, INSN_ALIAS },
{"lw",        "C",   "Ct,Ck(Cs)",  MATCH_C_LW, MASK_C_LW, match_opcode, INSN_ALIAS },
{"lw",        "I",   "d,o(s)",  MATCH_LW, MASK_LW, match_opcode,   WR_xd|RD_xs1 },
{"lw",        "I",   "d,A",  0, (int) M_LW, match_never, INSN_MACRO },
{"not",       "I",   "d,s",  MATCH_XORI | MASK_IMM, MASK_XORI | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"ori",       "I",   "d,s,j",  MATCH_ORI, MASK_ORI, match_opcode,   WR_xd|RD_xs1 },
{"or",       "C",   "Cs,Cw,Ct",  MATCH_C_OR, MASK_C_OR, match_opcode, INSN_ALIAS },
{"or",       "C",   "Cs,Ct,Cw",  MATCH_C_OR, MASK_C_OR, match_opcode, INSN_ALIAS },
{"or",        "I",   "d,s,t",  MATCH_OR, MASK_OR, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"or",        "I",   "d,s,j",  MATCH_ORI, MASK_ORI, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"auipc",     "I",   "d,u",  MATCH_AUIPC, MASK_AUIPC, match_opcode,  WR_xd },
{"seqz",      "I",   "d,s",  MATCH_SLTIU | ENCODE_ITYPE_IMM(1), MASK_SLTIU | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"snez",      "I",   "d,t",  MATCH_SLTU, MASK_SLTU | MASK_RS1, match_opcode,   INSN_ALIAS|WR_xd|RD_xs2 },
{"sltz",      "I",   "d,s",  MATCH_SLT, MASK_SLT | MASK_RS2, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"sgtz",      "I",   "d,t",  MATCH_SLT, MASK_SLT | MASK_RS1, match_opcode,   INSN_ALIAS|WR_xd|RD_xs2 },
{"slti",      "I",   "d,s,j",  MATCH_SLTI, MASK_SLTI, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"slt",       "I",   "d,s,t",  MATCH_SLT, MASK_SLT, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"slt",       "I",   "d,s,j",  MATCH_SLTI, MASK_SLTI, match_opcode,   WR_xd|RD_xs1 },
{"sltiu",     "I",   "d,s,j",  MATCH_SLTIU, MASK_SLTIU, match_opcode,   WR_xd|RD_xs1 },
{"sltu",      "I",   "d,s,t",  MATCH_SLTU, MASK_SLTU, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"sltu",      "I",   "d,s,j",  MATCH_SLTIU, MASK_SLTIU, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"sgt",       "I",   "d,t,s",  MATCH_SLT, MASK_SLT, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1|RD_xs2 },
{"sgtu",      "I",   "d,t,s",  MATCH_SLTU, MASK_SLTU, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1|RD_xs2 },
{"sb",        "I",   "t,q(s)",  MATCH_SB, MASK_SB, match_opcode,   RD_xs1|RD_xs2 },
{"sb",        "I",   "t,A,s",  0, (int) M_SB, match_never,  INSN_MACRO },
{"sh",        "I",   "t,q(s)",  MATCH_SH, MASK_SH, match_opcode,   RD_xs1|RD_xs2 },
{"sh",        "I",   "t,A,s",  0, (int) M_SH, match_never,  INSN_MACRO },
{"sw",        "C",   "CV,CM(Cc)",  MATCH_C_SWSP, MASK_C_SWSP, match_opcode, INSN_ALIAS },
{"sw",        "C",   "Ct,Ck(Cs)",  MATCH_C_SW, MASK_C_SW, match_opcode, INSN_ALIAS },
{"sw",        "I",   "t,q(s)",  MATCH_SW, MASK_SW, match_opcode,   RD_xs1|RD_xs2 },
{"sw",        "I",   "t,A,s",  0, (int) M_SW, match_never,  INSN_MACRO },
{"fence",     "I",   "",  MATCH_FENCE | MASK_PRED | MASK_SUCC, MASK_FENCE | MASK_RD | MASK_RS1 | MASK_IMM, match_opcode,   INSN_ALIAS },
{"fence",     "I",   "P,Q",  MATCH_FENCE, MASK_FENCE | MASK_RD | MASK_RS1 | (MASK_IMM & ~MASK_PRED & ~MASK_SUCC), match_opcode,   0 },
{"fence.i",   "I",   "",  MATCH_FENCE_I, MASK_FENCE | MASK_RD | MASK_RS1 | MASK_IMM, match_opcode,   0 },
{"rdcycle",   "I",   "d",  MATCH_RDCYCLE, MASK_RDCYCLE, match_opcode,  WR_xd },
{"rdinstret", "I",   "d",  MATCH_RDINSTRET, MASK_RDINSTRET, match_opcode,  WR_xd },
{"rdtime",    "I",   "d",  MATCH_RDTIME, MASK_RDTIME, match_opcode,  WR_xd },
{"rdcycleh",  "32I", "d",  MATCH_RDCYCLEH, MASK_RDCYCLEH, match_opcode,  WR_xd },
{"rdinstreth","32I", "d",  MATCH_RDINSTRETH, MASK_RDINSTRETH, match_opcode,  WR_xd },
{"rdtimeh",   "32I", "d",  MATCH_RDTIMEH, MASK_RDTIMEH, match_opcode,  WR_xd },
{"ecall",     "I",   "",    MATCH_SCALL, MASK_SCALL, match_opcode,   0 },
{"scall",     "I",   "",   MATCH_SCALL, MASK_SCALL, match_opcode,   0 },
{"scallimm",  "I",   "b3",   MATCH_SCALL, MASK_SCALL_IMM, match_opcode,   0 },
{"xori",      "I",   "d,s,j",  MATCH_XORI, MASK_XORI, match_opcode,   WR_xd|RD_xs1 },
{"xor",       "C",   "Cs,Cw,Ct",  MATCH_C_XOR, MASK_C_XOR, match_opcode, INSN_ALIAS },
{"xor",       "C",   "Cs,Ct,Cw",  MATCH_C_XOR, MASK_C_XOR, match_opcode, INSN_ALIAS },
{"xor",       "I",   "d,s,t",  MATCH_XOR, MASK_XOR, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"xor",       "I",   "d,s,j",  MATCH_XORI, MASK_XORI, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"lwu",       "64I", "d,o(s)",  MATCH_LWU, MASK_LWU, match_opcode,   WR_xd|RD_xs1 },
{"lwu",       "64I", "d,A",  0, (int) M_LWU, match_never, INSN_MACRO },
{"ld",        "64C", "d,Cn(Cc)",  MATCH_C_LDSP, MASK_C_LDSP, match_rd_nonzero, INSN_ALIAS },
{"ld",        "64C", "Ct,Cl(Cs)",  MATCH_C_LD, MASK_C_LD, match_opcode, INSN_ALIAS },
{"ld",        "64I", "d,o(s)", MATCH_LD, MASK_LD, match_opcode,  WR_xd|RD_xs1 },
{"ld",        "64I", "d,A",  0, (int) M_LD, match_never, INSN_MACRO },
{"sd",        "64C", "CV,CN(Cc)",  MATCH_C_SDSP, MASK_C_SDSP, match_opcode, INSN_ALIAS },
{"sd",        "64C", "Ct,Cl(Cs)",  MATCH_C_SD, MASK_C_SD, match_opcode, INSN_ALIAS },
{"sd",        "64I", "t,q(s)",  MATCH_SD, MASK_SD, match_opcode,   RD_xs1|RD_xs2 },
{"sd",        "64I", "t,A,s",  0, (int) M_SD, match_never,  INSN_MACRO },
{"sext.w",    "64C", "d,CU",  MATCH_C_ADDIW, MASK_C_ADDIW | MASK_RVC_IMM, match_rd_nonzero, INSN_ALIAS },
{"sext.w",    "64I", "d,s",  MATCH_ADDIW, MASK_ADDIW | MASK_IMM, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"addiw",     "64C", "d,CU,Cj",  MATCH_C_ADDIW, MASK_C_ADDIW, match_rd_nonzero, INSN_ALIAS },
{"addiw",     "64I", "d,s,j",  MATCH_ADDIW, MASK_ADDIW, match_opcode,   WR_xd|RD_xs1 },
{"addw",      "64C", "Cs,Cw,Ct",  MATCH_C_ADDW, MASK_C_ADDW, match_opcode, INSN_ALIAS },
{"addw",      "64C", "Cs,Ct,Cw",  MATCH_C_ADDW, MASK_C_ADDW, match_opcode, INSN_ALIAS },
{"addw",      "64C", "d,CU,Cj",  MATCH_C_ADDIW, MASK_C_ADDIW, match_rd_nonzero, INSN_ALIAS },
{"addw",      "64I", "d,s,t",  MATCH_ADDW, MASK_ADDW, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"addw",      "64I", "d,s,j",  MATCH_ADDIW, MASK_ADDIW, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"negw",      "64I", "d,t",  MATCH_SUBW, MASK_SUBW | MASK_RS1, match_opcode,   INSN_ALIAS|WR_xd|RD_xs2 }, /* sub 0 */
{"slliw",     "64I", "d,s,<",   MATCH_SLLIW, MASK_SLLIW, match_opcode,   WR_xd|RD_xs1 },
{"sllw",      "64I", "d,s,t",   MATCH_SLLW, MASK_SLLW, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"sllw",      "64I", "d,s,<",   MATCH_SLLIW, MASK_SLLIW, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"srliw",     "64I", "d,s,<",   MATCH_SRLIW, MASK_SRLIW, match_opcode,   WR_xd|RD_xs1 },
{"srlw",      "64I", "d,s,t",   MATCH_SRLW, MASK_SRLW, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"srlw",      "64I", "d,s,<",   MATCH_SRLIW, MASK_SRLIW, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"sraiw",     "64I", "d,s,<",   MATCH_SRAIW, MASK_SRAIW, match_opcode,   WR_xd|RD_xs1 },
{"sraw",      "64I", "d,s,t",   MATCH_SRAW, MASK_SRAW, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"sraw",      "64I", "d,s,<",   MATCH_SRAIW, MASK_SRAIW, match_opcode,   INSN_ALIAS|WR_xd|RD_xs1 },
{"subw",      "64C", "Cs,Cw,Ct",  MATCH_C_SUBW, MASK_C_SUBW, match_opcode, INSN_ALIAS },
{"subw",      "64I", "d,s,t",  MATCH_SUBW, MASK_SUBW, match_opcode,   WR_xd|RD_xs1|RD_xs2 },

/* Atomic memory operation instruction subset */
{"lr.w",         "A",   "d,0(s)",    MATCH_LR_W, MASK_LR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.w",         "A",   "d,t,0(s)",  MATCH_SC_W, MASK_SC_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.w",     "A",   "d,t,0(s)",  MATCH_AMOADD_W, MASK_AMOADD_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.w",    "A",   "d,t,0(s)",  MATCH_AMOSWAP_W, MASK_AMOSWAP_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.w",     "A",   "d,t,0(s)",  MATCH_AMOAND_W, MASK_AMOAND_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.w",      "A",   "d,t,0(s)",  MATCH_AMOOR_W, MASK_AMOOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.w",     "A",   "d,t,0(s)",  MATCH_AMOXOR_W, MASK_AMOXOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.w",     "A",   "d,t,0(s)",  MATCH_AMOMAX_W, MASK_AMOMAX_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.w",    "A",   "d,t,0(s)",  MATCH_AMOMAXU_W, MASK_AMOMAXU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.w",     "A",   "d,t,0(s)",  MATCH_AMOMIN_W, MASK_AMOMIN_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.w",    "A",   "d,t,0(s)",  MATCH_AMOMINU_W, MASK_AMOMINU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lr.w.aq",      "A",   "d,0(s)",    MATCH_LR_W | MASK_AQ, MASK_LR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.w.aq",      "A",   "d,t,0(s)",  MATCH_SC_W | MASK_AQ, MASK_SC_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.w.aq",  "A",   "d,t,0(s)",  MATCH_AMOADD_W | MASK_AQ, MASK_AMOADD_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.w.aq", "A",   "d,t,0(s)",  MATCH_AMOSWAP_W | MASK_AQ, MASK_AMOSWAP_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.w.aq",  "A",   "d,t,0(s)",  MATCH_AMOAND_W | MASK_AQ, MASK_AMOAND_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.w.aq",   "A",   "d,t,0(s)",  MATCH_AMOOR_W | MASK_AQ, MASK_AMOOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.w.aq",  "A",   "d,t,0(s)",  MATCH_AMOXOR_W | MASK_AQ, MASK_AMOXOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.w.aq",  "A",   "d,t,0(s)",  MATCH_AMOMAX_W | MASK_AQ, MASK_AMOMAX_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.w.aq", "A",   "d,t,0(s)",  MATCH_AMOMAXU_W | MASK_AQ, MASK_AMOMAXU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.w.aq",  "A",   "d,t,0(s)",  MATCH_AMOMIN_W | MASK_AQ, MASK_AMOMIN_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.w.aq", "A",   "d,t,0(s)",  MATCH_AMOMINU_W | MASK_AQ, MASK_AMOMINU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lr.w.rl",      "A",   "d,0(s)",    MATCH_LR_W | MASK_RL, MASK_LR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.w.rl",      "A",   "d,t,0(s)",  MATCH_SC_W | MASK_RL, MASK_SC_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.w.rl",  "A",   "d,t,0(s)",  MATCH_AMOADD_W | MASK_RL, MASK_AMOADD_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.w.rl", "A",   "d,t,0(s)",  MATCH_AMOSWAP_W | MASK_RL, MASK_AMOSWAP_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.w.rl",  "A",   "d,t,0(s)",  MATCH_AMOAND_W | MASK_RL, MASK_AMOAND_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.w.rl",   "A",   "d,t,0(s)",  MATCH_AMOOR_W | MASK_RL, MASK_AMOOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.w.rl",  "A",   "d,t,0(s)",  MATCH_AMOXOR_W | MASK_RL, MASK_AMOXOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.w.rl",  "A",   "d,t,0(s)",  MATCH_AMOMAX_W | MASK_RL, MASK_AMOMAX_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.w.rl", "A",   "d,t,0(s)",  MATCH_AMOMAXU_W | MASK_RL, MASK_AMOMAXU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.w.rl",  "A",   "d,t,0(s)",  MATCH_AMOMIN_W | MASK_RL, MASK_AMOMIN_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.w.rl", "A",   "d,t,0(s)",  MATCH_AMOMINU_W | MASK_RL, MASK_AMOMINU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lr.w.sc",      "A",   "d,0(s)",    MATCH_LR_W | MASK_AQRL, MASK_LR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.w.sc",      "A",   "d,t,0(s)",  MATCH_SC_W | MASK_AQRL, MASK_SC_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.w.sc",  "A",   "d,t,0(s)",  MATCH_AMOADD_W | MASK_AQRL, MASK_AMOADD_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.w.sc", "A",   "d,t,0(s)",  MATCH_AMOSWAP_W | MASK_AQRL, MASK_AMOSWAP_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.w.sc",  "A",   "d,t,0(s)",  MATCH_AMOAND_W | MASK_AQRL, MASK_AMOAND_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.w.sc",   "A",   "d,t,0(s)",  MATCH_AMOOR_W | MASK_AQRL, MASK_AMOOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.w.sc",  "A",   "d,t,0(s)",  MATCH_AMOXOR_W | MASK_AQRL, MASK_AMOXOR_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.w.sc",  "A",   "d,t,0(s)",  MATCH_AMOMAX_W | MASK_AQRL, MASK_AMOMAX_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.w.sc", "A",   "d,t,0(s)",  MATCH_AMOMAXU_W | MASK_AQRL, MASK_AMOMAXU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.w.sc",  "A",   "d,t,0(s)",  MATCH_AMOMIN_W | MASK_AQRL, MASK_AMOMIN_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.w.sc", "A",   "d,t,0(s)",  MATCH_AMOMINU_W | MASK_AQRL, MASK_AMOMINU_W | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lr.d",         "64A", "d,0(s)",    MATCH_LR_D, MASK_LR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.d",         "64A", "d,t,0(s)",  MATCH_SC_D, MASK_SC_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.d",     "64A", "d,t,0(s)",  MATCH_AMOADD_D, MASK_AMOADD_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.d",    "64A", "d,t,0(s)",  MATCH_AMOSWAP_D, MASK_AMOSWAP_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.d",     "64A", "d,t,0(s)",  MATCH_AMOAND_D, MASK_AMOAND_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.d",      "64A", "d,t,0(s)",  MATCH_AMOOR_D, MASK_AMOOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.d",     "64A", "d,t,0(s)",  MATCH_AMOXOR_D, MASK_AMOXOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.d",     "64A", "d,t,0(s)",  MATCH_AMOMAX_D, MASK_AMOMAX_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.d",    "64A", "d,t,0(s)",  MATCH_AMOMAXU_D, MASK_AMOMAXU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.d",     "64A", "d,t,0(s)",  MATCH_AMOMIN_D, MASK_AMOMIN_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.d",    "64A", "d,t,0(s)",  MATCH_AMOMINU_D, MASK_AMOMINU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lr.d.aq",      "64A", "d,0(s)",    MATCH_LR_D | MASK_AQ, MASK_LR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.d.aq",      "64A", "d,t,0(s)",  MATCH_SC_D | MASK_AQ, MASK_SC_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.d.aq",  "64A", "d,t,0(s)",  MATCH_AMOADD_D | MASK_AQ, MASK_AMOADD_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.d.aq", "64A", "d,t,0(s)",  MATCH_AMOSWAP_D | MASK_AQ, MASK_AMOSWAP_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.d.aq",  "64A", "d,t,0(s)",  MATCH_AMOAND_D | MASK_AQ, MASK_AMOAND_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.d.aq",   "64A", "d,t,0(s)",  MATCH_AMOOR_D | MASK_AQ, MASK_AMOOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.d.aq",  "64A", "d,t,0(s)",  MATCH_AMOXOR_D | MASK_AQ, MASK_AMOXOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.d.aq",  "64A", "d,t,0(s)",  MATCH_AMOMAX_D | MASK_AQ, MASK_AMOMAX_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.d.aq", "64A", "d,t,0(s)",  MATCH_AMOMAXU_D | MASK_AQ, MASK_AMOMAXU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.d.aq",  "64A", "d,t,0(s)",  MATCH_AMOMIN_D | MASK_AQ, MASK_AMOMIN_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.d.aq", "64A", "d,t,0(s)",  MATCH_AMOMINU_D | MASK_AQ, MASK_AMOMINU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lr.d.rl",      "64A", "d,0(s)",    MATCH_LR_D | MASK_RL, MASK_LR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.d.rl",      "64A", "d,t,0(s)",  MATCH_SC_D | MASK_RL, MASK_SC_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.d.rl",  "64A", "d,t,0(s)",  MATCH_AMOADD_D | MASK_RL, MASK_AMOADD_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.d.rl", "64A", "d,t,0(s)",  MATCH_AMOSWAP_D | MASK_RL, MASK_AMOSWAP_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.d.rl",  "64A", "d,t,0(s)",  MATCH_AMOAND_D | MASK_RL, MASK_AMOAND_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.d.rl",   "64A", "d,t,0(s)",  MATCH_AMOOR_D | MASK_RL, MASK_AMOOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.d.rl",  "64A", "d,t,0(s)",  MATCH_AMOXOR_D | MASK_RL, MASK_AMOXOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.d.rl",  "64A", "d,t,0(s)",  MATCH_AMOMAX_D | MASK_RL, MASK_AMOMAX_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.d.rl", "64A", "d,t,0(s)",  MATCH_AMOMAXU_D | MASK_RL, MASK_AMOMAXU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.d.rl",  "64A", "d,t,0(s)",  MATCH_AMOMIN_D | MASK_RL, MASK_AMOMIN_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.d.rl", "64A", "d,t,0(s)",  MATCH_AMOMINU_D | MASK_RL, MASK_AMOMINU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"lr.d.sc",      "64A", "d,0(s)",    MATCH_LR_D | MASK_AQRL, MASK_LR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1 },
{"sc.d.sc",      "64A", "d,t,0(s)",  MATCH_SC_D | MASK_AQRL, MASK_SC_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoadd.d.sc",  "64A", "d,t,0(s)",  MATCH_AMOADD_D | MASK_AQRL, MASK_AMOADD_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoswap.d.sc", "64A", "d,t,0(s)",  MATCH_AMOSWAP_D | MASK_AQRL, MASK_AMOSWAP_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoand.d.sc",  "64A", "d,t,0(s)",  MATCH_AMOAND_D | MASK_AQRL, MASK_AMOAND_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoor.d.sc",   "64A", "d,t,0(s)",  MATCH_AMOOR_D | MASK_AQRL, MASK_AMOOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amoxor.d.sc",  "64A", "d,t,0(s)",  MATCH_AMOXOR_D | MASK_AQRL, MASK_AMOXOR_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomax.d.sc",  "64A", "d,t,0(s)",  MATCH_AMOMAX_D | MASK_AQRL, MASK_AMOMAX_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomaxu.d.sc", "64A", "d,t,0(s)",  MATCH_AMOMAXU_D | MASK_AQRL, MASK_AMOMAXU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amomin.d.sc",  "64A", "d,t,0(s)",  MATCH_AMOMIN_D | MASK_AQRL, MASK_AMOMIN_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },
{"amominu.d.sc", "64A", "d,t,0(s)",  MATCH_AMOMINU_D | MASK_AQRL, MASK_AMOMINU_D | MASK_AQRL, match_opcode,   WR_xd|RD_xs1|RD_xs2 },

/* Multiply/Divide instruction subset */
{"mul",       "M",   "d,s,t",  MATCH_MUL, MASK_MUL, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"mulh",      "M",   "d,s,t",  MATCH_MULH, MASK_MULH, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"mulhu",     "M",   "d,s,t",  MATCH_MULHU, MASK_MULHU, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"mulhsu",    "M",   "d,s,t",  MATCH_MULHSU, MASK_MULHSU, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"div",       "M",   "d,s,t",  MATCH_DIV, MASK_DIV, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"divu",      "M",   "d,s,t",  MATCH_DIVU, MASK_DIVU, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"rem",       "M",   "d,s,t",  MATCH_REM, MASK_REM, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"remu",      "M",   "d,s,t",  MATCH_REMU, MASK_REMU, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"mulw",      "64M", "d,s,t",  MATCH_MULW, MASK_MULW, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"divw",      "64M", "d,s,t",  MATCH_DIVW, MASK_DIVW, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"divuw",     "64M", "d,s,t",  MATCH_DIVUW, MASK_DIVUW, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"remw",      "64M", "d,s,t",  MATCH_REMW, MASK_REMW, match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"remuw",     "64M", "d,s,t",  MATCH_REMUW, MASK_REMUW, match_opcode,  WR_xd|RD_xs1|RD_xs2 },

/* Single-precision floating-point instruction subset */
{"frsr",      "F",   "d",  MATCH_FRCSR, MASK_FRCSR, match_opcode,  WR_xd },
{"fssr",      "F",   "s",  MATCH_FSCSR, MASK_FSCSR | MASK_RD, match_opcode,  RD_xs1 },
{"fssr",      "F",   "d,s",  MATCH_FSCSR, MASK_FSCSR, match_opcode,  WR_xd|RD_xs1 },
{"frcsr",     "F",   "d",  MATCH_FRCSR, MASK_FRCSR, match_opcode,  WR_xd },
{"fscsr",     "F",   "s",  MATCH_FSCSR, MASK_FSCSR | MASK_RD, match_opcode,  RD_xs1 },
{"fscsr",     "F",   "d,s",  MATCH_FSCSR, MASK_FSCSR, match_opcode,  WR_xd|RD_xs1 },
{"frrm",      "F",   "d",  MATCH_FRRM, MASK_FRRM, match_opcode,  WR_xd },
{"fsrm",      "F",   "s",  MATCH_FSRM, MASK_FSRM | MASK_RD, match_opcode,  RD_xs1 },
{"fsrm",      "F",   "d,s",  MATCH_FSRM, MASK_FSRM, match_opcode,  WR_xd|RD_xs1 },
{"frflags",   "F",   "d",  MATCH_FRFLAGS, MASK_FRFLAGS, match_opcode,  WR_xd },
{"fsflags",   "F",   "s",  MATCH_FSFLAGS, MASK_FSFLAGS | MASK_RD, match_opcode,  RD_xs1 },
{"fsflags",   "F",   "d,s",  MATCH_FSFLAGS, MASK_FSFLAGS, match_opcode,  WR_xd|RD_xs1 },
{"flw",       "32C", "D,Cm(Cc)",  MATCH_C_FLWSP, MASK_C_FLWSP, match_opcode, INSN_ALIAS },
{"flw",       "32C", "CD,Ck(Cs)",  MATCH_C_FLW, MASK_C_FLW, match_opcode, INSN_ALIAS },
{"flw",       "F",   "D,o(s)",  MATCH_FLW, MASK_FLW, match_opcode,   WR_fd|RD_xs1 },
{"flw",       "F",   "D,A,s",  0, (int) M_FLW, match_never,  INSN_MACRO },
{"fsw",       "32C", "CT,CM(Cc)",  MATCH_C_FSWSP, MASK_C_FSWSP, match_opcode, INSN_ALIAS },
{"fsw",       "32C", "CD,Ck(Cs)",  MATCH_C_FSW, MASK_C_FSW, match_opcode, INSN_ALIAS },
{"fsw",       "F",   "T,q(s)",  MATCH_FSW, MASK_FSW, match_opcode,   RD_xs1|RD_fs2 },
{"fsw",       "F",   "T,A,s",  0, (int) M_FSW, match_never,  INSN_MACRO },
{"fmv.x.s",   "F",   "d,S",  MATCH_FMV_X_S, MASK_FMV_X_S, match_opcode,  WR_xd|RD_fs1 },
{"fmv.s.x",   "F",   "D,s",  MATCH_FMV_S_X, MASK_FMV_S_X, match_opcode,  WR_fd|RD_xs1 },
{"fmv.s",     "F",   "D,U",  MATCH_FSGNJ_S, MASK_FSGNJ_S, match_rs1_eq_rs2,   INSN_ALIAS|WR_fd|RD_fs1|RD_fs2 },
{"fneg.s",    "F",   "D,U",  MATCH_FSGNJN_S, MASK_FSGNJN_S, match_rs1_eq_rs2,   INSN_ALIAS|WR_fd|RD_fs1|RD_fs2 },
{"fabs.s",    "F",   "D,U",  MATCH_FSGNJX_S, MASK_FSGNJX_S, match_rs1_eq_rs2,   INSN_ALIAS|WR_fd|RD_fs1|RD_fs2 },
{"fsgnj.s",   "F",   "D,S,T",  MATCH_FSGNJ_S, MASK_FSGNJ_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsgnjn.s",  "F",   "D,S,T",  MATCH_FSGNJN_S, MASK_FSGNJN_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsgnjx.s",  "F",   "D,S,T",  MATCH_FSGNJX_S, MASK_FSGNJX_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fadd.s",    "F",   "D,S,T",  MATCH_FADD_S | MASK_RM, MASK_FADD_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fadd.s",    "F",   "D,S,T,m",  MATCH_FADD_S, MASK_FADD_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsub.s",    "F",   "D,S,T",  MATCH_FSUB_S | MASK_RM, MASK_FSUB_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsub.s",    "F",   "D,S,T,m",  MATCH_FSUB_S, MASK_FSUB_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmul.s",    "F",   "D,S,T",  MATCH_FMUL_S | MASK_RM, MASK_FMUL_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmul.s",    "F",   "D,S,T,m",  MATCH_FMUL_S, MASK_FMUL_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fdiv.s",    "F",   "D,S,T",  MATCH_FDIV_S | MASK_RM, MASK_FDIV_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fdiv.s",    "F",   "D,S,T,m",  MATCH_FDIV_S, MASK_FDIV_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsqrt.s",   "F",   "D,S",  MATCH_FSQRT_S | MASK_RM, MASK_FSQRT_S | MASK_RM, match_opcode,  WR_fd|RD_fs1 },
{"fsqrt.s",   "F",   "D,S,m",  MATCH_FSQRT_S, MASK_FSQRT_S, match_opcode,  WR_fd|RD_fs1 },
{"fmin.s",    "F",   "D,S,T",  MATCH_FMIN_S, MASK_FMIN_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmax.s",    "F",   "D,S,T",  MATCH_FMAX_S, MASK_FMAX_S, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmadd.s",   "F",   "D,S,T,R",  MATCH_FMADD_S | MASK_RM, MASK_FMADD_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fmadd.s",   "F",   "D,S,T,R,m",  MATCH_FMADD_S, MASK_FMADD_S, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmadd.s",  "F",   "D,S,T,R",  MATCH_FNMADD_S | MASK_RM, MASK_FNMADD_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmadd.s",  "F",   "D,S,T,R,m",  MATCH_FNMADD_S, MASK_FNMADD_S, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fmsub.s",   "F",   "D,S,T,R",  MATCH_FMSUB_S | MASK_RM, MASK_FMSUB_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fmsub.s",   "F",   "D,S,T,R,m",  MATCH_FMSUB_S, MASK_FMSUB_S, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmsub.s",  "F",   "D,S,T,R",  MATCH_FNMSUB_S | MASK_RM, MASK_FNMSUB_S | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmsub.s",  "F",   "D,S,T,R,m",  MATCH_FNMSUB_S, MASK_FNMSUB_S, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fcvt.w.s",  "F",   "d,S",  MATCH_FCVT_W_S | MASK_RM, MASK_FCVT_W_S | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.w.s",  "F",   "d,S,m",  MATCH_FCVT_W_S, MASK_FCVT_W_S, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.wu.s", "F",   "d,S",  MATCH_FCVT_WU_S | MASK_RM, MASK_FCVT_WU_S | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.wu.s", "F",   "d,S,m",  MATCH_FCVT_WU_S, MASK_FCVT_WU_S, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.s.w",  "F",   "D,s",  MATCH_FCVT_S_W | MASK_RM, MASK_FCVT_S_W | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.s.w",  "F",   "D,s,m",  MATCH_FCVT_S_W, MASK_FCVT_S_W, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.s.wu", "F",   "D,s",  MATCH_FCVT_S_WU | MASK_RM, MASK_FCVT_S_W | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.s.wu", "F",   "D,s,m",  MATCH_FCVT_S_WU, MASK_FCVT_S_WU, match_opcode,   WR_fd|RD_xs1 },
{"fclass.s",  "F",   "d,S",  MATCH_FCLASS_S, MASK_FCLASS_S, match_opcode,   WR_xd|RD_fs1 },
{"feq.s",     "F",   "d,S,T",    MATCH_FEQ_S, MASK_FEQ_S, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"flt.s",     "F",   "d,S,T",    MATCH_FLT_S, MASK_FLT_S, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fle.s",     "F",   "d,S,T",    MATCH_FLE_S, MASK_FLE_S, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fgt.s",     "F",   "d,T,S",    MATCH_FLT_S, MASK_FLT_S, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fge.s",     "F",   "d,T,S",    MATCH_FLE_S, MASK_FLE_S, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fcvt.l.s",  "64F", "d,S",  MATCH_FCVT_L_S | MASK_RM, MASK_FCVT_L_S | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.l.s",  "64F", "d,S,m",  MATCH_FCVT_L_S, MASK_FCVT_L_S, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.lu.s", "64F", "d,S",  MATCH_FCVT_LU_S | MASK_RM, MASK_FCVT_LU_S | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.lu.s", "64F", "d,S,m",  MATCH_FCVT_LU_S, MASK_FCVT_LU_S, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.s.l",  "64F", "D,s",  MATCH_FCVT_S_L | MASK_RM, MASK_FCVT_S_L | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.s.l",  "64F", "D,s,m",  MATCH_FCVT_S_L, MASK_FCVT_S_L, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.s.lu", "64F", "D,s",  MATCH_FCVT_S_LU | MASK_RM, MASK_FCVT_S_L | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.s.lu", "64F", "D,s,m",  MATCH_FCVT_S_LU, MASK_FCVT_S_LU, match_opcode,   WR_fd|RD_xs1 },

/* Double-precision floating-point instruction subset */
{"fld",       "C",   "D,Cn(Cc)",  MATCH_C_FLDSP, MASK_C_FLDSP, match_opcode, INSN_ALIAS },
{"fld",       "C",   "CD,Cl(Cs)",  MATCH_C_FLD, MASK_C_FLD, match_opcode, INSN_ALIAS },
{"fld",       "D",   "D,o(s)",  MATCH_FLD, MASK_FLD, match_opcode,  WR_fd|RD_xs1 },
{"fld",       "D",   "D,A,s",  0, (int) M_FLD, match_never,  INSN_MACRO },
{"fsd",       "C",   "CT,CN(Cc)",  MATCH_C_FSDSP, MASK_C_FSDSP, match_opcode, INSN_ALIAS },
{"fsd",       "C",   "CD,Cl(Cs)",  MATCH_C_FSD, MASK_C_FSD, match_opcode, INSN_ALIAS },
{"fsd",       "D",   "T,q(s)",  MATCH_FSD, MASK_FSD, match_opcode,  RD_xs1|RD_fs2 },
{"fsd",       "D",   "T,A,s",  0, (int) M_FSD, match_never,  INSN_MACRO },
{"fmv.d",     "D",   "D,U",  MATCH_FSGNJ_D, MASK_FSGNJ_D, match_rs1_eq_rs2,   INSN_ALIAS|WR_fd|RD_fs1|RD_fs2 },
{"fneg.d",    "D",   "D,U",  MATCH_FSGNJN_D, MASK_FSGNJN_D, match_rs1_eq_rs2,   INSN_ALIAS|WR_fd|RD_fs1|RD_fs2 },
{"fabs.d",    "D",   "D,U",  MATCH_FSGNJX_D, MASK_FSGNJX_D, match_rs1_eq_rs2,   INSN_ALIAS|WR_fd|RD_fs1|RD_fs2 },
{"fsgnj.d",   "D",   "D,S,T",  MATCH_FSGNJ_D, MASK_FSGNJ_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsgnjn.d",  "D",   "D,S,T",  MATCH_FSGNJN_D, MASK_FSGNJN_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsgnjx.d",  "D",   "D,S,T",  MATCH_FSGNJX_D, MASK_FSGNJX_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fadd.d",    "D",   "D,S,T",  MATCH_FADD_D | MASK_RM, MASK_FADD_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fadd.d",    "D",   "D,S,T,m",  MATCH_FADD_D, MASK_FADD_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsub.d",    "D",   "D,S,T",  MATCH_FSUB_D | MASK_RM, MASK_FSUB_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsub.d",    "D",   "D,S,T,m",  MATCH_FSUB_D, MASK_FSUB_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmul.d",    "D",   "D,S,T",  MATCH_FMUL_D | MASK_RM, MASK_FMUL_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmul.d",    "D",   "D,S,T,m",  MATCH_FMUL_D, MASK_FMUL_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fdiv.d",    "D",   "D,S,T",  MATCH_FDIV_D | MASK_RM, MASK_FDIV_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fdiv.d",    "D",   "D,S,T,m",  MATCH_FDIV_D, MASK_FDIV_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fsqrt.d",   "D",   "D,S",  MATCH_FSQRT_D | MASK_RM, MASK_FSQRT_D | MASK_RM, match_opcode,  WR_fd|RD_fs1 },
{"fsqrt.d",   "D",   "D,S,m",  MATCH_FSQRT_D, MASK_FSQRT_D, match_opcode,  WR_fd|RD_fs1 },
{"fmin.d",    "D",   "D,S,T",  MATCH_FMIN_D, MASK_FMIN_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmax.d",    "D",   "D,S,T",  MATCH_FMAX_D, MASK_FMAX_D, match_opcode,   WR_fd|RD_fs1|RD_fs2 },
{"fmadd.d",   "D",   "D,S,T,R",  MATCH_FMADD_D | MASK_RM, MASK_FMADD_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fmadd.d",   "D",   "D,S,T,R,m",  MATCH_FMADD_D, MASK_FMADD_D, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmadd.d",  "D",   "D,S,T,R",  MATCH_FNMADD_D | MASK_RM, MASK_FNMADD_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmadd.d",  "D",   "D,S,T,R,m",  MATCH_FNMADD_D, MASK_FNMADD_D, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fmsub.d",   "D",   "D,S,T,R",  MATCH_FMSUB_D | MASK_RM, MASK_FMSUB_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fmsub.d",   "D",   "D,S,T,R,m",  MATCH_FMSUB_D, MASK_FMSUB_D, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmsub.d",  "D",   "D,S,T,R",  MATCH_FNMSUB_D | MASK_RM, MASK_FNMSUB_D | MASK_RM, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fnmsub.d",  "D",   "D,S,T,R,m",  MATCH_FNMSUB_D, MASK_FNMSUB_D, match_opcode,   WR_fd|RD_fs1|RD_fs2|RD_fs3 },
{"fcvt.w.d",  "D",   "d,S",  MATCH_FCVT_W_D | MASK_RM, MASK_FCVT_W_D | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.w.d",  "D",   "d,S,m",  MATCH_FCVT_W_D, MASK_FCVT_W_D, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.wu.d", "D",   "d,S",  MATCH_FCVT_WU_D | MASK_RM, MASK_FCVT_WU_D | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.wu.d", "D",   "d,S,m",  MATCH_FCVT_WU_D, MASK_FCVT_WU_D, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.d.w",  "D",   "D,s",  MATCH_FCVT_D_W, MASK_FCVT_D_W | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.d.wu", "D",   "D,s",  MATCH_FCVT_D_WU, MASK_FCVT_D_WU | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.d.s",  "D",   "D,S",  MATCH_FCVT_D_S, MASK_FCVT_D_S | MASK_RM, match_opcode,   WR_fd|RD_fs1 },
{"fcvt.s.d",  "D",   "D,S",  MATCH_FCVT_S_D | MASK_RM, MASK_FCVT_S_D | MASK_RM, match_opcode,   WR_fd|RD_fs1 },
{"fcvt.s.d",  "D",   "D,S,m",  MATCH_FCVT_S_D, MASK_FCVT_S_D, match_opcode,   WR_fd|RD_fs1 },
{"fclass.d",  "D",   "d,S",  MATCH_FCLASS_D, MASK_FCLASS_D, match_opcode,   WR_xd|RD_fs1 },
{"feq.d",     "D",   "d,S,T",    MATCH_FEQ_D, MASK_FEQ_D, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"flt.d",     "D",   "d,S,T",    MATCH_FLT_D, MASK_FLT_D, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fle.d",     "D",   "d,S,T",    MATCH_FLE_D, MASK_FLE_D, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fgt.d",     "D",   "d,T,S",    MATCH_FLT_D, MASK_FLT_D, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fge.d",     "D",   "d,T,S",    MATCH_FLE_D, MASK_FLE_D, match_opcode,  WR_xd|RD_fs1|RD_fs2 },
{"fmv.x.d",   "64D", "d,S",  MATCH_FMV_X_D, MASK_FMV_X_D, match_opcode,  WR_xd|RD_fs1 },
{"fmv.d.x",   "64D", "D,s",  MATCH_FMV_D_X, MASK_FMV_D_X, match_opcode,  WR_fd|RD_xs1 },
{"fcvt.l.d",  "64D", "d,S",  MATCH_FCVT_L_D | MASK_RM, MASK_FCVT_L_D | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.l.d",  "64D", "d,S,m",  MATCH_FCVT_L_D, MASK_FCVT_L_D, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.lu.d", "64D", "d,S",  MATCH_FCVT_LU_D | MASK_RM, MASK_FCVT_LU_D | MASK_RM, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.lu.d", "64D", "d,S,m",  MATCH_FCVT_LU_D, MASK_FCVT_LU_D, match_opcode,  WR_xd|RD_fs1 },
{"fcvt.d.l",  "64D", "D,s",  MATCH_FCVT_D_L | MASK_RM, MASK_FCVT_D_L | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.d.l",  "64D", "D,s,m",  MATCH_FCVT_D_L, MASK_FCVT_D_L, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.d.lu", "64D", "D,s",  MATCH_FCVT_D_LU | MASK_RM, MASK_FCVT_D_L | MASK_RM, match_opcode,   WR_fd|RD_xs1 },
{"fcvt.d.lu", "64D", "D,s,m",  MATCH_FCVT_D_LU, MASK_FCVT_D_LU, match_opcode,   WR_fd|RD_xs1 },

/* Compressed instructions */
{"c.ebreak",  "C",   "",  MATCH_C_EBREAK, MASK_C_EBREAK, match_opcode, 0 },
{"c.jr",      "C",   "d",  MATCH_C_JR, MASK_C_JR, match_rd_nonzero, 0 },
{"c.jalr",    "C",   "d",  MATCH_C_JALR, MASK_C_JALR, match_rd_nonzero, 0 },
{"c.j",       "C",   "Ca",  MATCH_C_J, MASK_C_J, match_opcode, 0 },
{"c.jal",     "32C", "Ca",  MATCH_C_JAL, MASK_C_JAL, match_opcode, 0 },
{"c.beqz",    "C",   "Cs,Cp",  MATCH_C_BEQZ, MASK_C_BEQZ, match_opcode, 0 },
{"c.bnez",    "C",   "Cs,Cp",  MATCH_C_BNEZ, MASK_C_BNEZ, match_opcode, 0 },
{"c.lwsp",    "C",   "d,Cm(Cc)",  MATCH_C_LWSP, MASK_C_LWSP, match_rd_nonzero, 0 },
{"c.lw",      "C",   "Ct,Ck(Cs)",  MATCH_C_LW, MASK_C_LW, match_opcode, 0 },
{"c.swsp",    "C",   "CV,CM(Cc)",  MATCH_C_SWSP, MASK_C_SWSP, match_opcode, 0 },
{"c.sw",      "C",   "Ct,Ck(Cs)",  MATCH_C_SW, MASK_C_SW, match_opcode, 0 },
{"c.nop",     "C",   "",  MATCH_C_ADDI, 0xffff, match_opcode, 0 },
{"c.mv",      "C",   "d,CV",  MATCH_C_MV, MASK_C_MV, match_c_add, 0 },
{"c.lui",     "C",   "d,Cu",  MATCH_C_LUI, MASK_C_LUI, match_c_lui, 0 },
{"c.li",      "C",   "d,Cj",  MATCH_C_LI, MASK_C_LI, match_rd_nonzero, 0 },
{"c.addi4spn","C",   "Ct,Cc,CK", MATCH_C_ADDI4SPN, MASK_C_ADDI4SPN, match_opcode, 0 },
{"c.addi16sp","C",   "Cc,CL", MATCH_C_ADDI16SP, MASK_C_ADDI16SP, match_opcode, 0 },
{"c.addi",    "C",   "d,Cj",  MATCH_C_ADDI, MASK_C_ADDI, match_rd_nonzero, 0 },
{"c.add",     "C",   "d,CV",  MATCH_C_ADD, MASK_C_ADD, match_c_add, 0 },
{"c.sub",     "C",   "Cs,Ct",  MATCH_C_SUB, MASK_C_SUB, match_opcode, 0 },
{"c.and",     "C",   "Cs,Ct",  MATCH_C_AND, MASK_C_AND, match_opcode, 0 },
{"c.or",      "C",   "Cs,Ct",  MATCH_C_OR, MASK_C_OR, match_opcode, 0 },
{"c.xor",     "C",   "Cs,Ct",  MATCH_C_XOR, MASK_C_XOR, match_opcode, 0 },
{"c.slli",    "C",   "d,C>",  MATCH_C_SLLI, MASK_C_SLLI, match_rd_nonzero, 0 },
{"c.srli",    "C",   "Cs,C>",  MATCH_C_SRLI, MASK_C_SRLI, match_opcode, 0 },
{"c.srai",    "C",   "Cs,C>",  MATCH_C_SRAI, MASK_C_SRAI, match_opcode, 0 },
{"c.andi",    "C",   "Cs,Cj",  MATCH_C_ANDI, MASK_C_ANDI, match_opcode, 0 },
{"c.addiw",   "64C", "d,Cj",  MATCH_C_ADDIW, MASK_C_ADDIW, match_rd_nonzero, 0 },
{"c.addw",    "64C", "Cs,Ct",  MATCH_C_ADDW, MASK_C_ADDW, match_opcode, 0 },
{"c.subw",    "64C", "Cs,Ct",  MATCH_C_SUBW, MASK_C_SUBW, match_opcode, 0 },
{"c.ldsp",    "64C", "d,Cn(Cc)",  MATCH_C_LDSP, MASK_C_LDSP, match_rd_nonzero, 0 },
{"c.ld",      "64C", "Ct,Cl(Cs)",  MATCH_C_LD, MASK_C_LD, match_opcode, 0 },
{"c.sdsp",    "64C", "CV,CN(Cc)",  MATCH_C_SDSP, MASK_C_SDSP, match_opcode, 0 },
{"c.sd",      "64C", "Ct,Cl(Cs)",  MATCH_C_SD, MASK_C_SD, match_opcode, 0 },
{"c.fldsp",   "C",   "D,Cn(Cc)",  MATCH_C_FLDSP, MASK_C_FLDSP, match_opcode, 0 },
{"c.fld",     "C",   "CD,Cl(Cs)",  MATCH_C_FLD, MASK_C_FLD, match_opcode, 0 },
{"c.fsdsp",   "C",   "CT,CN(Cc)",  MATCH_C_FSDSP, MASK_C_FSDSP, match_opcode, 0 },
{"c.fsd",     "C",   "CD,Cl(Cs)",  MATCH_C_FSD, MASK_C_FSD, match_opcode, 0 },
{"c.flwsp",   "32C", "D,Cm(Cc)",  MATCH_C_FLWSP, MASK_C_FLWSP, match_opcode, 0 },
{"c.flw",     "32C", "CD,Ck(Cs)",  MATCH_C_FLW, MASK_C_FLW, match_opcode, 0 },
{"c.fswsp",   "32C", "CT,CM(Cc)",  MATCH_C_FSWSP, MASK_C_FSWSP, match_opcode, 0 },
{"c.fsw",     "32C", "CD,Ck(Cs)",  MATCH_C_FSW, MASK_C_FSW, match_opcode, 0 },

/* Supervisor instructions */
{"csrr",      "I",   "d,E",  MATCH_CSRRS, MASK_CSRRS | MASK_RS1, match_opcode,  WR_xd },
{"csrwi",     "I",   "E,Z",  MATCH_CSRRWI, MASK_CSRRWI | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrw",      "I",   "E,s",  MATCH_CSRRW, MASK_CSRRW | MASK_RD, match_opcode,  RD_xs1 },
{"csrw",      "I",   "E,Z",  MATCH_CSRRWI, MASK_CSRRWI | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrsi",     "I",   "E,Z",  MATCH_CSRRSI, MASK_CSRRSI | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrs",      "I",   "E,s",  MATCH_CSRRS, MASK_CSRRS | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrs",      "I",   "E,Z",  MATCH_CSRRSI, MASK_CSRRSI | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrci",     "I",   "E,Z",  MATCH_CSRRCI, MASK_CSRRCI | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrc",      "I",   "E,s",  MATCH_CSRRC, MASK_CSRRC | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrc",      "I",   "E,Z",  MATCH_CSRRCI, MASK_CSRRCI | MASK_RD, match_opcode,  WR_xd|RD_xs1 },
{"csrrw",     "I",   "d,E,s",  MATCH_CSRRW, MASK_CSRRW, match_opcode,  WR_xd|RD_xs1 },
{"csrrw",     "I",   "d,E,Z",  MATCH_CSRRWI, MASK_CSRRWI, match_opcode,  WR_xd|RD_xs1 },
{"csrrs",     "I",   "d,E,s",  MATCH_CSRRS, MASK_CSRRS, match_opcode,  WR_xd|RD_xs1 },
{"csrrs",     "I",   "d,E,Z",  MATCH_CSRRSI, MASK_CSRRSI, match_opcode,  WR_xd|RD_xs1 },
{"csrrc",     "I",   "d,E,s",  MATCH_CSRRC, MASK_CSRRC, match_opcode,  WR_xd|RD_xs1 },
{"csrrc",     "I",   "d,E,Z",  MATCH_CSRRCI, MASK_CSRRCI, match_opcode,  WR_xd|RD_xs1 },
{"csrrwi",    "I",   "d,E,Z",  MATCH_CSRRWI, MASK_CSRRWI, match_opcode,  WR_xd|RD_xs1 },
{"csrrsi",    "I",   "d,E,Z",  MATCH_CSRRSI, MASK_CSRRSI, match_opcode,  WR_xd|RD_xs1 },
{"csrrci",    "I",   "d,E,Z",  MATCH_CSRRCI, MASK_CSRRCI, match_opcode,  WR_xd|RD_xs1 },
{"mrts",      "I",   "",     MATCH_MRTS, MASK_MRTS, match_opcode,  0 },
{"sfence.vm", "I",   "",     MATCH_SFENCE_VM, MASK_SFENCE_VM | MASK_RS1, match_opcode,  0 },
{"sfence.vm", "I",   "s",    MATCH_SFENCE_VM, MASK_SFENCE_VM, match_opcode,  RD_xs1 },
/* V1.7 Supervisor Instructions, Deprecated now, impact pulpv0 and pulpv1
{"eret",      "I",   "",     MATCH_SRET, MASK_SRET, match_opcode,  0 },
{"sret",      "I",   "",     MATCH_SRET, MASK_SRET, match_opcode,  0 },
{"wfi",       "I",   "",     MATCH_WFI, MASK_WFI, match_opcode,  0 },
*/
/* V1.9 Supervisor Instructions */
{"uret",      "I",   "",     MATCH_V19_URET, MASK_V19_USHM, match_opcode,  0 },
{"sret",      "I",   "",     MATCH_V19_SRET, MASK_V19_USHM, match_opcode,  0 },
{"hret",      "I",   "",     MATCH_V19_HRET, MASK_V19_USHM, match_opcode,  0 },
{"mret",      "I",   "",     MATCH_V19_MRET, MASK_V19_USHM, match_opcode,  0 },
{"wfi",       "I",   "",     MATCH_V19_WFI, MASK_V19_WFI, match_opcode,  0 },
{"eret",      "I",   "",     MATCH_ERET, MASK_ERET, match_opcode,  0 },


/* PULP specific opcodes */

/* Pulp slim */

{"p.mul",      		"Xpulpslim", "d,s,t",  	MATCH_MUL, 				MASK_MUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* 32x32 into 64 support */

{"p.mulh",      	"Xpulpslim", "d,s,t",  	MATCH_MULH, 				MASK_MULH, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.mulhu",     	"Xpulpslim", "d,s,t",  	MATCH_MULHU, 				MASK_MULHU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.mulhsu",    	"Xpulpslim", "d,s,t",  	MATCH_MULHSU, 				MASK_MULHSU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },

/* 32 bit div and rem */

{"p.div",      		"Xpulpslim", "d,s,t", 	MATCH_DIV, 				MASK_DIV, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.divu",     		"Xpulpslim", "d,s,t", 	MATCH_DIVU, 				MASK_DIVU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.rem",      		"Xpulpslim", "d,s,t", 	MATCH_REM, 				MASK_REM, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.remu",     		"Xpulpslim", "d,s,t", 	MATCH_REMU,				MASK_REMU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },

/* Load from event unit */

{"p.elw",		"Xpulpslim", "d,o(s)",	MATCH_LWU, 				MASK_LWU, 	match_opcode,   WR_xd|RD_xs1 },

/* Pulp slim end */

/* Pulp v0 => move to pulp v0, Sven version. Disable HW loop since hw is buggy */

/* post-increment and register-register loads */

{"p.lb",  		"Xpulpv0", "d,o(s)",  	MATCH_LB,        			MASK_LB,      	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv0", "d,o(s!)", 	MATCH_LBPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv0", "d,t(s)",  	MATCH_LBRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lb",  		"Xpulpv0", "d,t(s!)", 	MATCH_LBRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lbu", 		"Xpulpv0", "d,o(s)",  	MATCH_LBU,       			MASK_LBU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv0", "d,o(s!)", 	MATCH_LBUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv0", "d,t(s)",  	MATCH_LBURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lbu", 		"Xpulpv0", "d,t(s!)", 	MATCH_LBURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lh",  		"Xpulpv0", "d,o(s)",  	MATCH_LH,        			MASK_LH,      	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv0", "d,o(s!)", 	MATCH_LHPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv0", "d,t(s)",  	MATCH_LHRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lh",  		"Xpulpv0", "d,t(s!)", 	MATCH_LHRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lhu", 		"Xpulpv0", "d,o(s)",  	MATCH_LHU,       			MASK_LHU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv0", "d,o(s!)", 	MATCH_LHUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv0", "d,t(s)",  	MATCH_LHURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lhu", 		"Xpulpv0", "d,t(s!)", 	MATCH_LHURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lw",  		"Xpulpv0", "d,o(s)",  	MATCH_LW,        			MASK_LW,      	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv0", "d,o(s!)", 	MATCH_LWPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv0", "d,t(s)",  	MATCH_LWRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lw",  		"Xpulpv0", "d,t(s!)", 	MATCH_LWRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* post-increment and reg-reg stores */

{"p.sb",  		"Xpulpv0", "t,q(s)",  	MATCH_SB,        			MASK_SB,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv0", "t,q(s!)", 	MATCH_SBPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv0", "t,r(s)",  	MATCH_SBRR,      			MASK_SRR,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv0", "t,r(s!)", 	MATCH_SBRRPOST,  			MASK_SRRPOST, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sh",  		"Xpulpv0", "t,q(s)",  	MATCH_SH,        			MASK_SH,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv0", "t,q(s!)", 	MATCH_SHPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv0", "t,r(s)",  	MATCH_SHRR,      			MASK_SRR,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv0", "t,r(s!)", 	MATCH_SHRRPOST,  			MASK_SRRPOST, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sw",  		"Xpulpv0", "t,q(s)",  	MATCH_SW,        			MASK_SW,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv0", "t,q(s!)", 	MATCH_SWPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv0", "t,r(s)",  	MATCH_SWRR,      			MASK_SRR,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv0", "t,r(s!)", 	MATCH_SWRRPOST,  			MASK_SRRPOST, 	match_opcode,	RD_xs1|RD_xs2},

/* additional ALU operations */

{"p.avg",   		"Xpulpv0", "d,s,t", 	MATCH_AVG,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.avgu",  		"Xpulpv0", "d,s,t", 	MATCH_AVGU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.slet",  		"Xpulpv0", "d,s,t", 	MATCH_SLET,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.sletu", 		"Xpulpv0", "d,s,t", 	MATCH_SLETU, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.min",   		"Xpulpv0", "d,s,t", 	MATCH_MIN,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.minu",  		"Xpulpv0", "d,s,t", 	MATCH_MINU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.max",   		"Xpulpv0", "d,s,t", 	MATCH_MAX,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.maxu",  		"Xpulpv0", "d,s,t", 	MATCH_MAXU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.ror",   		"Xpulpv0", "d,s,t", 	MATCH_ROR,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* additional ALU operations with only a single source operand */

{"p.ff1",   		"Xpulpv0", "d,s",   	MATCH_FF1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.fl1",   		"Xpulpv0", "d,s",   	MATCH_FL1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.clb",   		"Xpulpv0", "d,s",   	MATCH_CLB,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.cnt",   		"Xpulpv0", "d,s",   	MATCH_CNT,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exths", 		"Xpulpv0", "d,s",   	MATCH_EXTHS, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exthz", 		"Xpulpv0", "d,s",   	MATCH_EXTHZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbs", 		"Xpulpv0", "d,s",   	MATCH_EXTBS,		 		MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbz", 		"Xpulpv0", "d,s",   	MATCH_EXTBZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.abs",   		"Xpulpv0", "d,s",   	MATCH_ABS,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},

// DIFT
{"p.set",   		"Xpulpv0", "d",   	  MATCH_SET,   				MASK_SET, 	  match_opcode, 	WR_xd|RD_xs1},
{"p.spsw",  		"Xpulpv0", "t,q(s)",  MATCH_SPSW,        	MASK_SPSW,    match_opcode,	  RD_xs1|RD_xs2},
{"p.spsh",      "Xpulpv0", "t,q(s)",  MATCH_SPSH,         MASK_SPSH,    match_opcode,   RD_xs1|RD_xs2},
{"p.spsb",      "Xpulpv0", "t,q(s)",  MATCH_SPSB,         MASK_SPSB,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmem",      "Xpulpv0", "t,q(s)",  MATCH_HMEM,         MASK_HMEM,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmark",     "Xpulpv0", "d,j",     MATCH_HMARK,        MASK_HMARK,   match_opcode,   WR_xd|RD_xs1},
{"p.hset",      "Xpulpv0", "d,j",     MATCH_HSET,         MASK_HSET,    match_opcode,   WR_xd|RD_xs1},


/* hardware loops */

{"lp.starti", 		"Xpulpv0", "di,b1",    	MATCH_HWLP_STARTI, 			MASK_HWLP_STARTI,match_opcode, 	0},
{"lp.endi",   		"Xpulpv0", "di,b1",    	MATCH_HWLP_ENDI,   			MASK_HWLP_ENDI,  match_opcode, 	0},
{"lp.count",  		"Xpulpv0", "di,s",     	MATCH_HWLP_COUNT,  			MASK_HWLP_COUNT, match_opcode, 	RD_xs1},
{"lp.counti", 		"Xpulpv0", "di,ji",    	MATCH_HWLP_COUNTI, 			MASK_HWLP_COUNTI,match_opcode, 	0},
{"lp.setup",  		"Xpulpv0", "di,s,b1",  	MATCH_HWLP_SETUP,  			MASK_HWLP_SETUP, match_opcode, 	RD_xs1},
{"lp.setupi", 		"Xpulpv0", "di,ji,b2", 	MATCH_HWLP_SETUPI, 			MASK_HWLP_SETUPI,match_opcode, 	0},

/* 32x32 into 32 multiplication */

{"p.mul",      		"Xpulpv0", "d,s,t",  	MATCH_MUL, 				MASK_MUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* 32x32 into 32 MAC operation */

{"p.mac",       	"Xpulpv0", "d,s,t,r", 	MATCH_MAC,     				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},

/* 16x16 into 32 MAC operations */

{"p.mac.zl.zl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZLZL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.macu",      	"Xpulpv0", "d,s,t,r", 	MATCH_MACZLZL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.zl.zh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZLZH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.zh.zl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZHZL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.machlu",    	"Xpulpv0", "d,s,t,r", 	MATCH_MACZHZL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.zh.zh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZHZH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.machhu",    	"Xpulpv0", "d,s,t,r", 	MATCH_MACZHZH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},

{"p.mac.zl.sl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZLSL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.zl.sh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZLSH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.zh.sl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZHSL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.zh.sh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACZHSH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},

{"p.mac.sl.zl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSLZL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.sl.zh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSLZH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.sh.zl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSHZL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.machlsu",   	"Xpulpv0", "d,s,t,r", 	MATCH_MACSHZL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.sh.zh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSHZH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},

{"p.mac.sl.sl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSLSL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.macs",      	"Xpulpv0", "d,s,t,r", 	MATCH_MACSLSL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.sl.sh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSLSH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.sh.sl", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSHSL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.machls",    	"Xpulpv0", "d,s,t,r", 	MATCH_MACSHSL, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.mac.sh.sh", 	"Xpulpv0", "d,s,t,r", 	MATCH_MACSHSH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},
{"p.machhs",    	"Xpulpv0", "d,s,t,r", 	MATCH_MACSHSH, 				MASK_MAC, 	match_opcode, 	WR_xd|RD_xs1|RD_xs2|RD_xs3},

/* Pulp v1 */
/* post-increment and register-register loads */

{"p.lb",  		"Xpulpv1", "d,o(s)",  	MATCH_LB,        			MASK_LB,      	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv1", "d,o(s!)", 	MATCH_LBPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv1", "d,t(s)",  	MATCH_LBRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lb",  		"Xpulpv1", "d,t(s!)", 	MATCH_LBRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lbu", 		"Xpulpv1", "d,o(s)",  	MATCH_LBU,       			MASK_LBU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv1", "d,o(s!)", 	MATCH_LBUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv1", "d,t(s)",  	MATCH_LBURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lbu", 		"Xpulpv1", "d,t(s!)", 	MATCH_LBURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lh",  		"Xpulpv1", "d,o(s)",  	MATCH_LH,        			MASK_LH,      	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv1", "d,o(s!)", 	MATCH_LHPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv1", "d,t(s)",  	MATCH_LHRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lh",  		"Xpulpv1", "d,t(s!)", 	MATCH_LHRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lhu", 		"Xpulpv1", "d,o(s)",  	MATCH_LHU,       			MASK_LHU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv1", "d,o(s!)", 	MATCH_LHUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv1", "d,t(s)",  	MATCH_LHURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lhu", 		"Xpulpv1", "d,t(s!)", 	MATCH_LHURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lw",  		"Xpulpv1", "d,o(s)",  	MATCH_LW,        			MASK_LW,      	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv1", "d,o(s!)", 	MATCH_LWPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv1", "d,t(s)",  	MATCH_LWRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lw",  		"Xpulpv1", "d,t(s!)", 	MATCH_LWRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* post-increment and reg-reg stores */

{"p.sb",  		"Xpulpv1", "t,q(s)",  	MATCH_SB,        			MASK_SB,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv1", "t,q(s!)", 	MATCH_SBPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv1", "t,r(s)",  	MATCH_SBRR,      			MASK_SRR,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv1", "t,r(s!)", 	MATCH_SBRRPOST,  			MASK_SRRPOST, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sh",  		"Xpulpv1", "t,q(s)",  	MATCH_SH,        			MASK_SH,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv1", "t,q(s!)", 	MATCH_SHPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv1", "t,r(s)",  	MATCH_SHRR,      			MASK_SRR,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv1", "t,r(s!)", 	MATCH_SHRRPOST,  			MASK_SRRPOST, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sw",  		"Xpulpv1", "t,q(s)",  	MATCH_SW,        			MASK_SW,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv1", "t,q(s!)", 	MATCH_SWPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv1", "t,r(s)",  	MATCH_SWRR,      			MASK_SRR,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv1", "t,r(s!)", 	MATCH_SWRRPOST,  			MASK_SRRPOST, 	match_opcode,	RD_xs1|RD_xs2},

/* additional ALU operations */

{"p.avg",   		"Xpulpv1", "d,s,t", 	MATCH_AVG,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.avgu",  		"Xpulpv1", "d,s,t", 	MATCH_AVGU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.slet",  		"Xpulpv1", "d,s,t", 	MATCH_SLET,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.sletu", 		"Xpulpv1", "d,s,t", 	MATCH_SLETU, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.min",   		"Xpulpv1", "d,s,t", 	MATCH_MIN,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.minu",  		"Xpulpv1", "d,s,t", 	MATCH_MINU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.max",   		"Xpulpv1", "d,s,t", 	MATCH_MAX,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.maxu",  		"Xpulpv1", "d,s,t", 	MATCH_MAXU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.ror",   		"Xpulpv1", "d,s,t", 	MATCH_ROR,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* additional ALU operations with only a single source operand */

{"p.ff1",   		"Xpulpv1", "d,s",   	MATCH_FF1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.fl1",   		"Xpulpv1", "d,s",   	MATCH_FL1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.clb",   		"Xpulpv1", "d,s",   	MATCH_CLB,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.cnt",   		"Xpulpv1", "d,s",   	MATCH_CNT,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exths", 		"Xpulpv1", "d,s",   	MATCH_EXTHS, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exthz", 		"Xpulpv1", "d,s",   	MATCH_EXTHZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbs", 		"Xpulpv1", "d,s",   	MATCH_EXTBS,		 		MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbz", 		"Xpulpv1", "d,s",   	MATCH_EXTBZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.abs",   		"Xpulpv1", "d,s",   	MATCH_ABS,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},

// DIFT
{"p.set",   		"Xpulpv1", "d",   	  MATCH_SET,   				MASK_SET, 	  match_opcode, 	WR_xd},
{"p.spsw",  		"Xpulpv1", "t,q(s)",  MATCH_SPSW,        	MASK_SPSW,    match_opcode,	  RD_xs1|RD_xs2},
{"p.spsh",      "Xpulpv1", "t,q(s)",  MATCH_SPSH,         MASK_SPSH,    match_opcode,   RD_xs1|RD_xs2},
{"p.spsb",      "Xpulpv1", "t,q(s)",  MATCH_SPSB,         MASK_SPSB,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmem",      "Xpulpv1", "t,q(s)",  MATCH_HMEM,         MASK_HMEM,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmark",     "Xpulpv1", "d,j",     MATCH_HMARK,        MASK_HMARK,   match_opcode,   WR_xd|RD_xs1},
{"p.hset",      "Xpulpv1", "d,j",     MATCH_HSET,         MASK_HSET,    match_opcode,   WR_xd|RD_xs1},

/* hardware loops */

{"lp.starti", 		"Xpulpv1", "di,b1",    	MATCH_HWLP_STARTI, 			MASK_HWLP_STARTI,match_opcode, 	0},
{"lp.endi",   		"Xpulpv1", "di,b1",    	MATCH_HWLP_ENDI,   			MASK_HWLP_ENDI,  match_opcode, 	0},
{"lp.count",  		"Xpulpv1", "di,s",     	MATCH_HWLP_COUNT,  			MASK_HWLP_COUNT, match_opcode, 	RD_xs1},
{"lp.counti", 		"Xpulpv1", "di,ji",    	MATCH_HWLP_COUNTI, 			MASK_HWLP_COUNTI,match_opcode, 	0},
{"lp.setup",  		"Xpulpv1", "di,s,b1",  	MATCH_HWLP_SETUP,  			MASK_HWLP_SETUP, match_opcode, 	RD_xs1},
{"lp.setupi", 		"Xpulpv1", "di,ji,b2", 	MATCH_HWLP_SETUPI, 			MASK_HWLP_SETUPI,match_opcode, 	0},

/* 32x32 into 32 multiplication */

{"p.mul",      		"Xpulpv1", "d,s,t",  	MATCH_MUL, 				MASK_MUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* 32x32 into 32 MAC operation */

{"p.mac",		"Xpulpv1", "d,s,t",  	MATCH_MULH, 				MASK_MULH, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* 16x16 into 32 MAC operations */

{"p.macs",		"Xpulpv1", "d,s,t",  	MATCH_MACS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhs",		"Xpulpv1", "d,s,t",  	MATCH_MACHHS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.macu",		"Xpulpv1", "d,s,t",  	MATCH_MACU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhu",		"Xpulpv1", "d,s,t",  	MATCH_MACHHU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* Pulp v2 */
/* post-increment and register-register loads */

{"p.lb",  		"Xpulpv2", "d,o(s)",  	MATCH_LB,        			MASK_LB,      	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv2", "d,o(s!)", 	MATCH_LBPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv2", "d,t(s)",  	MATCH_LBRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lb",  		"Xpulpv2", "d,t(s!)", 	MATCH_LBRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lbu", 		"Xpulpv2", "d,o(s)",  	MATCH_LBU,       			MASK_LBU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv2", "d,o(s!)", 	MATCH_LBUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv2", "d,t(s)",  	MATCH_LBURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lbu", 		"Xpulpv2", "d,t(s!)", 	MATCH_LBURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lh",  		"Xpulpv2", "d,o(s)",  	MATCH_LH,        			MASK_LH,      	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv2", "d,o(s!)", 	MATCH_LHPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv2", "d,t(s)",  	MATCH_LHRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lh",  		"Xpulpv2", "d,t(s!)", 	MATCH_LHRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lhu", 		"Xpulpv2", "d,o(s)",  	MATCH_LHU,       			MASK_LHU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv2", "d,o(s!)", 	MATCH_LHUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv2", "d,t(s)",  	MATCH_LHURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lhu", 		"Xpulpv2", "d,t(s!)", 	MATCH_LHURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lw",  		"Xpulpv2", "d,o(s)",  	MATCH_LW,        			MASK_LW,      	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv2", "d,o(s!)", 	MATCH_LWPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv2", "d,t(s)",  	MATCH_LWRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lw",  		"Xpulpv2", "d,t(s!)", 	MATCH_LWRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* post-increment and reg-reg stores */

{"p.sb",  		"Xpulpv2", "t,q(s)",  	MATCH_SB,        			MASK_SB,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv2", "t,q(s!)", 	MATCH_SBPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv2", "t,d(s)",  	MATCH_SBRR,      			MASK_PALU,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv2", "t,d(s!)", 	MATCH_SBRRPOST,  			MASK_PALU, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sh",  		"Xpulpv2", "t,q(s)",  	MATCH_SH,        			MASK_SH,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv2", "t,q(s!)", 	MATCH_SHPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv2", "t,d(s)",  	MATCH_SHRR,      			MASK_PALU,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv2", "t,d(s!)", 	MATCH_SHRRPOST,  			MASK_PALU, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sw",  		"Xpulpv2", "t,q(s)",  	MATCH_SW,        			MASK_SW,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv2", "t,q(s!)", 	MATCH_SWPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv2", "t,d(s)",  	MATCH_SWRR,      			MASK_PALU,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv2", "t,d(s!)", 	MATCH_SWRRPOST,  			MASK_PALU, 	match_opcode,	RD_xs1|RD_xs2},

/* additional ALU operations */

{"p.abs",   		"Xpulpv2", "d,s", 	MATCH_AVG,   				MASK_PALUS,  	match_opcode,	WR_xd|RD_xs1},

// DIFT
{"p.set",   		"Xpulpv2", "d",   	  MATCH_SET,   				MASK_SET, 	  match_opcode, 	WR_xd},
{"p.spsw",  		"Xpulpv2", "t,q(s)",  MATCH_SPSW,        	MASK_SPSW,    match_opcode,	  RD_xs1|RD_xs2},
{"p.spsh",      "Xpulpv2", "t,q(s)",  MATCH_SPSH,         MASK_SPSH,    match_opcode,   RD_xs1|RD_xs2},
{"p.spsb",      "Xpulpv2", "t,q(s)",  MATCH_SPSB,         MASK_SPSB,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmem",      "Xpulpv2", "t,q(s)",  MATCH_HMEM,         MASK_HMEM,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmark",     "Xpulpv2", "d,j",     MATCH_HMARK,        MASK_HMARK,   match_opcode,   WR_xd|RD_xs1},
{"p.hset",      "Xpulpv2", "d,j",     MATCH_HSET,         MASK_HSET,    match_opcode,   WR_xd|RD_xs1},

// {"p.avgu",  		"Xpulpv2", "d,s,t", 	MATCH_AVGU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.slet",  		"Xpulpv2", "d,s,t", 	MATCH_SLET,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.sletu", 		"Xpulpv2", "d,s,t", 	MATCH_SLETU, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.min",   		"Xpulpv2", "d,s,t", 	MATCH_MIN,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.minu",  		"Xpulpv2", "d,s,t", 	MATCH_MINU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.max",   		"Xpulpv2", "d,s,t", 	MATCH_MAX,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.maxu",  		"Xpulpv2", "d,s,t", 	MATCH_MAXU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.ror",   		"Xpulpv2", "d,s,t", 	MATCH_ROR,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* additional ALU operations with only a single source operand */

{"p.ff1",   		"Xpulpv2", "d,s",   	MATCH_FF1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.fl1",   		"Xpulpv2", "d,s",   	MATCH_FL1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.clb",   		"Xpulpv2", "d,s",   	MATCH_CLB,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.cnt",   		"Xpulpv2", "d,s",   	MATCH_CNT,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exths", 		"Xpulpv2", "d,s",   	MATCH_EXTHS, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exthz", 		"Xpulpv2", "d,s",   	MATCH_EXTHZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbs", 		"Xpulpv2", "d,s",   	MATCH_EXTBS,		 		MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbz", 		"Xpulpv2", "d,s",   	MATCH_EXTBZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},

/* clip and bit manipulation */


{"p.clip",   		"Xpulpv2", "d,s,bi", 	MATCH_CLIP,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipi",   		"Xpulpv2", "d,s,bi", 	MATCH_CLIP,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipr",   		"Xpulpv2", "d,s,t", 	MATCH_CLIPR,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipu",   		"Xpulpv2", "d,s,bi", 	MATCH_CLIPU,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipui",   		"Xpulpv2", "d,s,bi", 	MATCH_CLIPU,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipur",   		"Xpulpv2", "d,s,t", 	MATCH_CLIPUR,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},

{"p.extract",  		"Xpulpv2", "d,s,b5,bi",	MATCH_EXTRACT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extracti", 		"Xpulpv2", "d,s,b5,bi",	MATCH_EXTRACT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractr", 		"Xpulpv2", "d,s,t",	MATCH_EXTRACTR,				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractu", 		"Xpulpv2", "d,s,b5,bi",	MATCH_EXTRACTU, 			MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractui", 	"Xpulpv2", "d,s,b5,bi",	MATCH_EXTRACTU, 			MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractur", 	"Xpulpv2", "d,s,t",	MATCH_EXTRACTUR, 			MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.insert",  		"Xpulpv2", "d,s,b5,bi",	MATCH_INSERT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.inserti",  		"Xpulpv2", "d,s,b5,bi",	MATCH_INSERT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.insertr",  		"Xpulpv2", "d,s,t",	MATCH_INSERTR, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.bset",  		"Xpulpv2", "d,s,b5,bi",	MATCH_BSET, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bseti",  		"Xpulpv2", "d,s,b5,bi",	MATCH_BSET, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bsetr",  		"Xpulpv2", "d,s,t",	MATCH_BSETR, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.bclr",  		"Xpulpv2", "d,s,b5,bi",	MATCH_BCLR, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bclri",  		"Xpulpv2", "d,s,b5,bi",	MATCH_BCLR, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bclrr",  		"Xpulpv2", "d,s,t",	MATCH_BCLRR, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},

/* hardware loops */

{"lp.starti", 		"Xpulpv2", "di,b1",    	MATCH_HWLP_STARTI, 			MASK_HWLP_STARTI,match_opcode, 	0},
{"lp.endi",   		"Xpulpv2", "di,b1",    	MATCH_HWLP_ENDI,   			MASK_HWLP_ENDI,  match_opcode, 	0},
{"lp.count",  		"Xpulpv2", "di,s",     	MATCH_HWLP_COUNT,  			MASK_HWLP_COUNT, match_opcode, 	RD_xs1},
{"lp.counti", 		"Xpulpv2", "di,ji",    	MATCH_HWLP_COUNTI, 			MASK_HWLP_COUNTI,match_opcode, 	0},
{"lp.setup",  		"Xpulpv2", "di,s,b1",  	MATCH_HWLP_SETUP,  			MASK_HWLP_SETUP, match_opcode, 	RD_xs1},
{"lp.setupi", 		"Xpulpv2", "di,ji,b2", 	MATCH_HWLP_SETUPI, 			MASK_HWLP_SETUPI,match_opcode, 	0},

/* 32x32 into 32 multiplication */

{"p.mul",      		"Xpulpv2", "d,s,t",  	MATCH_MUL, 				MASK_MUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* 32x32 into 64 support */

{"p.mulh",      	"Xpulpv2", "d,s,t",  	MATCH_MULH, 				MASK_MULH, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.mulhu",     	"Xpulpv2", "d,s,t",  	MATCH_MULHU, 				MASK_MULHU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.mulhsu",    	"Xpulpv2", "d,s,t",  	MATCH_MULHSU, 				MASK_MULHSU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },

/* 32 bit div and rem */

{"p.div",      		"Xpulpv2",   "d,s,t",  	MATCH_DIV, 				MASK_DIV, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.divu",     		"Xpulpv2",   "d,s,t",  	MATCH_DIVU, 				MASK_DIVU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.rem",      		"Xpulpv2",   "d,s,t",  	MATCH_REM, 				MASK_REM, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.remu",     		"Xpulpv2",   "d,s,t",  	MATCH_REMU,				MASK_REMU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },

/* 32x32 into 32 Mac/Msu */

{"p.mac",		"Xpulpv2", "d,s,t",  	MATCH_MAC32, 				MASK_MACMSU32, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.msu",		"Xpulpv2", "d,s,t",  	MATCH_MSU32, 				MASK_MACMSU32, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/*  16x16 into 32 Mult/Mac with optional norm and rounding */

{"p.muls",		"Xpulpv2", "d,s,t",  	MATCH_MULS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhs",		"Xpulpv2", "d,s,t",  	MATCH_MULHHS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulu",		"Xpulpv2", "d,s,t",  	MATCH_MULU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhu",		"Xpulpv2", "d,s,t",  	MATCH_MULHHU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.macs",		"Xpulpv2", "d,s,t",  	MATCH_MACS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhs",		"Xpulpv2", "d,s,t",  	MATCH_MACHHS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.macu",		"Xpulpv2", "d,s,t",  	MATCH_MACU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhu",		"Xpulpv2", "d,s,t",  	MATCH_MACHHU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.mulsn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhsn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULHHSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulsrn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULSRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhsrn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULHHSRN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.mulun",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhun",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULHHUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulurn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULURN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhurn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MULHHURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.macsn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhsn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACHHSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.macsrn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACSRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhsrn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACHHSRN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.macun",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhun",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACHHUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.macurn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACURN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhurn",		"Xpulpv2", "d,s,t,b5", 	MATCH_MACHHURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/*  Add/Sub with norm and rounding */

{"p.addn",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addni",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addnr",		"Xpulpv2", "d,s,t", 	MATCH_ADDNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addun",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.adduni",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addunr",		"Xpulpv2", "d,s,t", 	MATCH_ADDUNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addrn",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addrni",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addrnr",		"Xpulpv2", "d,s,t", 	MATCH_ADDRNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addurn",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addurni",		"Xpulpv2", "d,s,t,b5", 	MATCH_ADDURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addurnr",		"Xpulpv2", "d,s,t", 	MATCH_ADDURNR,				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.subn",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subni",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subnr",		"Xpulpv2", "d,s,t", 	MATCH_SUBNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subun",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subuni",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subunr",		"Xpulpv2", "d,s,t", 	MATCH_SUBUNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subrn",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subrni",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subrnr",		"Xpulpv2", "d,s,t", 	MATCH_SUBRNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.suburn",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.suburni",		"Xpulpv2", "d,s,t,b5", 	MATCH_SUBURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.suburnr",		"Xpulpv2", "d,s,t", 	MATCH_SUBURNR,				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* Vector Operations */

{"pv.add.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_ADD|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.add.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_ADD|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sub.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_SUB|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sub.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_SUB|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.avg.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_AVG|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.avg.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_AVG|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.avgu.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_AVGU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.avgu.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_AVGU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.min.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_MIN|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.min.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_MIN|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.minu.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_MINU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.minu.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_MINU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.max.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_MAX|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.max.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_MAX|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.maxu.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_MAXU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.maxu.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_MAXU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.srl.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sci.h",	"Xpulpv2", "d,s,bU",	MATCH_V_OP_SRL|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.srl.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sci.b",	"Xpulpv2", "d,s,bU",	MATCH_V_OP_SRL|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sra.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sci.h",	"Xpulpv2", "d,s,bU",	MATCH_V_OP_SRA|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sra.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sci.b",	"Xpulpv2", "d,s,bU",	MATCH_V_OP_SRA|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sll.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sci.h",	"Xpulpv2", "d,s,bU",	MATCH_V_OP_SLL|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sll.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sci.b",	"Xpulpv2", "d,s,bU",	MATCH_V_OP_SLL|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.or.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sci.h",		"Xpulpv2", "d,s,bs",	MATCH_V_OP_OR|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.or.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sci.b",		"Xpulpv2", "d,s,bs",	MATCH_V_OP_OR|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.xor.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_XOR|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.xor.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_XOR|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.and.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sc.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_AND|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.and.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sc.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_AND|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.abs.h",		"Xpulpv2", "d,s",	MATCH_V_OP_ABS|MATCH_V_OP_H_VV,		MASK_V_OP2,	match_opcode,	WR_xd|RD_xs1},
{"pv.abs.b",		"Xpulpv2", "d,s",	MATCH_V_OP_ABS|MATCH_V_OP_B_VV,		MASK_V_OP2,	match_opcode,	WR_xd|RD_xs1},

{"pv.extract.h",	"Xpulpv2", "d,s,bf",	MATCH_V_OP_EXTRACT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.extract.b",	"Xpulpv2", "d,s,bF",	MATCH_V_OP_EXTRACT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.extractu.h",	"Xpulpv2", "d,s,bf",	MATCH_V_OP_DOTSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.extractu.b",	"Xpulpv2", "d,s,bF",	MATCH_V_OP_DOTSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.insert.h",		"Xpulpv2", "d,s,bf",	MATCH_V_OP_SDOTUP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.insert.b",		"Xpulpv2", "d,s,bF",	MATCH_V_OP_SDOTUP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.dotsp.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_DOTUP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.dotsp.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_DOTUP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.dotup.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.dotup.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.dotusp.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_INSERT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.dotusp.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_INSERT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sdotsp.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sdotsp.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sdotup.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_DOTUSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sdotup.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_DOTUSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sdotusp.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_SDOTSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sdotusp.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_SDOTSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.shuffle.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shuffle.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shuffle.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei0.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei1.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_SHUFFLEI1|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei2.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_SHUFFLEI2|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei3.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_SHUFFLEI3|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.shuffle2.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SHUFFLE2|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shuffle2.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_SHUFFLE2|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.pack.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_PACK|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.packhi.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_PACKHI|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.packlo.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_PACKLO|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.cmpeq.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPEQ|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpeq.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPEQ|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpne.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPNE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpne.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPNE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpgt.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPGT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpgt.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPGT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpge.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPGE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpge.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPGE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmplt.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPLT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmplt.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPLT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmple.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sci.h",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPLE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmple.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sci.b",	"Xpulpv2", "d,s,bs",	MATCH_V_OP_CMPLE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpgtu.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPGTU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpgtu.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPGTU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpgeu.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPGEU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpgeu.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPGEU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpltu.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPLTU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpltu.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPLTU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpleu.h",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sc.h",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sci.h",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPLEU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpleu.b",		"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sc.b",	"Xpulpv2", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sci.b",	"Xpulpv2", "d,s,bu",	MATCH_V_OP_CMPLEU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
/*
{"pv.ball",       	"Xpulpv2",  "s,p",  	MATCH_BEQM1, 			MASK_BEQM1|MASK_RS2, 	match_opcode,   RD_xs1 },
{"pv.bnall",       	"Xpulpv2",  "s,p",  	MATCH_BNEM1, 			MASK_BNEM1|MASK_RS2, 	match_opcode,   RD_xs1 },
*/
{"p.beqimm",       	"Xpulpv2",  "s,bI,p",  	MATCH_BEQM1, 				MASK_BEQM1, 	match_opcode,   RD_xs1 },
{"p.bneimm",       	"Xpulpv2",  "s,bI,p",  	MATCH_BNEM1, 				MASK_BNEM1, 	match_opcode,   RD_xs1 },

/* Load from event unit */

{"p.elw",		"Xpulpv2", "d,o(s)",	MATCH_LWU, 				MASK_LWU, 	match_opcode,   WR_xd|RD_xs1 },


/* Pulp v3 */
/* post-increment and register-register loads */

{"p.lb",  		"Xpulpv3", "d,o(s)",  	MATCH_LB,        			MASK_LB,      	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv3", "d,o(s!)", 	MATCH_LBPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lb",  		"Xpulpv3", "d,t(s)",  	MATCH_LBRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lb",  		"Xpulpv3", "d,t(s!)", 	MATCH_LBRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lbu", 		"Xpulpv3", "d,o(s)",  	MATCH_LBU,       			MASK_LBU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv3", "d,o(s!)", 	MATCH_LBUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lbu", 		"Xpulpv3", "d,t(s)",  	MATCH_LBURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lbu", 		"Xpulpv3", "d,t(s!)", 	MATCH_LBURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lh",  		"Xpulpv3", "d,o(s)",  	MATCH_LH,        			MASK_LH,      	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv3", "d,o(s!)", 	MATCH_LHPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lh",  		"Xpulpv3", "d,t(s)",  	MATCH_LHRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lh",  		"Xpulpv3", "d,t(s!)", 	MATCH_LHRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lhu", 		"Xpulpv3", "d,o(s)",  	MATCH_LHU,       			MASK_LHU,     	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv3", "d,o(s!)", 	MATCH_LHUPOST,   			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lhu", 		"Xpulpv3", "d,t(s)",  	MATCH_LHURR,     			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lhu", 		"Xpulpv3", "d,t(s!)", 	MATCH_LHURRPOST, 			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"p.lw",  		"Xpulpv3", "d,o(s)",  	MATCH_LW,        			MASK_LW,      	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv3", "d,o(s!)", 	MATCH_LWPOST,    			MASK_LPOST,   	match_opcode,	WR_xd|RD_xs1},
{"p.lw",  		"Xpulpv3", "d,t(s)",  	MATCH_LWRR,      			MASK_LRR,     	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.lw",  		"Xpulpv3", "d,t(s!)", 	MATCH_LWRRPOST,  			MASK_LRRPOST, 	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* post-increment and reg-reg stores */

{"p.sb",  		"Xpulpv3", "t,q(s)",  	MATCH_SB,        			MASK_SB,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv3", "t,q(s!)", 	MATCH_SBPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv3", "t,d(s)",  	MATCH_SBRR,      			MASK_PALU,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sb",  		"Xpulpv3", "t,d(s!)", 	MATCH_SBRRPOST,  			MASK_PALU, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sh",  		"Xpulpv3", "t,q(s)",  	MATCH_SH,        			MASK_SH,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv3", "t,q(s!)", 	MATCH_SHPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv3", "t,d(s)",  	MATCH_SHRR,      			MASK_PALU,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sh",  		"Xpulpv3", "t,d(s!)", 	MATCH_SHRRPOST,  			MASK_PALU, 	match_opcode,	RD_xs1|RD_xs2},

{"p.sw",  		"Xpulpv3", "t,q(s)",  	MATCH_SW,        			MASK_SW,      	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv3", "t,q(s!)", 	MATCH_SWPOST,    			MASK_SPOST,   	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv3", "t,d(s)",  	MATCH_SWRR,      			MASK_PALU,     	match_opcode,	RD_xs1|RD_xs2},
{"p.sw",  		"Xpulpv3", "t,d(s!)", 	MATCH_SWRRPOST,  			MASK_PALU, 	match_opcode,	RD_xs1|RD_xs2},

/* additional ALU operations */

{"p.abs",   		"Xpulpv3", "d,s", 	MATCH_AVG,   				MASK_PALUS,  	match_opcode,	WR_xd|RD_xs1},

// DIFT
{"p.set",   		"Xpulpv3", "d",   	  MATCH_SET,   				MASK_SET, 	  match_opcode, 	WR_xd},
{"p.spsw",  		"Xpulpv3", "t,q(s)",  MATCH_SPSW,        	MASK_SPSW,    match_opcode,	  RD_xs1|RD_xs2},
{"p.spsh",      "Xpulpv3", "t,q(s)",  MATCH_SPSH,         MASK_SPSH,    match_opcode,   RD_xs1|RD_xs2},
{"p.spsb",      "Xpulpv3", "t,q(s)",  MATCH_SPSB,         MASK_SPSB,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmem",      "Xpulpv3", "t,q(s)",  MATCH_HMEM,         MASK_HMEM,    match_opcode,   RD_xs1|RD_xs2},
{"p.hmark",     "Xpulpv3", "d,j",     MATCH_HMARK,        MASK_HMARK,   match_opcode,   WR_xd|RD_xs1},
{"p.hset",      "Xpulpv3", "d,j",     MATCH_HSET,         MASK_HSET,    match_opcode,   WR_xd|RD_xs1},

// {"p.avgu",  		"Xpulpv3", "d,s,t", 	MATCH_AVGU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.slet",  		"Xpulpv3", "d,s,t", 	MATCH_SLET,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.sletu", 		"Xpulpv3", "d,s,t", 	MATCH_SLETU, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.min",   		"Xpulpv3", "d,s,t", 	MATCH_MIN,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.minu",  		"Xpulpv3", "d,s,t", 	MATCH_MINU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.max",   		"Xpulpv3", "d,s,t", 	MATCH_MAX,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.maxu",  		"Xpulpv3", "d,s,t", 	MATCH_MAXU,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"p.ror",   		"Xpulpv3", "d,s,t", 	MATCH_ROR,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1|RD_xs2},

/* additional ALU operations with only a single source operand */

{"p.ff1",   		"Xpulpv3", "d,s",   	MATCH_FF1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.fl1",   		"Xpulpv3", "d,s",   	MATCH_FL1,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.clb",   		"Xpulpv3", "d,s",   	MATCH_CLB,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.cnt",   		"Xpulpv3", "d,s",   	MATCH_CNT,   				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exths", 		"Xpulpv3", "d,s",   	MATCH_EXTHS, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.exthz", 		"Xpulpv3", "d,s",   	MATCH_EXTHZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbs", 		"Xpulpv3", "d,s",   	MATCH_EXTBS,		 		MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},
{"p.extbz", 		"Xpulpv3", "d,s",   	MATCH_EXTBZ, 				MASK_PALUS, 	match_opcode, 	WR_xd|RD_xs1},

/* clip and bit manipulation */

{"p.clip",   		"Xpulpv3", "d,s,bi", 	MATCH_CLIP,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipi",   		"Xpulpv3", "d,s,bi", 	MATCH_CLIP,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipr",   		"Xpulpv3", "d,s,t", 	MATCH_CLIPR,   				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipu",   		"Xpulpv3", "d,s,bi", 	MATCH_CLIPU,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipui",   		"Xpulpv3", "d,s,bi", 	MATCH_CLIPU,   				MASK_PALU1,  	match_opcode,	WR_xd|RD_xs1},
{"p.clipur",   		"Xpulpv3", "d,s,t", 	MATCH_CLIPUR,  				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},

{"p.extract",  		"Xpulpv3", "d,s,b5,bi",	MATCH_EXTRACT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extracti", 		"Xpulpv3", "d,s,b5,bi",	MATCH_EXTRACT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractr", 		"Xpulpv3", "d,s,t",	MATCH_EXTRACTR,				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractu", 		"Xpulpv3", "d,s,b5,bi",	MATCH_EXTRACTU, 			MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractui", 	"Xpulpv3", "d,s,b5,bi",	MATCH_EXTRACTU, 			MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.extractur", 	"Xpulpv3", "d,s,t",	MATCH_EXTRACTUR, 			MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.insert",  		"Xpulpv3", "d,s,b5,bi",	MATCH_INSERT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.inserti",  		"Xpulpv3", "d,s,b5,bi",	MATCH_INSERT, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.insertr",  		"Xpulpv3", "d,s,t",	MATCH_INSERTR, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.bset",  		"Xpulpv3", "d,s,b5,bi",	MATCH_BSET, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bseti",  		"Xpulpv3", "d,s,b5,bi",	MATCH_BSET, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bsetr",  		"Xpulpv3", "d,s,t",	MATCH_BSETR, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},
{"p.bclr",  		"Xpulpv3", "d,s,b5,bi",	MATCH_BCLR, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bclri",  		"Xpulpv3", "d,s,b5,bi",	MATCH_BCLR, 				MASK_PALU2,  	match_opcode,	WR_xd|RD_xs1},
{"p.bclrr",  		"Xpulpv3", "d,s,t",	MATCH_BCLRR, 				MASK_PALU,  	match_opcode,	WR_xd|RD_xs1},

/* hardware loops */

{"lp.starti", 		"Xpulpv3", "di,b1",    	MATCH_HWLP_STARTI, 			MASK_HWLP_STARTI,match_opcode, 	0},
{"lp.endi",   		"Xpulpv3", "di,b1",    	MATCH_HWLP_ENDI,   			MASK_HWLP_ENDI,  match_opcode, 	0},
{"lp.count",  		"Xpulpv3", "di,s",     	MATCH_HWLP_COUNT,  			MASK_HWLP_COUNT, match_opcode, 	RD_xs1},
{"lp.counti", 		"Xpulpv3", "di,ji",    	MATCH_HWLP_COUNTI, 			MASK_HWLP_COUNTI,match_opcode, 	0},
{"lp.setup",  		"Xpulpv3", "di,s,b1",  	MATCH_HWLP_SETUP,  			MASK_HWLP_SETUP, match_opcode, 	RD_xs1},
{"lp.setupi", 		"Xpulpv3", "di,ji,b2", 	MATCH_HWLP_SETUPI, 			MASK_HWLP_SETUPI,match_opcode, 	0},

/* 32x32 into 32 multiplication */

{"p.mul",      		"Xpulpv3", "d,s,t",  	MATCH_MUL, 				MASK_MUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* 32x32 into 64 support */

{"p.mulh",      	"Xpulpv3", "d,s,t",  	MATCH_MULH, 				MASK_MULH, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.mulhu",     	"Xpulpv3", "d,s,t",  	MATCH_MULHU, 				MASK_MULHU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.mulhsu",    	"Xpulpv3", "d,s,t",  	MATCH_MULHSU, 				MASK_MULHSU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },

/* 32 bit div and rem */

{"p.div",      		"Xpulpv3",   "d,s,t",  	MATCH_DIV, 				MASK_DIV, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.divu",     		"Xpulpv3",   "d,s,t",  	MATCH_DIVU, 				MASK_DIVU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.rem",      		"Xpulpv3",   "d,s,t",  	MATCH_REM, 				MASK_REM, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },
{"p.remu",     		"Xpulpv3",   "d,s,t",  	MATCH_REMU,				MASK_REMU, 	match_opcode,  WR_xd|RD_xs1|RD_xs2 },

/* 32x32 into 32 Mac/Msu */

{"p.mac",		"Xpulpv3", "d,s,t",  	MATCH_MAC32, 				MASK_MACMSU32, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.msu",		"Xpulpv3", "d,s,t",  	MATCH_MSU32, 				MASK_MACMSU32, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/*  16x16 into 32 Mult/Mac with optional norm and rounding */

{"p.muls",		"Xpulpv3", "d,s,t",  	MATCH_MULS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhs",		"Xpulpv3", "d,s,t",  	MATCH_MULHHS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulu",		"Xpulpv3", "d,s,t",  	MATCH_MULU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhu",		"Xpulpv3", "d,s,t",  	MATCH_MULHHU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.macs",		"Xpulpv3", "d,s,t",  	MATCH_MACS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhs",		"Xpulpv3", "d,s,t",  	MATCH_MACHHS, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.macu",		"Xpulpv3", "d,s,t",  	MATCH_MACU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhu",		"Xpulpv3", "d,s,t",  	MATCH_MACHHU, 				MASK_MACMUL, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.mulsn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhsn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULHHSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulsrn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULSRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhsrn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULHHSRN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.mulun",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhun",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULHHUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulurn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULURN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.mulhhurn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MULHHURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.macsn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhsn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACHHSN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.macsrn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACSRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhsrn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACHHSRN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.macun",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhun",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACHHUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.macurn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACURN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.machhurn",		"Xpulpv3", "d,s,t,b5", 	MATCH_MACHHURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/*  Add/Sub with norm and rounding */

{"p.addn",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addni",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addnr",		"Xpulpv3", "d,s,t", 	MATCH_ADDNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addun",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.adduni",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addunr",		"Xpulpv3", "d,s,t", 	MATCH_ADDUNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addrn",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addrni",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addrnr",		"Xpulpv3", "d,s,t", 	MATCH_ADDRNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addurn",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addurni",		"Xpulpv3", "d,s,t,b5", 	MATCH_ADDURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.addurnr",		"Xpulpv3", "d,s,t", 	MATCH_ADDURNR,				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

{"p.subn",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subni",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subnr",		"Xpulpv3", "d,s,t", 	MATCH_SUBNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subun",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subuni",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBUN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subunr",		"Xpulpv3", "d,s,t", 	MATCH_SUBUNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subrn",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subrni",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBRN, 				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.subrnr",		"Xpulpv3", "d,s,t", 	MATCH_SUBRNR, 				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.suburn",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.suburni",		"Xpulpv3", "d,s,t,b5", 	MATCH_SUBURN,				MASK_MACMULNR, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },
{"p.suburnr",		"Xpulpv3", "d,s,t", 	MATCH_SUBURNR,				MASK_PALU, 	match_opcode,  	WR_xd|RD_xs1|RD_xs2 },

/* Vector Operations */

{"pv.add.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_ADD|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.add.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_ADD|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.add.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_ADD|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sub.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_SUB|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sub.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SUB|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sub.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_SUB|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.avg.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_AVG|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.avg.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AVG|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avg.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_AVG|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.avgu.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_AVGU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.avgu.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_AVGU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.avgu.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_AVGU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.min.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_MIN|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.min.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MIN|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.min.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_MIN|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.minu.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_MINU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.minu.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_MINU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.minu.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_MINU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.max.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_MAX|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.max.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MAX|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.max.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_MAX|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.maxu.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_MAXU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.maxu.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_MAXU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.maxu.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_MAXU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.srl.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sci.h",	"Xpulpv3", "d,s,bU",	MATCH_V_OP_SRL|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.srl.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRL|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.srl.sci.b",	"Xpulpv3", "d,s,bU",	MATCH_V_OP_SRL|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sra.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sci.h",	"Xpulpv3", "d,s,bU",	MATCH_V_OP_SRA|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sra.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SRA|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sra.sci.b",	"Xpulpv3", "d,s,bU",	MATCH_V_OP_SRA|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sll.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sci.h",	"Xpulpv3", "d,s,bU",	MATCH_V_OP_SLL|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sll.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SLL|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sll.sci.b",	"Xpulpv3", "d,s,bU",	MATCH_V_OP_SLL|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.or.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sci.h",		"Xpulpv3", "d,s,bs",	MATCH_V_OP_OR|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.or.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_OR|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.or.sci.b",		"Xpulpv3", "d,s,bs",	MATCH_V_OP_OR|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.xor.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_XOR|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.xor.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_XOR|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.xor.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_XOR|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.and.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_H_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sc.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_H_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_AND|MATCH_V_OP_H_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.and.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_B_VV,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sc.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_AND|MATCH_V_OP_B_VR,		MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.and.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_AND|MATCH_V_OP_B_VI,		MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.abs.h",		"Xpulpv3", "d,s",	MATCH_V_OP_ABS|MATCH_V_OP_H_VV,		MASK_V_OP2,	match_opcode,	WR_xd|RD_xs1},
{"pv.abs.b",		"Xpulpv3", "d,s",	MATCH_V_OP_ABS|MATCH_V_OP_B_VV,		MASK_V_OP2,	match_opcode,	WR_xd|RD_xs1},

{"pv.extract.h",	"Xpulpv3", "d,s,bf",	MATCH_V_OP_EXTRACT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.extract.b",	"Xpulpv3", "d,s,bF",	MATCH_V_OP_EXTRACT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.extractu.h",	"Xpulpv3", "d,s,bf",	MATCH_V_OP_DOTSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.extractu.b",	"Xpulpv3", "d,s,bF",	MATCH_V_OP_DOTSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.insert.h",		"Xpulpv3", "d,s,bf",	MATCH_V_OP_SDOTUP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.insert.b",		"Xpulpv3", "d,s,bF",	MATCH_V_OP_SDOTUP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.dotsp.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_DOTUP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.dotsp.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotsp.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_DOTUP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.dotup.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.dotup.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotup.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_EXTRACTU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.dotusp.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_INSERT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.dotusp.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_INSERT|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.dotusp.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_INSERT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sdotsp.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sdotsp.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotsp.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_SDOTUSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sdotup.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_DOTUSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sdotup.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_DOTUSP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotup.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_DOTUSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.sdotusp.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_SDOTSP|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.sdotusp.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SDOTSP|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.sdotusp.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_SDOTSP|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.shuffle.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shuffle.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shuffle.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei0.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_SHUFFLE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei1.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_SHUFFLEI1|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei2.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_SHUFFLEI2|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shufflei3.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_SHUFFLEI3|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.shuffle2.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SHUFFLE2|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.shuffle2.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_SHUFFLE2|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.pack.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_PACK|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.packhi.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_PACKHI|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.packlo.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_PACKLO|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},

{"pv.cmpeq.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPEQ|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpeq.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPEQ|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpeq.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPEQ|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpne.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPNE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpne.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPNE|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpne.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPNE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpgt.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPGT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpgt.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGT|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgt.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPGT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpge.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPGE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpge.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGE|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpge.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPGE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmplt.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPLT|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmplt.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLT|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmplt.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPLT|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmple.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sci.h",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPLE|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmple.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLE|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmple.sci.b",	"Xpulpv3", "d,s,bs",	MATCH_V_OP_CMPLE|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpgtu.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPGTU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpgtu.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGTU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgtu.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPGTU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpgeu.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPGEU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpgeu.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPGEU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpgeu.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPGEU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpltu.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPLTU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpltu.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLTU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpltu.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPLTU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},

{"pv.cmpleu.h",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_H_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sc.h",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_H_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sci.h",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPLEU|MATCH_V_OP_H_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
{"pv.cmpleu.b",		"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_B_VV,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sc.b",	"Xpulpv3", "d,s,t",	MATCH_V_OP_CMPLEU|MATCH_V_OP_B_VR,	MASK_V_OP,	match_opcode,	WR_xd|RD_xs1|RD_xs2},
{"pv.cmpleu.sci.b",	"Xpulpv3", "d,s,bu",	MATCH_V_OP_CMPLEU|MATCH_V_OP_B_VI,	MASK_V_OP1,	match_opcode,	WR_xd|RD_xs1},
/*
{"pv.ball",       	"Xpulpv3",  "s,p",  	MATCH_BEQM1, 			MASK_BEQM1|MASK_RS2, 	match_opcode,   RD_xs1 },
{"pv.bnall",       	"Xpulpv3",  "s,p",  	MATCH_BNEM1, 			MASK_BNEM1|MASK_RS2, 	match_opcode,   RD_xs1 },
*/
{"p.beqimm",       	"Xpulpv3",  "s,bI,p",  	MATCH_BEQM1, 				MASK_BEQM1, 	match_opcode,   RD_xs1 },
{"p.bneimm",       	"Xpulpv3",  "s,bI,p",  	MATCH_BNEM1, 				MASK_BNEM1, 	match_opcode,   RD_xs1 },

/* Load from event unit */

{"p.elw",		"Xpulpv3", "d,o(s)",	MATCH_LWU, 				MASK_LWU, 	match_opcode,   WR_xd|RD_xs1 },

/* Rocket Custom Coprocessor extension */
{"custom0",   "Xcustom", "d,s,t,^j", MATCH_CUSTOM0_RD_RS1_RS2, MASK_CUSTOM0_RD_RS1_RS2, match_opcode, 0},
{"custom0",   "Xcustom", "d,s,^t,^j", MATCH_CUSTOM0_RD_RS1, MASK_CUSTOM0_RD_RS1, match_opcode, 0},
{"custom0",   "Xcustom", "d,^s,^t,^j", MATCH_CUSTOM0_RD, MASK_CUSTOM0_RD, match_opcode, 0},
{"custom0",   "Xcustom", "^d,s,t,^j", MATCH_CUSTOM0_RS1_RS2, MASK_CUSTOM0_RS1_RS2, match_opcode, 0},
{"custom0",   "Xcustom", "^d,s,^t,^j", MATCH_CUSTOM0_RS1, MASK_CUSTOM0_RS1, match_opcode, 0},
{"custom0",   "Xcustom", "^d,^s,^t,^j", MATCH_CUSTOM0, MASK_CUSTOM0, match_opcode, 0},
{"custom1",   "Xcustom", "d,s,t,^j", MATCH_CUSTOM1_RD_RS1_RS2, MASK_CUSTOM1_RD_RS1_RS2, match_opcode, 0},
{"custom1",   "Xcustom", "d,s,^t,^j", MATCH_CUSTOM1_RD_RS1, MASK_CUSTOM1_RD_RS1, match_opcode, 0},
{"custom1",   "Xcustom", "d,^s,^t,^j", MATCH_CUSTOM1_RD, MASK_CUSTOM1_RD, match_opcode, 0},
{"custom1",   "Xcustom", "^d,s,t,^j", MATCH_CUSTOM1_RS1_RS2, MASK_CUSTOM1_RS1_RS2, match_opcode, 0},
{"custom1",   "Xcustom", "^d,s,^t,^j", MATCH_CUSTOM1_RS1, MASK_CUSTOM1_RS1, match_opcode, 0},
{"custom1",   "Xcustom", "^d,^s,^t,^j", MATCH_CUSTOM1, MASK_CUSTOM1, match_opcode, 0},
{"custom2",   "Xcustom", "d,s,t,^j", MATCH_CUSTOM2_RD_RS1_RS2, MASK_CUSTOM2_RD_RS1_RS2, match_opcode, 0},
{"custom2",   "Xcustom", "d,s,^t,^j", MATCH_CUSTOM2_RD_RS1, MASK_CUSTOM2_RD_RS1, match_opcode, 0},
{"custom2",   "Xcustom", "d,^s,^t,^j", MATCH_CUSTOM2_RD, MASK_CUSTOM2_RD, match_opcode, 0},
{"custom2",   "Xcustom", "^d,s,t,^j", MATCH_CUSTOM2_RS1_RS2, MASK_CUSTOM2_RS1_RS2, match_opcode, 0},
{"custom2",   "Xcustom", "^d,s,^t,^j", MATCH_CUSTOM2_RS1, MASK_CUSTOM2_RS1, match_opcode, 0},
{"custom2",   "Xcustom", "^d,^s,^t,^j", MATCH_CUSTOM2, MASK_CUSTOM2, match_opcode, 0},
{"custom3",   "Xcustom", "d,s,t,^j", MATCH_CUSTOM3_RD_RS1_RS2, MASK_CUSTOM3_RD_RS1_RS2, match_opcode, 0},
{"custom3",   "Xcustom", "d,s,^t,^j", MATCH_CUSTOM3_RD_RS1, MASK_CUSTOM3_RD_RS1, match_opcode, 0},
{"custom3",   "Xcustom", "d,^s,^t,^j", MATCH_CUSTOM3_RD, MASK_CUSTOM3_RD, match_opcode, 0},
{"custom3",   "Xcustom", "^d,s,t,^j", MATCH_CUSTOM3_RS1_RS2, MASK_CUSTOM3_RS1_RS2, match_opcode, 0},
{"custom3",   "Xcustom", "^d,s,^t,^j", MATCH_CUSTOM3_RS1, MASK_CUSTOM3_RS1, match_opcode, 0},
{"custom3",   "Xcustom", "^d,^s,^t,^j", MATCH_CUSTOM3, MASK_CUSTOM3, match_opcode, 0},

};

#define RISCV_NUM_OPCODES \
  ((sizeof riscv_builtin_opcodes) / (sizeof (riscv_builtin_opcodes[0])))
const int bfd_riscv_num_builtin_opcodes = RISCV_NUM_OPCODES;

/* Removed const from the following to allow for dynamic extensions to the
   built-in instruction set.  */
struct riscv_opcode *riscv_opcodes =
  (struct riscv_opcode *) riscv_builtin_opcodes;
int bfd_riscv_num_opcodes = RISCV_NUM_OPCODES;
#undef RISCV_NUM_OPCODES
