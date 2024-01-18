global switch_context
global restore_context

;void switch_context(Context** old_context, Context* new_context)
switch_context:
	mov eax, [esp + 4] ; move old_context to eax
	mov edx, [esp + 8] ; move new_context to edx

	; These registers are not saved by previous function calls
    push ebp
    push ebx
    push esi
    push edi

	mov [eax], esp ; set old_context to current esp 
	mov esp, edx ; load new_context as esp

	; Restore registers
    pop edi
    pop esi
    pop ebx
    pop ebp

	ret

;Basically restores state of all registers
restore_context:
	pop gs
	pop fs
	pop es
	pop ds

	popad
	add esp, 8

	sti
	iret