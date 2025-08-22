bits 64
section .text

global SyscallLogString
SyscallLogString:
	mov eax, 0x80000000
	mov r10, rcx
	syscall
	ret

global SyscallPutString 
SyscallPutString:
	mov eax, 0x80000001
	mov r10, rcx
	syscall
	ret

global SyscallExit 
SyscallExit:
	mov eax, 0x80000002
	mov r10, rcx
	syscall
	ret

global SyscallOpenWindow 
SyscallOpenWindow:
	mov eax, 0x80000003
	mov r10, rcx
	syscall
	ret

global SyscallWinWriteString 
SyscallWinWriteString:
	mov eax, 0x80000004
	mov r10, rcx
	syscall
	ret

global SyscallWinFillRectangle 
SyscallWinFillRectangle:
	mov eax, 0x80000005
	mov r10, rcx
	syscall
	ret

global SyscallGetCurrentTick 
SyscallGetCurrentTick:
	mov eax, 0x80000006
	mov r10, rcx
	syscall
	ret