pushf
push   rax
push   rbx
push   rcx
push   rdx
push   rbp
push   rsi
push   rdi
push   r8
push   r9
push   r10
push   r11
push   r12
push   r13
push   r14
push   r15
sub    rsp,0x100

movabs rcx,0x0
mov    rdx,0x1
xor    r8,r8
movabs rax,0x0
call   rax

add    rsp,0x100
pop   r15
pop   r14
pop   r13
pop   r12
pop   r11
pop   r10
pop   r9
pop   r8
pop   rdi
pop   rsi
pop   rbp
pop   rdx
pop   rcx
pop   rbx
pop   rax
popf
mov qword ptr [rip+1], 1
ret
nop
nop
nop
nop
nop
nop
nop
nop