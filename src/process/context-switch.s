global switch_context
global restore_context

;void switch_context(struct InterruptFrame* new_context)
switch_context:
	mov eax, [esp + 4] ; move new_context to eax
	mov esp, eax ; load new_context as esp

    ; PIC_ACK timer interrupt
    mov al, 0x20
    mov dx, 0x20
    out dx, al

    ; Restore loaded context
    popad
    pop gs
	pop fs
	pop es
	pop ds

    add esp, 8

	iret


;Basically restores state of all registers
restore_context:
	popad
	pop gs
	pop fs
	pop es
	pop ds

	add esp, 8

	iret