#include <xinu.h>
#include <stdint.h>
#include <stdio.h>
#define UNGETMAX 10             /* Can un-get at most 10 characters. */
static unsigned char ungetArray[UNGETMAX];
unsigned int  bufp = 0; 

/*@author nathan rusch, josh wild, chat gpt, stack overflow
 *
 */

syscall kgetc(){
	
	volatile struct ns16550_uart_csreg *regptr;
	regptr = (struct ns16550_uart_csreg *)UART_BASE;
	
	char c = 0; 
	//gets the character from the buffer array 
	if(bufp  > 0){
	    	bufp--;
		c = ungetArray[bufp];
		return c; 	
	}
    	//busy waiter so that lsr is not overwhelmed and dr is ready
    	while(!(regptr -> lsr & UART_LSR_DR)){
    		}
    	c = regptr -> rbr;
 
    	return c; 
}

syscall kcheckc(void){
	
    	volatile struct ns16550_uart_csreg *regptr;
    	regptr = (struct ns16550_uart_csreg *)UART_BASE;	
    
    	//check unget array for characters
    	if(bufp  > 0){
            	return TRUE;
    	}
    	//check if lsr and dr have data ready and return accordingly
    	if((regptr -> lsr & UART_LSR_DR)){
	    	return TRUE;
	}
	return FALSE;
    }


syscall kungetc(unsigned char c){
	if(bufp < UNGETMAX){
		ungetArray[bufp++] = c;
	        kprintf("success\r\n");	
		return c; 
	}else{
		kprintf("error\r\n");
		return SYSERR; 
	}
}

syscall kputc(uchar c)
{
    volatile struct ns16550_uart_csreg *regptr;
    regptr = (struct ns16550_uart_csreg *)UART_BASE;
    
    //busy wait while both lsr and thre are equal to 0 
    while((regptr -> lsr & UART_LSR_THRE) == 0){
    }

    //assign the next character to the transmit holding register
    regptr -> thr = c;
    
    return c; 
}

syscall kprintf(const char *format, ...)
{
    int retval;
    va_list ap;

    va_start(ap, format);
    retval = _doprnt(format, ap, (int (*)(long, long))kputc, 0);
    va_end(ap);
    return retval;
}
