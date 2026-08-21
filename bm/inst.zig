pub const Inst = packed struct {
    type: Type = .nop,
    operand: Word = 0,

    pub const Type = enum(u8) {
        nop,
        hlt,
        jmp,
        push,
        drop,
        dup,
        addi,
        subi,
        multi,
        divi,
        debug_print,
    };

    pub const Word = u64;
};
