global switch_context
global restore_context

;void switch_context(Task* old_task, Task* new_task)
; I have no idea why it's not possible to save the context by moving the registers to memory
; It felt more elegant to be able to save it into global variables that can always be tracked
; Works for now, though
switch_context:
	mov eax, [esp + 4]

	; mov [eax + 16], ecx
	; mov [eax + 20], edx
	; mov [eax + 32], ebp
	; mov [eax + 12], ebx
	; mov [eax + 24], esi
	; mov [eax + 28], edi
    ; push ebp
    ; push ebx
    ; push esi
    ; push edi

	mov [eax + 4], esp
	mov eax, [esp + 8]
	mov esp, [eax + 4] 

    ; pop edi
    ; pop esi
    ; pop ebx
    ; pop ebp
	; mov edi, [eax + 28]
	; mov esi, [eax + 24]
	; mov ebx, [eax + 12]
	; mov ebp, [eax + 32]
	; mov edx, [eax + 20]
	; mov ecx, [eax + 16]

	ret

;Basically restores state of all registers
restore_context:
	pop gs
	pop fs
	pop es
	pop ds

	popad
	add esp, 8
	iret