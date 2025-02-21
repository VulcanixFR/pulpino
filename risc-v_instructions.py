#!/bin/python3

def parse_r (instruction):
    return {
        "opcode":   f"7'h{(instruction & 0b1111111):x}",
        "rd":       f"5'h{((instruction >> 7) & 0b11111):x}",
        "func3":    f"3'h{((instruction >> 12) & 0b111):x}",
        "rs1":      f"5'h{((instruction >> 15) & 0b11111):x}",
        "rs2":      f"5'h{((instruction >> 20) & 0b11111):x}",
        "func7":    f"7'h{((instruction >> 25) & 0b1111111):x}"
    }

def parse_i (instruction):
    opcode  = instruction & 0b1111111
    rd      = (instruction >> 7) & 0b11111
    func3   = (instruction >> 12) & 0b111
    rs1     = (instruction >> 15) & 0b11111
    imm     = instruction >> 20
    return {
        "opcode": f"7'h{opcode:x}",
        "rd": f"5'h{rd:x}",
        "func3": f"3'h{func3:x}",
        "rs1": f"5'h{rs1:x}",
        "imm": f"12'h{imm:x}"
    }

def parse_s (instruction):
    return {
        "opcode":   f"7'h{(instruction & 0b1111111):x}",
        "func3":    f"3'h{((instruction >> 12) & 0b111):x}",
        "rs1":      f"5'h{((instruction >> 15) & 0b11111):x}",
        "rs2":      f"5'h{((instruction >> 20) & 0b11111):x}",
        "imm":      f"12'h{((instruction >> 20) | ((instruction >> 7) & 0b11111)):x}"
    }

def make_r (opcode, rd, funct3, rs1, rs2, func7):
    instr = (
        opcode          |
        (rd << 7)       |
        (funct3 << 12)  |
        (rs1 << 15)     |
        (rs2 << 20)     |
        (func7 << 25)
    )
    match = (
        opcode          |
        (0 << 7)        |
        (funct3 << 12)  |
        (0   << 15)     |
        (0   << 20)     |
        (func7 << 25)  
    )
    mask = (
        0b1111111       |
        (0 << 7)        |
        (0b111 << 12)   |
        (0   << 15)     |
        (0   << 20)     |
        (0b1111111 << 25)  
    )
    return (instr, match, mask)

def make_i (opcode, rd, func3, rs1, imm):
    instr = (
        opcode          |
        (rd << 7)       |
        (func3 << 12)   |
        (rs1   << 15)   |
        (imm   << 20)
    )
    match = (
        opcode          |
        (0 << 7)        |
        (func3 << 12)   |
        (0     << 15)   |
        (0     << 20)
    )
    mask = (
        0b1111111       |
        (0 << 7)        |
        (0b111 << 12)   |
        (0 << 15)       |
        (0 << 20)
    )
    return (instr, match, mask)

def make_s (opcode, func3, rs1, rs2, imm):
    instr = (
        opcode                  |
        ((imm & 0b11111) << 7)  |
        (func3 << 12)           |
        (rs1 << 15)             |
        (rs2 << 20)             |
        ((imm >> 5) << 25)      
    )
    match = (
        opcode                  |
        (0 << 7)                |
        (func3 << 12)           |
        (0   << 15)             |
        (0   << 20)             |
        (0 << 25)      
    )
    mask = (
        0b1111111               |
        (0 << 7)                |
        (0b111 << 12)           |
        (0   << 15)             |
        (0   << 20)             |
        (0 << 25)      
    )
    return (instr, match, mask)

# Affiche les instructions de façon joli
def col16 (n: str):
    return f"\x1b[38;5;{n}m"

def print_r (i: int):
    return (
        "\x1b[1m"
        + col16(198) +
        f"{((i >> 25) & 0b1111111):07b} "
        + col16(46) +
        f"{((i >> 20) & 0b11111):05b} "
        + col16(129) +
        f"{((i >> 15) & 0b11111):05b} "
        + col16(196) +
        f"{((i >> 12) & 0b111):03b} "
        + col16(27) +
        f"{((i >> 7) & 0b11111):05b} "
        + col16(202) +
        f"{((i >> 0) & 0b1111111):07b}"
        "\x1b[0m"
    )

def print_s (i: int):
    return (
        "\x1b[1m"
        + col16(50) +
        f"{((i >> 25) & 0b1111111):07b} "
        + col16(46) +
        f"{((i >> 20) & 0b11111):05b} "
        + col16(129) +
        f"{((i >> 15) & 0b11111):05b} "
        + col16(196) +
        f"{((i >> 12) & 0b111):03b} "
        + col16(50) +
        f"{((i >> 7) & 0b11111):05b} "
        + col16(202) +
        f"{((i >> 0) & 0b1111111):07b}"
        "\x1b[0m"
    )

print("Test instructions existantes")

pset_match  = 0xb4000033
pset_mask   = 0xfffff07f
pspsw_match = 0x00007023
pspsw_mask  = 0x0000707f

print("p.set")
print(parse_r(pset_match))

print("p.spsw")
print(parse_s(pspsw_match))

print("Test des calculs de match et de mask")

instr, match, mask = make_r(0x33, 0, 0x0, 0, 0, 0x5a)
print(f"{pset_match} == {match}", pset_match == match)
print(f"{pset_mask} == {mask}", pset_mask == mask)

print(f"{print_r(pset_mask)}", "<- Les bits pour rs1 et rs2 sont forcés à 0 dans p.set")
print(f"{print_r(mask)}", "<- Masque considéré normal")

instr, match, mask = make_s(0x23, 0x7, 0, 0, 0)
print(f"{pspsw_match} == {match}", pspsw_match == match)
print(f"{pspsw_mask} == {mask}", pspsw_mask == mask)

# Espace de réflexion :

# Cet opcode servira à :
# - Implémenter les variantes manquantes de p.spsw
# - Impélmenter p.hmem
OPCODE_existe   = [ 0x73, 0x0f, 0x33, 0x13, 0x23, 0x03, 0x63, 0x67, 0x6f, 0x17, 0x37, 0x0b, 0x2b, 0x5b, 0x57, 0x7b ]
OPCODE_neo      = [ (i << 3) | 0b011 for i in range(16) ]
OPCODE_dispo    = [ n for n in OPCODE_neo if n not in OPCODE_existe ]
__str_dispo     = [ f"{n:02x}" for n in OPCODE_dispo ]
print("OPCODEs disponibles :", ", ".join(__str_dispo))

OPCODE_DIFTSTORE = OPCODE_dispo[0]
print(f"OPCODE DIFT_STORE = {OPCODE_DIFTSTORE:X} ({OPCODE_DIFTSTORE:07b})")
OPCODE_DIFTIMM = OPCODE_dispo[1] 
print(f"OPCODE DIFT_IMM = {OPCODE_DIFTIMM:X} ({OPCODE_DIFTIMM:07b})")

# Pour l'implémentation des fonctionnalités, 
# on se basera sur l'opcode OPCODE_STORE

# p.hmark :
print("\nConception de l'instruction p.hmark : ")

# [Pas possible]
# - p.hmark xr, N
#   Marque le niveau de sécurité du registre xr à N
# => Type I (*pas de rd)
# => Modifier un addi x0, xr, N car équivaut à un nop dans notre cas
# => Ne permettre que les 2 bits de poids faible dans imm
# ! => Les instructions I ne sont pas facilement détournables
# -> Recherche d'une alternative

# [Alternative]
# - p.hmark xr, xh
#   Marque le niveau de sécurité du registre xr à la valeur contenue dans xh
#
# Exemple :
#   addi t6, 2
#   p.hmark r1, t6
#   jalr ra, r1
#
# Création de l'instruction : à l'image de p.set
# Détournement de `add x0, xr, xh` via func7 et func3
# On réutilise le func7 utilisé pour p.set
# On passe func3 à 001

p_hmark_mask  = make_r(0b1111111, 0b11111, 0b111, 0, 0, 0b1111111)[0]
p_hmark_match = make_r(0x33, 0, 0b001, 0, 0, 0x5a)[0]
print(f"p.hmark MASK  : 0x{p_hmark_mask:08X} ({print_r(p_hmark_mask)} <- Les bits pour rd sont forcés à 0)")
print(f"p.hmark MATCH : 0x{p_hmark_match:08X} ({print_r(p_hmark_match)})")

def make_p_hmark (rs1: int, rs2: int):
    inv = ~p_hmark_mask
    t = make_r(0, 0, 0, rs1, rs2, 0)[0]
    return p_hmark_match | (t & inv)

# p.hset
print("\nConception de l'instruction p.hset : ")

# - p.hset xh
#   Définit le niveau de sécurité pour une fonction appelée. Ne prend
#   effet qu'après un jal. Le niveau de sécurité pris est spécifié 
#   dans xh.
#  
# Exemple :
#   addi t6, 1
#   p.hset t6
#   jal ra, -12
#
# Création de l'instruction : à l'image de p.set
# Détournement de `add x0, x0, xh` via func7 et func3
# On réutilise le func7 utilisé pour p.set
# On passe func3 à 010

p_hset_mask  = make_r(0b1111111, 0b11111, 0b111, 0b11111, 0, 0b1111111)[0]
p_hset_match = make_r(0x33, 0, 0b010, 0, 0, 0x5a)[0]
print(f"p.hset MASK  : 0x{p_hset_mask:08X} ({print_r(p_hset_mask)} <- Les bits pour rd sont forcés à 0)")
print(f"p.hset MATCH : 0x{p_hset_match:08X} ({print_r(p_hset_match)})")

def make_p_hset (rs2: int):
    inv = ~p_hset_mask
    t = make_r(0, 0, 0, 0, rs2, 0)[0]
    return p_hset_match | (t & inv)

print("\nConception de l'instruction p.hset : ")
# - p.hmem
#
# Création de l'instruction : à l'image de p.spsw
# On passe func3 à 011

_, p_hmem_match, p_hmem_mask = make_s(OPCODE_DIFTSTORE, 0b011, 0, 0, 0)

print(f"p.hmem MASK  : 0x{p_hmem_mask:08X} ({print_r(p_hmem_mask)}")
print(f"p.hmem MATCH : 0x{p_hmem_match:08X} ({print_r(p_hmem_match)})")

print("\nExemples d'instructions hiérarchiques en binaire")

ex1 = make_p_hmark(1, 2)
print(f"p.hmark x1, x2    0x{ex1:08X} ({print_r(ex1)})")
ex1 = make_p_hset(6)
print(f"p.hset x6         0x{ex1:08X} ({print_r(ex1)})")

print("\nRedéfinition des instructions liées au DIFT")

_, p_spsb_match, p_spsb_mask = make_s(OPCODE_DIFTSTORE, 0b000, 0, 0, 0)
_, p_spsh_match, p_spsh_mask = make_s(OPCODE_DIFTSTORE, 0b001, 0, 0, 0)
_, p_spsw_match, p_spsw_mask = make_s(OPCODE_DIFTSTORE, 0b010, 0, 0, 0)

print(f"p.spsb MASK  : {p_spsb_mask:08X} ({print_s(p_spsb_mask)})")
print(f"p.spsb MATCH : {p_spsb_match:08X} ({print_s(p_spsb_match)})")
print()
print(f"p.spsh MASK  : {p_spsh_mask:08X} ({print_s(p_spsh_mask)})")
print(f"p.spsh MATCH : {p_spsh_match:08X} ({print_s(p_spsh_match)})")
print()
print(f"p.spsw MASK  : {p_spsw_mask:08X} ({print_s(p_spsw_mask)})")
print(f"p.spsw MATCH : {p_spsw_match:08X} ({print_s(p_spsw_match)})")

print("\nRécapitulatif des masks et matchs")
print("Instruction", "|", "Mask" + " " * 33, "|", "Match")
print("p.set      ", "|", print_r(pset_mask), "|", print_r(pset_match))
print("p.spsb     ", "|", print_s(p_spsb_mask), "|", print_s(p_spsb_match))
print("p.spsh     ", "|", print_s(p_spsh_mask), "|", print_s(p_spsh_match))
print("p.spsw     ", "|", print_s(p_spsw_mask), "|", print_s(p_spsw_match))
print("p.hmark    ", "|", print_r(p_hmark_mask), "|", print_r(p_hmark_match))
print("p.hset     ", "|", print_r(p_hset_mask), "|", print_r(p_hset_match))
print("p.hmem     ", "|", print_s(p_hmem_mask), "|", print_s(p_hmem_mask))


"""

{"sb",        "I",   "t,q(s)",  MATCH_SB, MASK_SB, match_opcode,   RD_xs1|RD_xs2 },
{"sh",        "I",   "t,q(s)",  MATCH_SH, MASK_SH, match_opcode,   RD_xs1|RD_xs2 },
{"sw",        "I",   "t,q(s)",  MATCH_SW, MASK_SW, match_opcode,   RD_xs1|RD_xs2 },

{"addi",      "I",   "d,s,j",  MATCH_ADDI, MASK_ADDI, match_opcode,  WR_xd|RD_xs1 },

#define WR_xd INSN_WRITE_GPR_D
#define WR_fd INSN_WRITE_FPR_D
#define RD_xs1 INSN_READ_GPR_S
#define RD_xs2 INSN_READ_GPR_T
#define RD_xs3 INSN_READ_GPR_R
#define RD_fs1 INSN_READ_FPR_S
#define RD_fs2 INSN_READ_FPR_T
#define RD_fs3 INSN_READ_FPR_R

"""