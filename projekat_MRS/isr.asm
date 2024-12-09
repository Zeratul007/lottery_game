;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file


;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
			.text
P1ISR		push.w	R8						; save R8 value on stack
			mov.w	&P1IFG, R8				; read data into R8
			and.w	#0x0032, R8
			jz		end
			bis		#BIT4, &TBCTL			; move digit value to R12, argument register
			bic		#BIT1, &P1IFG				; call function WriteLed with argument in R12
			bic		#BIT4, &P1IFG
			bic		#BIT5, &P1IFG

end			pop.w	R8						; return R8 original value
			reti							; back to the main loop

;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".int47"                ; MSP430 USCIA1 Vector
            .short  P1ISR

