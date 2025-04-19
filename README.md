# FOS
This is the fruit of an academic project that we were assigned while taking CIS353 - Operating Systems. It is the most challenging project I worked on, and the one I enjoyed the most. The skeleton code for this project was forked from JOS, the operating system used by MIT for teaching their OS class. Also, the F stands for FCIS, the name of our department.

Setting up and using this project is a bit challenging, since it relies on a delicate setup where every component, including the IDE, is expected to be bundled with the code for it to work properly. Since this is impractical for an open source project, I might try to make this project more portable and simple to run. 

Git history is surprisingly shallow since we were not that comfortable with Git back then, and relied on sending the zipped repository back and forth on our WhatsApp group chat. Thankfully, we are way past this :)

## Project Overview

This project implements the following components:

- **Kernel Memory Management** - Dynamic allocation with first-fit algorithms
- **Process Management** - Command processing and system call interface
- **Memory Protection** - Fault handling and page management
- **Shared Memory** - Inter-process communication
- **User Space** - Heap management and memory allocation
- **Concurrency** - Semaphores and process synchronization
- **Scheduling** - Priority-based round-robin scheduler

## Features

#### Memory Management
- **Dynamic Allocator**
  - Block allocation with first-fit algorithm
  - Memory reallocation and deallocation
  - Kernel heap management with virtual/physical address translation
  - Fast page allocator for performance optimization

#### System Interface
- **Command Processing** - Interactive command prompt
- **System Calls** - Complete syscall interface with parameter validation
- **Sleep/Wakeup Mechanisms** - Process synchronization primitives

#### Memory Protection
- **Fault Handler** - Invalid pointer detection and page fault handling
- **Process Isolation** - Kernel dynamic allocation for process management

#### Shared Memory
- **Object Management** - Creation, search, and deletion of shared objects
- **Memory Allocation** - First-fit allocation for shared resources
- **Cross-Process Communication** - Safe memory sharing between processes

#### User Space
- **Heap Management** - User-level memory allocation
- **System Integration** - Seamless kernel-user space interaction

#### Concurrency
- **Semaphores** - Complete semaphore implementation
- **Process Synchronization** - Wait/signal operations

#### Scheduling
- **Priority Scheduler** - Multi-level priority management
- **Round-Robin Algorithm** - Fair CPU time distribution
- **Timer Integration** - Preemptive scheduling support

#### Advanced Features
- **Fault Handler II** - Enhanced fault detection with clock replacement algorithm
- **Performance Optimization** - Fast allocators and efficient data structures
- **Comprehensive Testing** - Extensive test suite with automated validation
## File Structure
```
kern/
├── mem/
│   ├── kheap.c/.h          # Kernel heap management
│   ├── shared_memory_manager.c  # Shared memory implementation
│   └── chunk_operations.c   # Memory chunk operations
├── trap/
│   ├── fault_handler.c     # Page fault handling
│   ├── trap.c              # Trap handling
│   └── syscall.c/.h        # System call interface
├── sched/
│   └── sched.c             # Process scheduler
├── cpu/
│   └── sched_helpers.c     # Scheduling utilities
├── cmd/
│   ├── commands.c          # Command processing
│   └── command_prompt.c    # Interactive shell
├── conc/
│   ├── channel.c           # Communication channels
│   └── sleeplock.c         # Sleep locks
├── proc/
│   ├── user_environment.c  # Process environment
│   └── working_set_manager.c # Memory working set
└── tests/ # Comprehensive testing
```
### User Space Libraries
```
lib/
├── uheap.c                 # User heap implementation
├── semaphore.c             # Semaphore library
├── syscall.c               # System call wrappers
└── dynamic_allocator.c     # Dynamic memory allocation
```
## Testing

Tests were provided by the faculty to aid in development, and some tests were only released after the submission deadline. The tests include:

- **Automated Testing**: Each module includes automated test scenarios
- **Integration Tests**: Cross-module functionality validation
- **Performance Tests**: Timing and efficiency benchmarks
- **Stress Tests**: High-load and edge case validation
### Test Categories
- **SEEN Tests**: Pre-validated test cases
- **UNSEEN Tests**: Hidden test scenarios for comprehensive evaluation
- **Performance Benchmarks**: Speed and efficiency measurements
## Authors
- [Abdelrahman Maamoun](https://github.com/amaamoun99)
- [Farida Ali](https://github.com/farridaali)
- [Hana Omar](https://github.com/HanaOmar1)
- [Manar Sayed](https://github.com/Manarsayedd)
- [Mostafa Faheem](https://github.com/mostafafaheem)
- [Yahia Aldeeb](https://github.com/YahiaAldeeb)
- [Yasmeen Ahmed](https://github.com/yassmeenahmedd)
