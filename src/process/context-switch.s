global switch_context
global restore_context

;void switch_context(struct InterruptFrame* new_context)
switch_context:
	mov eax, [esp + 4] ; move new_context to eax

	; These registers are not saved by previous function calls
    ; push ebp
    ; push ebx
    ; push esi
    ; push edi

	mov esp, eax ; load new_context as esp

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