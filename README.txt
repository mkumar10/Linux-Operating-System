README

Team x2A, Members - Bhanu, Wei, Maghav, Rachit

This directory contains an implementation of a basic Operating System for the x86 Architecture(Intel 32 bit).

Contents:
- Device Drivers
	- Terminal
	- Programmable Interval Timer(PIT)
	- Real Time Clock(RTC)
	- Read Only File System
	- PS_2 Controller

- Paging and Segmentation

- System Calls
- Interrupts
- Exceptions

- Scheduling and Parallel Processing

- Unique Implementation of a Try Lock

How to Run:
Run kernel.c on an x86 Virtual Machine.
Written in x86 assembly and C as part of ECE-391 coursework over a span of 4 weeks.


Parts of Code are Copyright University of Illinois, Urbana Champaign

---------------------------------------------------------------------------------------------------------------


Team x2A Bug Log

Checkpoint 1
1) 
	Problem: Triple Fault after the IDT Initialization
	Solution: We weren't using the correct Gate values for the mapping to the interrupt vectors.
	Time to Fix: 2 hours

2)
	Problem: Undefined behavior during the initialization process. Abrupt triple faults.
	Solution: We were using %eax in Assembly code, but didn't initialize the %eax value to 0
			  Debugged through gdb, using si
	Time to Fix: 3 hours with the help of Mika
---

Checkpoint 2
1)
	Problem: Terminal scrolling issues. The new input buffer would show up at the top instead of at the bottom.
	Solution: Logical bug. Removed through code review and revision.
	Time to Fix: 1 hour

2)
	Problem: Returning from Keyboard Interrupt - Page Fault
	Solution: We weren't clearing the stack before doing iret. So, we had an incorrect iret context.
			  Added Assembly Linkage to cleanup stack before iret.
	Time to Fix: 4 hours
---

Checkpoint 3
1)
	Problem: General Protection Fault during execute
	Solution: Added a critical section, and changed helper function to void inline functions. We were messing up the stack for 
			  the iret context.
	Time to Fix: 3 hours

2)
	Problem: Page Fault Exception - After context switching
	Solution: The paging was setup incorrectly and we didn't modify the Page Directory, Table to allow user-level access. 
	Time to Fix: 0.5 hours

3)
	Problem: Returning control to parent program after halt would raise an Exception sometimes Triple Fault, give General Protection or 
			 just Page Fault.
	Solution: esp0 value had to be set always to the bottom of the program's kernel stack when returning. We were saving the incorrect
			  esp0 value during execute.
	Time to Fix: 6 hours
---

Checkpoint 4
1)
	Problem: cat Page Faults for files with data spread over multiple data nodes.
	Solution: read_data usage was incorrect. The terminal_read system call wouldn't use and update the appropriate parameters to read_data.
			  We combined the old read_data function with the new expected read system call functionality.
	Time to Fix: 2 hours

2)
	Problem: RTC_read was setup incorrectly
	Solution: RTC_read didn't block until the next RTC tick. We weren't using our flag correctly.
	Time to Fix: 1 hour

3)
	Problem: Restart the shell enough number of times and we would Page Fault.
	Solution: Problem with halt. We forgot to decrement the process_id when restarting the shell.
			  So, we would never really restart the shell, instead just start a new shell.
			  Stepped through gdb to realize that the pid was always incrementing.
	Time to Fix: 3 hours
---

Checkpoint 5

1)
	Problem: We would Page Fault or get General Protection Fault when switching stacks for programs.
	Solution: We weren't saving the correct ESP, EBP values
	Time to Fix: 2 hours

2)
	Problem: We accidentally created 2 different scheduling schemes that confilcted
	Solution: We had to remove switching context/execution when pressing Alt + F key
	Time to Fix: 1 hour

3)
	Problem: We would Page/General Protection Fault when switching quickly between multiple terminals.
	Solution: Multiple functions would access the same variables. Protected the shared resources by adding a spin lock.
	Time to Fix: 2 hours

4) 
	Problem: Terminal read, write flags were completely mismatched in the multiple terminals scenario
	Solution: We had to rewrite an entire terminal_putc handler and setup the values of the flags appropriately again.
	Time to Fix: 7 hours

5) 
	Problem: Multiple fish would not work correctly and lag when multiple programs run that required RTC ticks.
	Solution: We had to virtualize our Vidmap syscall paging and RTC.
	Time to Fix: 4 hours
---

