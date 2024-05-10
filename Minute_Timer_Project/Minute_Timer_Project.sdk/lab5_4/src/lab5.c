#include "xparameters.h"
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xscutimer.h"
#include "led_ip.h"

//====================================================

XGpio output;
int i = 0;
XScuTimer Timer;
void displayVal(int, XGpio);
void delay(int);
int getAddr(int, int, XGpio);
void countUp();
void countDown();

void countUp() {
	while(i < 60) {
		displayVal(i, output);
		i++;
	}
}

void countDown() {
	while(i >= 0) {
		displayVal(i, output);
		i--;
	}
}

void displayVal(int i, XGpio o) {
	int leftDigit = i/10;
	int rightDigit = i % 10;

	if(i > 9) {
		for(int h = 0; h < 1300000; h++) {
			getAddr(leftDigit, 1, o);
			getAddr(rightDigit, 0, o);
		}
	}
	else {
		getAddr(rightDigit, 0, o);
		delay(50000000);
	}
}

int getAddr(int i, int r, XGpio o) {
	if(i < 0 || i > 9 ) { print("Error"); return 0; }
	u32 baseAddr = 0b00000000;
	if(r == 1) baseAddr |= 0b10000000;
	if(i == 0) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b00111111);
	}
	else if(i == 1) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b00000110);
	}
	else if(i == 2) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b01011011);
	}
	else if(i == 3) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b01001111);
	}
	else if(i == 4) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b01100110);
	}
	else if(i == 5) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b01101101);
	}
	else if(i == 6) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b01111101);
	}
	else if(i == 7) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b00000111);
	}
	else if(i == 8) {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b01111111);
	}
	else {
		XGpio_DiscreteWrite(&o, 1, baseAddr | 0b01100111);
	}
	return 1;
}

void delay(int t) {
	for(int i = 0; i < t*2; i++) {}
}

int main (void) 
{

   XGpio dip, push;
   int psb_check, dip_check;
   XGpio_Initialize(&output, 0x41200000);
	
   xil_printf("-- Start of the Program --\r\n");
 
   XGpio_Initialize(&dip, XPAR_SWITCHES_DEVICE_ID); // Modify this
   XGpio_SetDataDirection(&dip, 1, 0xffffffff);
	
   XGpio_Initialize(&push, XPAR_BUTTONS_DEVICE_ID); // Modify this
   XGpio_SetDataDirection(&push, 1, 0xffffffff);
	

   while (1)
   {
	  psb_check = XGpio_DiscreteRead(&push, 1);
	  xil_printf("Push Buttons Status %x\r\n", psb_check);
	  dip_check = XGpio_DiscreteRead(&dip, 1);
	  xil_printf("DIP Switch Status %x\r\n", dip_check);
	  
	  // output dip switches value on LED_ip device
	  LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, dip_check);
	  
	  for (i=0; i<9999999; i++);
   }
}