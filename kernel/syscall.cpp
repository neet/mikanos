#include <cstdint>

#include "msr.hpp"
#include "asmfunc.h"

void InitializeSyscall()
{
	WriteMSR(kIA32_EFER, 0x0501u);
	WriteMSR(kIA32_LSTAR, reinterpret_cast<uint64_t>(SyscallEntry));
	WriteMSR(kIA32_STAR, static_cast<uint64_t>(8) << 32 | static_cast<uint64_t>(16 | 3) << 48);
}