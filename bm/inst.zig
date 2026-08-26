pub const Inst = packed struct {
    type: Type = .nop,
    operand: Word = 0,

    pub const Type = enum(u8) {
        nop,
        hlt,
        jmp,
        push,
        drop,
        swap,
        dup,
        addi,
        subi,
        multi,
        divi,
        debug_print,

        pub fn has_operand(self: Type) bool {
            return switch (self) {
                .jmp, .push, .dup, .swap => true,
                else => false,
            };
        }

        pub const alternatives = .{
            .dump = .debug_print,
        };
    };

    pub const Word = u64;

    pub fn format(self: @This(), writer: *Writer) Writer.Error!void {
        try writer.print("{t}", .{self.type});
        if (self.type.has_operand()) {
            try writer.print("({})", .{self.operand});
        }
    }

    pub fn read(reader: *Reader) Reader.Error!?Inst {
        return reader.takeStruct(Inst, .little) catch |err|
            switch (err) {
                Reader.Error.EndOfStream => null,
                else => err,
            };
    }

    pub fn write(self: Inst, writer: *Writer) Writer.Error!void {
        try writer.writeStruct(self, .little);
    }
};

const std = @import("std");
const Io = std.Io;
const Reader = Io.Reader;
const Writer = Io.Writer;
