global switch_context
global restore_context

;void switch_context(struct InterruptFrame* new_context, uint32_t cs)
switch_context:
    ; PIC_ACK timer interrupt
    mov al, 0x20
    mov dx, 0x20
    out dx, al

	mov eax, [esp + 4] ; move new_context to eax
	mov ebx, [esp + 8] ; move intra_privilege to ebx
	mov esp, eax ; load new_context as esp

    ; check privilege change
    ; On a non privilege change, iret does not pop useresp
    ; Hence, we have to load the stack manually
    mov ecx, cs
    and ecx, 3
    and ebx, 3
    cmp ebx, ecx
    jne non_privilege_change
	mov eax, [eax + 12]         ; load past iframe with offset from saved ebp
	sub eax, 0x20
	mov esp, eax

non_privilege_change:
    ; Restore loaded context
    popad
    pop gs
	pop fs
	pop es
	pop ds

    add esp, 8

	iret
