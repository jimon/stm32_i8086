org 0h
mov ax,2000h
mov ds,ax
mov ax,2h
mov di,ax
mov ax, 5566h
mov word [ds:di], ax
waitloop:
mov ax,0
jmp waitloop
