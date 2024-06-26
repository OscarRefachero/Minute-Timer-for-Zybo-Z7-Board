#include "xparameters.h"
#include <stdio.h>
//#include "platform.h"
#include "xscugic.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xscutimer.h"
#include "led_ip.h"

//====================================================

XGpio output, dip, push;
int i, psb_check, dip_check, leftDigit, rightDigit;
XScuTimer Timer;
void displayVal(int, XGpio);
void delay(int);
int getAddr(int, int, XGpio);
static int Init_Peripherals();
static void Push_Intr_Handler(void *CallBackRef);
void getPeripheralStatus();
void init_Button_Int();
XScuGic IntcInstance;		/* Interrupt Controller Instance */
XScuGic *IntcInstancePtr = &IntcInstance;
XScuGic_Config *IntcConfig;

static int Init_Peripherals()
{
	int pResult = XGpio_Initialize(&output, XPAR_SWITCHES_BASEADDR);
	if(pResult != XST_SUCCESS) {
		xil_printf("Failed to initialize Seven Segment Display\r\n");
		return XST_FAILURE;
	}

	pResult = XGpio_Initialize(&dip, XPAR_SWITCHES_DEVICE_ID);
	if(pResult != XST_SUCCESS) {
		xil_printf("Failed to initialize Switches\r\n");
		return XST_FAILURE;
	}
	XGpio_SetDataDirection(&dip, 1, 0xffffffff);

	pResult = XGpio_Initialize(&push, XPAR_BUTTONS_DEVICE_ID);
	if(pResult != XST_SUCCESS) {
		xil_printf("Failed to initialize Buttons\r\n");
		return XST_FAILURE;
	}
	XGpio_SetDataDirection(&push, 1, 0xffffffff);

   return XST_SUCCESS;
}

void init_Button_Int() { //enables button interrupt service routine
	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	XScuGic_SetPriorityTriggerType(IntcInstancePtr, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR, 0xA0, 0x3);
	XScuGic_Connect(IntcInstancePtr, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR, (Xil_ExceptionHandler)Push_Intr_Handler, (void *)&push);
	XScuGic_Enable(IntcInstancePtr, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR);
	XScuGic_CPUWriteReg(IntcInstancePtr, 0x0, 0xf);
	XGpio_InterruptEnable(&push, 0xF);
	XGpio_InterruptGlobalEnable(&push);
}

static void Push_Intr_Handler(void *CallBackRef)
{
	XGpio *push_ptr = (XGpio *) CallBackRef;

	//xil_printf("Insider Button ISR! ... \r\n");

	// Disable GPIO interrupts
	XGpio_InterruptDisable(push_ptr, 0xF);

	//xil_printf("Here again ... \r\n");

	LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0xAAAA);

    (void)XGpio_InterruptClear(&push, 0xF);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&push, 0xF);
}

void displayVal(int i, XGpio o) {
	int leftDigit = i/10;
	int rightDigit = i % 10;

	if(i > 9) {
		for(int h = 0; h < 2000000; h++) {
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
	if(i < 0 || i > 9 ) { xil_printf("Error\n\r"); return 0; }
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

void getPeripheralStatus() {
	psb_check = XGpio_DiscreteRead(&push, 1); //button
	dip_check = XGpio_DiscreteRead(&dip, 1); //switch
	LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, dip_check);
}

int main (void)
{
   Init_Peripherals();
   xil_printf("Peripherals Initialized Successfully...\r\n");

   init_Button_Int();
   xil_printf("Button ISR Initialized...\r\n");

   xil_printf("-- Start of the Program --\r\n");
   displayVal(i, output);

   int j = 0; //Time elapsed and value to be outputted to S.S.D

   while (1)
   {
	  getPeripheralStatus();

	  if(psb_check == 8) { xil_printf("Exiting Program\n\r"); break; }

	  if(psb_check == 4) {
		  xil_printf("Resetting timer...\n\r");
		  j = 0;
		  displayVal(j, output);
	  }

	  if(dip_check == 8 && j < 60) {
		  xil_printf("Starting Count Up Sequence...\n\r");
		  //countUp function
		  while(j < 60 && dip_check == 8) {
			  ++j;
			  getPeripheralStatus();
		      displayVal(j, output);
		  }
	  }
	  if(dip_check == 4 && j > 0) {
		  xil_printf("Starting Count Down Sequence...\n\r");
		  //countDown function
		  while(j > 0 && dip_check == 4) {
			  --j;
			  getPeripheralStatus();
			  displayVal(j, output);
		  }
	  }

	  while((dip_check == 12 || dip_check == 0) && j > 9 && psb_check == 0) {
		  getPeripheralStatus();
		  getAddr(j/10, 1, output);
		  getAddr(j%10, 0, output);
	  }

	  for (i=0; i<9999999; i++);
   }
   //disables interrupt
   XGpio_InterruptDisable(&push, 0x3);
   XScuGic_Disable(IntcInstancePtr, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR);
   XScuGic_Disconnect(IntcInstancePtr, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR);
}
