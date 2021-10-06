
/* Define global variables */
struct {
	int row;
	int column;
	unsigned char event;
	const unsigned char row1[4];
  const unsigned char row2[4];
  const unsigned char row3[4];
  const unsigned char row4[4];
  const unsigned char* keys[];
} typedef keypad;


keypad keypad1 = {
  .row = ~0,  
  .column = ~0, 
  .event = 0,
  .row1 = {1, 2, 3, 0xA},
  .row2 = {4, 5, 6, 0xB},
  .row3 = {7, 8, 9, 0xC},
  .row4 = {0xE, 0, 0xF, 0xD},
  .keys = {keypad1.row1, keypad1.row2, keypad1.row3, keypad1.row4},
};

/*---------------------------------------------------*/
/* Initialize GPIO pins used in the program */
/* PB0 triggers interrupt EXTI1 */	
/* PB[6-3] = counter output */ 
/* PA[5-2] = keypad rows */  
/* PA[8-11] = keypad columns */	
/*---------------------------------------------------*/
void PinSetup() {
	
	/* Configure PA11-8, PA5-2 as inputs */
	RCC->AHBENR |= 0x01; // Enable GPIOA clock (bit 1)
	GPIOB->MODER &= ~(0x00FF0FF0);
	GPIOB->MODER |= (0x00550000); // Write to columns and read rows
	GPIOB->ODR = 0;
	
	GPIOB->PUPDR &= ~(0x00000FF0);
	GPIOB->PUPDR |= (0x00000550);  //Write rows and read columns
	
	/* Configure PB[6:3]=01 as output pins to disply value of counter 
	   and PB0=00 as input */
	RCC->AHBENR |= 0x02; // Enable GPIOB clock (bit 1)
	GPIOC->MODER &= ~(0x00003FC3); // Clear PB6-3, PB0 mode bits
	GPIOC->MODER |= (0x00001540); // General purpose output mode
}
/*----------------------------------------------------------*/
/* Output value to LEDs */
/*----------------------------------------------------------*/
void updateLEDs(unsigned char count) {
	
		GPIOB->BSRR |= (~count & 0x007E) << 16; //reset bits
		GPIOB->BSRR |= (count & 0x007E); //set bits
		/* GPIOB->ODR &= 0x0000;
		GPIOB->ODR |= count << 3; */
		/* GPIOB->ODR = (GPIOA->ODR & ~(0x007E)) | (count << 3); */
}

/*----------------------------------------------------------*/
/* Delay function - do nothing for about 1 second */
/*----------------------------------------------------------*/
void delay () {
	/* if (keypad1.event) {
	keypad1.event = keypad1.event - 1;
	} */
	int i,j,n;
	for (i=0; i<20; i++) { //outer loop
		for (j=0; j<20000; j++) { //inner loop
			n = j; //dummy operation for single-step test
		} //do nothing
	}
}

/*----------------------------------------------------------*/
/* SmallDelay function - do nothing for a short period */
/*----------------------------------------------------------*/
void smallDelay() {
  int i, j;
  for (i=0; i<4; i++) { //outer loop
    i = j;; //dummy operation for single-step test
  }
}

/*------------------------------------------------*/
/* Determine row number of pressed key */
/*------------------------------------------------*/
int readRow() {
	GPIOA->MODER &= ~(0x00FF0FF0);
	GPIOA->MODER |= (0x00550000);
	GPIOA->ODR = 0;
	GPIOA->PUPDR &= ~(0x00000FF0);
	GPIOA->PUPDR |= (0x00000550);
	
	smallDelay();
		
	int input = GPIOA->IDR&0x003C;
	switch(input) {
		case 0xE:
			return 0;
		case 0xD:
			return 1;
		case 0xB:
			return 2;
		case 0x7:
			return 3;
		default:
			return -1;
	}
}

/*------------------------------------------------*/
/* Determine column number of pressed key */
/*------------------------------------------------*/
int readColumn() {
	GPIOA->MODER &= ~(0x00FF0FF0);
	GPIOA->MODER |= (0x00000550);
	GPIOA->ODR = 0;
	GPIOA->PUPDR &= ~(0x00FF0000);
	GPIOA->PUPDR |= (0x00550000);

	smallDelay();
	
	int input = GPIOA->IDR&0x0F00;
	switch(input) {
		case 0xE0:
			return 0;
		case 0xD0:
			return 1;
		case 0xB0:
			return 2;
		case 0x70:
			return 3;
		default:
			return -1;
	}
}

/*----------------------------------------------------------*/
/* EXTI1_IRQHandler function - performs operations when EXTI1 is triggered */
/*----------------------------------------------------------*/
void EXTI1_IRQHandler() {
	EXTI->PR1 |= 0x0002; //Set pending register for EXTI1

	keypad1.row = readRow();
	keypad1.column = readColumn();
			
			if ((keypad1.row != -1) && (keypad1.column != -1)) {
				keypad1.event = 4;
				updateLEDs(keypad1.keys[keypad1.row][keypad1.column]);
			}
	
	RCC->AHBENR |= 0x01; // Enable GPIOA clock (bit 0)
	GPIOA->MODER &= ~(0x00FF0FF0);
	GPIOA->PUPDR &= ~(0x00FF0000);
	GPIOA->PUPDR |= (0x00550000);
			
	GPIOA->MODER &= ~(0x00FF0FF0);
	GPIOA->MODER |= (0x00000550);
	GPIOA->ODR = 0;
	GPIOA->PUPDR &= ~(0x00000FF0);
	GPIOA->PUPDR |= (0x00000550);		
	NVIC_ClearPendingIRQ(EXTI1_IRQn); //Reset pending register for EXTI1
	EXTI->PR1 |= 0x0002;
}

/*------------------------------------------------*/
/* Main program */
/*------------------------------------------------*/
int main(void) {
	
	PinSetup(); // Configure GPIO 
	InterruptSetup(); // Configure interrupts
	
	unsigned char count = 0;
	
	__enable_irq(); //Enable interrupts to occur
	
	/* Endless loop */
	while (1) {
		
		delay();
		
		if (count == 9) { 
			count = 0;   // Sets Count to 0 if currently at 9
		}
		else {
			count = count + 1;  // Increments Count
		}
		
		if (keypad1.event == 0) {
			updateLEDs(count);
		}
		
		
	} /* repeat forever */
}
