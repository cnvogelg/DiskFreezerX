		.section .stub,"ax"
		.global start_bin
_stub:
		LDR	R1, =_data_begin  /* start of data */
                LDR     R2, =0x200000     /* target pos of data */
                LDR     R3, =_stub_begin  /* end size of data */
LoopRel:        CMP     R1, R3
                LDRLO   R0, [R1], #4
                STRLO   R0, [R2], #4
                BLO     LoopRel
		        LDR	PC, =start_bin
