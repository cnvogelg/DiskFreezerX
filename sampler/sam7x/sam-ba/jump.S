		.section .init,"ax"
		.global _startup
		.func _startup
		LDR R4,=0x210000
		MOV SP, R4
		MSR CPSR_c, 0xd3
		MOV SP, R4
		LDR PC,=_stub_begin
