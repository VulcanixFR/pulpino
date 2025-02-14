#!/bin/python3

# Au CIME, on a Python 2 !

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

print(f"{pset_mask:b}", "<- Les bits pour rs1 et rs2 sont forcés à 0 dans p.set")
print(f"{mask:b}", "<- Masque considéré normal")

instr, match, mask = make_s(0x23, 0x7, 0, 0, 0)
print(f"{pspsw_match} == {match}", pspsw_match == match)
print(f"{pspsw_mask} == {mask}", pspsw_mask == mask)

