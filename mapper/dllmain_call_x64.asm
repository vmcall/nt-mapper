sub RSP, 0x28
movabs RCX, 0x0000000000000000	; hModule
mov RDX, 0x1					; fdwReason
xor R8, R8						; lpvReserved
movabs RAX, 0x0000000000000000	; DllMain Address
call RAX
add RSP, 0x28
ret