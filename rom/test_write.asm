org 0h
mov ax,2000h
mov ds,ax
mov ax,2h
mov di,ax
mov ax, 5566h
mov word [ds:di], ax
a:
mov ax,2
jmp a
; hlt