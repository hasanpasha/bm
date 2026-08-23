stack: Stack,

pc: usize = 0,

halt: bool = false,

pub const Stack = struct {
    data: std.ArrayList(Word),
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator, cap: usize) std.mem.Allocator.Error!Stack {
        return .{ .data = try .initCapacity(allocator, cap), .allocator = allocator };
    }

    pub fn deinit(self: *Stack) void {
        self.data.deinit(self.allocator);
    }

    pub fn get(self: *const Stack, idx: usize) Error!Word {
        const offset = @as(i64, @intCast(self.data.items.len)) - @as(i64, @intCast(idx)) - 1;
        if (offset < 0)
            return Error.stack_underflow;

        const addr: usize = @intCast(offset);
        return self.data.items[addr];
    }

    pub fn push(self: *Stack, word: Word) Error!void {
        if (self.data.items.len >= self.data.capacity)
            return Error.stack_overflow;

        self.data.appendAssumeCapacity(word);
    }

    pub fn pop(self: *Stack) Error!Word {
        return self.data.pop() orelse return Error.stack_underflow;
    }
};

pub const Error = error{
    stack_overflow,
    stack_underflow,
    divide_by_zero,
    illegal_inst_access,
};

pub fn execute_inst(self: *Machine, ins: Inst) Error!void {
    const inst_type = ins.type;
    const operand = ins.operand;

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

pub fn execute_program(self: *Machine, program: Program, limit: Limit) Error!void {
    var limit_counter: isize = switch (limit) {
        .no_limit => -1,
        .limited => |limit_val| @intCast(limit_val),
    };

    while (!self.halt and limit_counter != 0) {
        if (self.pc >= program.insts.items.len)
            return Error.illegal_inst_access;

        const inst = program.insts.items[self.pc];
        defer self.pc += 1;

        try self.execute_inst(inst);

        if (limit_counter > 0)
            limit_counter -= 1;
    }
}

const Machine = @This();

const std = @import("std");
const assert = std.debug.assert;
const log = std.log.scoped(.bm);

const Inst = @import("inst.zig").Inst;
const Word = Inst.Word;
const Program = @import("Program.zig");
