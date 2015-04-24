// EPOS IA32 CPU Mediator Implementation

#include <architecture/ia32/cpu.h>
#include <thread.h>

__BEGIN_SYS

// Class attributes
unsigned int IA32::_cpu_clock;
unsigned int IA32::_bus_clock;

// Class methods
void IA32::Context::save() volatile
{
    // Save the running thread context into its own stack (mostly for debugging)
    ASM("     push    %ebp                                            \n"
        "     mov     %esp, %ebp                                      \n"
        "     mov     8(%ebp), %esp   # sp = this                     \n"
        "     add     $40, %esp       # sp += sizeof(Context)         \n"
        "     push    4(%ebp)         # push eip                      \n"
        "     pushf                                                   \n"
        "     push    %eax                                            \n"
        "     push    %ecx                                            \n"
        "     push    %edx                                            \n"
        "     push    %ebx                                            \n"
        "     push    %ebp            # push esp                      \n"
        "     push    (%ebp)          # push ebp                      \n"
        "     push    %esi                                            \n"
        "     push    %edi                                            \n"
        "     mov     %ebp, %esp                                      \n"
        "     pop     %ebp                                            \n");
}

void IA32::Context::load() const volatile
{
    // Pop the context pushed into the stack during thread creation to initialize the CPU's context
    // Obs: POPA ignores the ESP saved by PUSHA. ESP is just normally incremented
    ASM("       mov     4(%esp), %esp       # sp = this             \n"
        "       popa                                                \n"
        "       popf                                                \n");
}

void IA32::switch_context(Context * volatile * o, Context * volatile n)
{
    // Save the previously running thread context ("o") into its stack 
    // and updates the its _context attribute
    // PUSHA saves an extra "esp" (which is always "this"), but saves several instruction fetches
    ASM("	pushf                  		                \n"
        "	pusha				                \n");
    ASM("       mov     40(%esp), %eax          # old           \n"
        "       mov     %esp, (%eax)                            \n");

    // Restore the next thread context ("n") from its stack (and the user-level stack pointer, updating the dummy TSS)
    ASM("	mov     44(%esp), %esp          # new	        \n");
    ASM("	popa				                \n"
        "	popf			                	\n");
    ASM("       mov     %%esp, %0                               \n" : "=m"(Thread::running()->_context) : );
}

__END_SYS
