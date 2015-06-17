org 0h
mov al,10h
out 20h, al
waitloop:
mov ax,0
jmp waitloop
