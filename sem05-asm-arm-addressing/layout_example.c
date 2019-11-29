
__asm__ (R"(
.global cut_struct
cut_struct:
    push {r4, r5} // notice that we decrease sp by pushing
    ldr r4, [sp, #8] // get last arg from stack (in fact we use initial value of sp)
    // r0 - obj, r1 - c, r2 - i, r3 - s, r4 - c2
    ldrb r5, [r0, #0]
    strb r5, [r1]
    ldr r5, [r0, #1]
    str r5, [r2]
    ldrh r5, [r0, #5]
    strh r5, [r3]
    ldrb r5, [r0, #7]
    strb r5, [r4]
    pop {r4, r5}
    push {r1-r12} // still one instruction
    pop {r1-r12}
    bx  lr
  
s1_ptr:
    .word s1
s2_ptr:
    .word s2
s1:
    .ascii "%d\n\0" // 4 bytes -> 4 bytes after padding
s2:
    .ascii "%d%d\0" // 5 bytes -> 8 bytes after padding
d1:
    .word 1234
)");

