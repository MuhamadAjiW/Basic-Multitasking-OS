global switch_context
global restore_context

;void switch_context(Task* old_task, Task* new_task)
switch_context:
	mov eax, [esp + 4] ; move old_task to eax
	mov [eax + 4], esp ; move esp to old_task's tsp 
	mov eax, [esp + 8] ; get new_task to eax
	mov esp, [eax + 4] ; load new_task's esp

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