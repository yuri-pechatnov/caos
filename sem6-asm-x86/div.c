.intel_syntax noprefix

        .text
        .global summ
summ:
    push ebx
    mov  ecx , N //ecx - counter, in the loop --N while N!=0

.Loop:
    //складываем с конца
    mov  ebx, ecx
    dec  ebx //ebx = ecx - 1
    mov  edx, A
    mov  eax, [edx + 4*ebx]
    mov  edx, B
    add  eax, [edx + 4*ebx]
    mov  edx, R
    mov  [edx + 4*ebx], eax
    loop .Loop
    //out of loop
    pop  ebx
    ret

    .text
    .global everyday795
everyday795:
    push ebp
    mov  ebp, esp
    sub  esp, 4 //int n; &n == [ebp - 4]
    push ebx

    mov  ebx, ebp //ebx = &n
    sub  ebx, 4   //ebx = &n
    push ebx
    mov  ebx, offset scanf_format
    push ebx
    call scanf
    add  esp, 8

    mov  ebx, [ebp - 4] // ebp = n
    imul ebx, [ebp + 8] // n *= x
    add  ebx, [ebp + 12] // n += y

    push ebx
    mov  ebx, printf_format_ptr
    push ebx
    call printf
    add  esp, 8

    pop ebx
    mov esp, ebp
    pop ebp
    ret

scanf_format:
    .string "%d"

printf_format_ptr:
    .long printf_format

    .data
printf_format:
    .string "%d\n"

