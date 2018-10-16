# nt-mapper
PE mapper in c++17

# Features
+ Relocate image
+ Fix import address table
  + Handle api-set
+ Export directory parsing (forwarded and normal)
+ Two execution modes: 'Thread creation' and 'Thread hijacking' 

# Thread Hijacker
+ Preserves all registers, volatile or not
  + Exception 1: SSE registers
  + Exception 2: AVX registers
+ Preserves all flags
+ Automatically frees hijack shellcode after execution
+ Arbitrary shadow-space for dllmain
+ Aligns stack in case of recusant code

# To-do
+ Static TLS
+ TLS callbacks
+ Loader entry
+ C++ exceptions

# Thanks
+ DarthTon
+ Daax
+ JustMagic
