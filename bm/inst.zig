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

        pub fn has_operand(self: Type) bool {
            return switch (self) {
                .jmp, .push, .dup => true,
                else => false,
            };
        }
    };

    pub const Word = u64;

    pub fn format(self: @This(), writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{t}", .{self.type});
        if (self.type.has_operand()) {
            try writer.print("({})", .{self.operand});
        }
    }
};

const std = @import("std");
