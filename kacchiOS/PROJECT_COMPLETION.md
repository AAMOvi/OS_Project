# kacchiOS - Lab Project Completion Summary

## Project Status: ✅ **COMPLETE**

All required ("Must Include"), "Good to have", and "Bonus" components have been implemented.

---

## ✅ Completed Features

### Memory Manager (70%)

- ✅ **Stack allocation** - `stack_alloc()` in [memory.c](memory.c)
- ✅ **Stack deallocation** - `stack_free()` in [memory.c](memory.c)
- ✅ **Heap allocation** - `heap_alloc()` with first-fit free list
- ✅ **Heap deallocation** - `heap_free()` with coalescing
- ✅ **Optimized allocation** - 16-byte alignment, block splitting, free-list coalescing

### Process Manager (20%)

- ✅ **Process table** - 8 PCB slots in [process.c](process.c)
- ✅ **Process creation** - `process_create()` with stack setup
- ✅ **State transition** - UNUSED → READY → CURRENT → BLOCKED/TERMINATED
- ✅ **Process termination** - `process_exit()` frees resources
- ✅ **Utility functions** - `process_current()`, `process_mark_ready()`
- ✅ **BLOCKED state** - for IPC blocking (bonus feature)
- ✅ **IPC** - Message queue with blocking send/recv ([ipc.c](ipc.c))

### Scheduler (10%)

- ✅ **Clear policy** - Cooperative round-robin with aging
- ✅ **Context switch** - Assembly helper in [context.S](context.S)
- ✅ **Configurable time quantum** - `scheduler_set_time_quantum()`
- ✅ **Aging** - Processes age in ready queue; promoted after threshold

---

## 🚀 How to Build & Run

### Build (in WSL Ubuntu)

```bash
cd /mnt/f/OS_Project/kacchiOS
make clean
make
```

### Run in QEMU

```bash
make run
```

### Demo Commands (in shell)

- `help` - Show available commands
- `send 123` - Send message via IPC to receiver process
- Type anything else to echo it back

---

## 📁 File Structure

```
kacchiOS/
├── memory.c / memory.h         # Heap/stack allocator with coalescing
├── process.c / process.h       # Process table, PCB, creation/exit
├── scheduler.c / scheduler.h   # Round-robin scheduler with aging
├── ipc.c / ipc.h               # Message queue IPC (blocking)
├── context.S                   # Context switch (esp/ebp/eip)
├── kernel.c                    # Main kernel: shell, heartbeat, IPC demo
├── boot.S                      # Multiboot entry, stack init
├── serial.c / serial.h         # COM1 serial I/O
├── string.c / string.h         # String utilities
├── link.ld                     # Linker script (separate RX/RW segments)
├── Makefile                    # Build system
└── README.md                   # This file
```

---

## 🎯 Assignment Compliance

| Component                            | Weight | Status              |
| ------------------------------------ | ------ | ------------------- |
| Memory Manager (stack/heap)          | 70%    | ✅ Complete         |
| Process Manager (table, states, IPC) | 20%    | ✅ Complete + Bonus |
| Scheduler (round-robin, aging)       | 10%    | ✅ Complete + Bonus |

**Total**: 100% + Bonus (IPC, aging, BLOCKED state)

---

## 🔧 Implementation Highlights

1. **Memory Manager**

   - 64KB heap with 16-byte alignment
   - First-fit allocation with block splitting
   - Coalescing on free to reduce fragmentation

2. **Process Manager**

   - 8 process slots, each with PID, state, context, stack
   - Bootstrap trampoline to launch process entry points
   - BLOCKED state for IPC synchronization

3. **Scheduler**

   - Cooperative yielding (no preemption yet; timer interrupt would be next)
   - Aging: processes waiting >3 yields get priority placement
   - Configurable time quantum (placeholder for future timer)

4. **IPC**
   - Blocking message queue (16-entry circular buffer)
   - Direct handoff optimization when receiver waiting
   - Unblock sender when receiver consumes message

---

## 📝 Testing

Run `make run` and observe:

- Welcome banner
- Shell prompt (`kacchiOS>`)
- Heartbeat process ticks incrementing
- Type `send 42` → receiver process prints `[ipc recv] value=42`
- Aging demonstrated when shell blocks on input and heartbeat accumulates age

---

## 📌 Notes

- Build warnings eliminated (no RWX segments, no missing stack notes)
- Cooperative scheduling (yield-driven); hardware timer interrupt would enable preemption
- IPC blocking/unblocking demonstrates BLOCKED state transitions

---

**Submitted by**: [Your Team Name]  
**Date**: January 3, 2026
