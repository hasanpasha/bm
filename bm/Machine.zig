stack: Stack,

program: Program,

pc: usize = 0,

halt: bool = false,

pub const Stack = struct {
    data: std.ArrayList(Word),
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator, capacity: usize) !Stack {
        return .{ .data = try .initCapacity(allocator, capacity), .allocator = allocator };
    }

    pub fn deinit(self: *Stack) void {
        self.data.deinit(self.allocator);
    }

    pub fn get(self: Stack, idx: usize) Error!Word {
        const offset = @as(i64, @intCast(self.data.items.len)) - @as(i64, @intCast(idx)) - 1;
        if (offset < 0)
            return Error.stack_underflow;

        const addr: usize = @intCast(offset);
        return self.data.items[addr];
    }

    pub fn push(self: *Stack, word: Word) Error!void {
        self.data.append(self.allocator, word) catch return Error.stack_overflow;
    }

    pub fn pop(self: *Stack) Error!Word {
        return self.data.pop() orelse Error.stack_underflow;
    }
};

pub const Error = error{
    stack_overflow,
    stack_underflow,
    divide_by_zero,
    illegal_inst_access,
};

pub fn init(allocator: std.mem.Allocator) !Machine {
    return .{ .stack = try .init(allocator, 1024), .program = try .init(allocator, 1024) };
}

pub fn deinit(self: *Machine) void {
    self.stack.deinit();
    self.program.deinit();
}

pub fn fetch_inst(self: *Machine) Error!Inst {
    const ins = self.program.get_or_null(self.pc) orelse return Error.illegal_inst_access;
    self.pc += 1;
    return ins;
}

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

pub fn execute_program(self: *Machine, limit: Limit) Error!void {
    var limit_counter: isize = switch (limit) {
        .no_limit => -1,
        .limited => |limit_val| @intCast(limit_val),
    };

    while (!self.halt and limit_counter != 0) {
        const ins = try self.fetch_inst();
        try self.execute_inst(ins);

        if (limit_counter > 0)
            limit_counter -= 1;
    }
}

const Machine = @This();

const std = @import("std");
const log = std.log.scoped(.bm);

const Inst = @import("inst.zig").Inst;
const Word = Inst.Word;

pub const Program = @import("Program.zig");
