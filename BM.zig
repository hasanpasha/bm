stack: Stack,

program: Program,

pc: Addr = 0,

halt: bool = false,

pub const stack_size = 1024;

pub const program_size = 1024;

pub const Word = u64;

pub const Addr = u64;

pub const Stack = struct {
    data: [stack_size]Word,
    idx: Addr,

    pub const init: Stack = std.mem.zeroes(Stack);

    pub fn get(self: Stack, idx: Addr) Error!Word {
        const offset = @as(i64, @intCast(self.idx)) - @as(i64, @intCast(idx)) - 1;
        if (offset < 0)
            return Error.stack_underflow;

        const addr: Addr = @intCast(offset);
        return self.data[addr];
    }

    pub fn push(self: *Stack, word: Word) Error!void {
        if (self.idx >= self.data.len) return Error.stack_overflow;
        self.data[self.idx] = word;
        self.idx += 1;
    }

    pub fn pop(self: *Stack) Error!Word {
        if (self.idx == 0) return Error.stack_underflow;
        self.idx -= 1;
        return self.data[self.idx];
    }
};

pub const Program = struct {
    insts: [program_size]Inst,
    size: usize,

    pub const init: Program = std.mem.zeroes(Program);

    pub fn init_from_slice(self: *Program, slice: []const Inst) void {
        self.size = slice.len;
        @memcpy(self.insts[0..slice.len], slice);
    }

    pub fn save_to_file(self: *const Program, io: std.Io, filepath: []const u8) !void {
        const file = try std.Io.Dir.cwd().createFile(io, filepath, .{ .truncate = true });
        defer file.close(io);

        var buffer: [1024]u8 = undefined;
        var filewriter = file.writer(io, &buffer);
        const writer = &filewriter.interface;

        for (0..self.size) |i| {
            const inst = self.insts[i];
            try writer.writeStruct(inst, .little);
        }

        try writer.flush();
    }

    pub fn load_from_file(self: *Program, io: std.Io, filepath: []const u8) !void {
        const file = try std.Io.Dir.cwd().openFile(io, filepath, .{});
        defer file.close(io);

        var buffer: [1024]u8 = undefined;
        var filereader = file.reader(io, &buffer);
        const reader = &filereader.interface;

        self.size = 0;

        while (self.size < self.insts.len) {
            const inst = reader.takeStruct(Inst, .little) catch |err| switch (err) {
                error.EndOfStream => break,
                else => return err,
            };

            self.insts[self.size] = inst;
            self.size += 1;
        }
    }
};

pub const Error = error{
    stack_overflow,
    stack_underflow,
    divide_by_zero,
    illegal_inst_access,
};

pub const init: Self = std.mem.zeroes(Self);

pub const InstType = enum(u8) {
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

pub const Inst = packed struct {
    type: InstType = .nop,
    operand: Word = 0,

    pub fn s(t: InstType) Inst {
        return .{ .type = t };
    }

    pub fn n(t: InstType, o: Word) Inst {
        return .{ .type = t, .operand = o };
    }
};

pub fn fetch_inst(self: *Self) Error!Inst {
    if (self.pc >= self.program.size)
        return Error.illegal_inst_access;

    const inst = self.program.insts[self.pc];
    self.pc += 1;
    return inst;
}

pub fn execute_inst(self: *Self, inst: Inst) Error!void {
    const inst_type = inst.type;
    const operand = inst.operand;

    switch (inst_type) {
        .nop => {},
        .push => try self.stack.push(operand),
        .drop => _ = try self.stack.pop(),
        .dup => {
            const word = try self.stack.get(operand);
            try self.stack.push(word);
        },
        .addi => {
            const b = try self.stack.pop();
            const a = try self.stack.pop();
            try self.stack.push(a +% b);
        },
        .subi => {
            const b = try self.stack.pop();
            const a = try self.stack.pop();
            try self.stack.push(a -% b);
        },
        .multi => {
            const b = try self.stack.pop();
            const a = try self.stack.pop();
            try self.stack.push(a *% b);
        },
        .divi => {
            const b = try self.stack.pop();
            const a = try self.stack.pop();

            if (b == 0) return Error.divide_by_zero;

            try self.stack.push(a / b);
        },
        .jmp => self.pc = operand,
        .debug_print => log.debug("{}", .{try self.stack.pop()}),
        .hlt => self.halt = true,
    }
}

pub const Limit = union(enum) {
    no_limit,
    limited: usize,
};

pub fn execute_program(self: *Self, limit: Limit) Error!void {
    var limit_counter: isize = switch (limit) {
        .no_limit => -1,
        .limited => |limit_val| @intCast(limit_val),
    };

    while (!self.halt and limit_counter != 0) {
        const inst = try self.fetch_inst();
        try self.execute_inst(inst);

        if (limit_counter > 0)
            limit_counter -= 1;
    }
}

const insts = [_]Inst{
    .s(.nop),
    .s(.nop),
    .n(.push, 0),
    .n(.push, 15),
    .s(.addi),
    .n(.dup, 0),
    .s(.debug_print),
    .n(.jmp, 3),
    .s(.nop),
    .s(.nop),
    .s(.nop),
    .s(.hlt),
};

pub fn main(_init: std.process.Init) !void {
    var bm = Self.init;

    // bm.program.init_from_slice(&insts);
    // try bm.program.save_to_file(_init.io, "test.bm");

    try bm.program.load_from_file(_init.io, "test.bm");
    try bm.execute_program(.{ .limited = 40 });
}

const Self = @This();

const std = @import("std");
const log = std.log.scoped(.bm);
