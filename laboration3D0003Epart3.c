#include "tinythreads.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/iom169p.h>
#define true	1
#define false	0

//mutex mute = MUTEX_INIT;		// Initialize the mutex.

int key_press;					// Global Variable, number of times the key have been pressed down.

/*
mutex mute_button = MUTEX_INIT;			// Alt #2.
mutex mute_blink = MUTEX_INIT;			// Alt #2.
*/

int ASCII_TABLE[10] = 
{
		0x1551,		// 0 = 1551, A = 0F51
		0x0110,		// 1
		0x1e11,		// 2
		0x1B11,		// 3
		0x0B50,		// 4
		0x1B41,		// 5
		0x1F41,		// 6
		0x0111,		// 7
		0x1F51,		// 8
		0x0B51		// 9
};
void writeChar(char ch, int pos){
	/* ch = character we want to write, pos = position on the LCD: 0->5 digits. */
	char mask;
	int karaktar = 0x0000;
	char nibbles = 0x00;
	int x;
	char lcddr = 0xec;

	int bokstav = atoi(&ch); /* Converts ch, which the variable is at the form 'x' and not x, into an int! */

	if(pos > 5){
		return;
	}
	if (pos & 0x01){
        mask = 0x0F;                /* Position 1, 3, 5 */
	}		
    else{
        mask = 0xF0;                /* Position 0, 2, 4  */
	}
	if(bokstav <= 9 && bokstav >= 0){
	karaktar = ASCII_TABLE[(int)bokstav]; 
	}	
	else{
	karaktar = 0x0000;
	}	
	lcddr  += (pos >>1); /* (0,0,1,1,2,2) (LCDDR0, LCDDR0) (LCDDR1, LCDDR1) (LCDDR2, LCDDR2) */

	for(x = 0; x < 4; x++){
		nibbles = karaktar & 0x000f;
		karaktar = karaktar >> 4;
		if(pos & 0x01){					/* True om 1,3,5 */
			nibbles = nibbles << 4;
		}
		_SFR_MEM8(lcddr) = ((_SFR_MEM8(lcddr) & mask) | nibbles);
		lcddr += 5;
		}	
}
void computePrimes(int pos) {
    long n;
    for(n = 1; ; n++) {
        if (is_prime(n)) {
            printAt(n, pos);
        }
    }
}
int is_prime(long i){
	/*	Checks if it's a prime number.
	*	Done by computing i % n, 
	*	(i.e., the remainder from division i/n) for all 2 <= n < i, 
	*	and returning false (0) if any such expression is 0, 
	*	true (1) otherwise.
	*/
	int start;
	for(start = 2; start < i; start++){
		if ((i % start) == 0) {
			return false;
		}		
	}
	return true;
}
void printAt(long num, int pos) {
	int pp;
	pp = pos;
    writeChar( (num % 100) / 10 + '0', pp);
    pp++;
    writeChar( num % 10 + '0', pp);
}

void buttonCount(){
	printAt(key_press, 3);
}

void blink3(){
	if(LCDDR3 != 0){		//LCDDR0, gammalt värde
		LCDDR3 = 0x0;		//LCDDR0 = 0x00; , gammalt värde
		TCNT1 = 0x0000;
	}
	else{
		LCDDR3 = 0x1;		//LCDDR0 = 0x04; , gammalt värde
		TCNT1 = 0x0000;
	}
}

int main() {
	// Clock prescaler
	CLKPR = 0x80;	//(1<<CLKPCE);
	CLKPR =	0x00;	//(0<<CLKPS3);
	// Device Initialization values:
	LCDCRA = 0xC0;	//(1<<LCDEN)  | (1<<LCDAB);						//0xC0;
	LCDCRB = 0xB7;	//(1<<LCDCS)  | (3<<LCDMUX0) | (7<<LCDPM0);		//0xB7;
	LCDFRR = 0x07;	//(0<<LCDPS0) | (7<<LCDCD0);					//0x07;
	LCDCCR = 0x0F;	//(15<<LCDCC0);									//0x0F;
	
	PORTB = (1 << PORTB7);		// Kolla
	EIMSK = 0x80;				// Enables Interrupts from PCINT15-8.
	PCMSK1 = 0x80;				// Enables Interrupts on pin PCINT15.
	TCCR1B = 0x0D;				// Clock prescaler set to 1024 and CFC.
	TCNT1 = 0x0;				// Clear register.
	TIMSK1 = 0x02;				// Timer output compare A interrupt
	//OCR1A = 0x186;				// 8 MHz / 1024/20 -> 7812,5/20 = 390,625 cycles = 50ms
	OCR1A = 0xF42;				//3906 Dec. (390,625cycles * 10 = 500ms)

	//printAt(key_press, 3);
	
	spawn(blink3, 0);				// Alt #2.
	spawn(buttonCount, 0);			// Alt #2.
	computePrimes(0);				// Alt #1-2.
	//printAt(key_press,3);
}

ISR(PCINT1_vect){
	if((PINB >> 7) == 0){
		key_press++;
		//printAt(key_press,3);
		//unlock(&mute_button);			// Alt #2.
		spawn(buttonCount,3);
		yield();
	}
}
ISR(TIMER1_COMPA_vect){
	//nr_interrupts++;					// Alt #1.
	//key_press++;
	//printAt(key_press,3);
	//unlock(&mute_blink);				// Alt #2.
	spawn(blink3, 0);					// Alt #3.
	yield();
}