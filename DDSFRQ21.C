#include "pic.h"
//#include "delay.c"
#include "float.h"

#define F_CPU 32000000


//Wait are these actaully commented out? They probably need to be changed anyway.
//***** configuration fuses
// int oscillator, code protect off, eeprom code protect off
//__CONFIG(FOSC2|CPD|CP0|CP1|0x0400); 
//__CONFIG(LVPDIS & INTIO & MCLRDIS & BOREN & WDTDIS & DATUNPROT);

#define TRUE    1
#define FALSE   0

// relay definitions
#define SCLK_HIGH       RA1=TRUE                //set ad9835 sclk line
#define SCLK_LOW        RA1=FALSE
#define SDATA_HIGH      RA2=TRUE                //set ad9835 sdata line
#define SDATA_LOW       RA2=FALSE
#define FSYNC_HIGH      RA3=TRUE                //set ad9835 fsync line
#define FSYNC_LOW       RA3=FALSE
#define SHIFT_BUTTON    RB0                     //SHIFT button
#define INC_BUTTON      RA6                     //INC button
#define SET_BUTTON      RA7                     //SET button
#define LCDSEG_A        RB1
#define LCDSEG_B        RB2
#define LCDSEG_C        RB3
#define LCDSEG_D        RB4
#define LCDSEG_E        RB5
#define LCDSEG_F        RB6
#define LCDSEG_G        RB7
#define LED1MHz         RB7
#define LED100KHz       RB6
#define LED10KHz        RB5
#define LED1KHz         RB4
#define LED100Hz        RB3
#define LED10Hz         RB2
#define LED1Hz          RB1
#define LEDsink         RA4
#define SEGsink         RA0

//global variables
	unsigned long int D1MHz;
	unsigned long int D100KHz;
	unsigned long int D10KHz;
	unsigned long int D1KHz;
	unsigned long int D100Hz;
	unsigned long int D10Hz;
	unsigned long int D1Hz;

//*****************************************************************
//SEND THE FREQUENCY WORD TO THE DDS CHIP
void SendWordDDS(unsigned int ddsword){
	unsigned int tw;
	SCLK_HIGH; 
	FSYNC_HIGH; 
	FSYNC_LOW;
	tw=ddsword;
	if((tw&32768)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&16384)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&8192)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&4096)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&2048)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&1024)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&512)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&256)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&128)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&64)==0) SDATA_LOW; else  SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&32)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&16)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&8)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&4)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&2)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;

	tw=ddsword;
	if((tw&1)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	FSYNC_HIGH;             //end 16bit word
}

//*****************************************************************
// sends the 32bit word passed, to the FREQ0 Reg of the 9835
void SendFreqRegDDS(unsigned long int fv){
	unsigned int word;
	unsigned long int t;
	t=fv;
	t=t&0x000000ff;
	word=(unsigned int)(0x3000+(t));        //get 8L LSB with mask
	SendWordDDS(word);
	t=fv;
	t=t&0x0000ff00;
	word=(unsigned int)(0x2100+(t>>=8));    //get 8H LSB with mask
	SendWordDDS(word);
	t=fv;
	t=t&0x00ff0000;
	word=(unsigned int)(0x3200+(t>>=16));   //get 8L MSB with mask
	SendWordDDS(word);
	t=fv;
	t=t&0xff000000;
	word=(unsigned int)(0x2300+(t>>=24));   //get 8H MSB with mask
	SendWordDDS(word);
}

//*****************************************************************
// converts a frequency in Hz to the 9835 register value
unsigned long int ConvertFrequency(unsigned long int temp){
	unsigned long int c;
	c=(unsigned long int)(temp/11.64153218e-3);
	return(c);
}

//*****************************************************************
//saves the current frequency to EEPROM
void SaveFreq(void){
	eeprom_write(1,D1MHz);
	eeprom_write(2,D100KHz);
	eeprom_write(3,D10KHz);
	eeprom_write(4,D1KHz);
	eeprom_write(5,D100Hz);
	eeprom_write(6,D10Hz);
	eeprom_write(7,D1Hz);
}

//*****************************************************************
//loads the frequency from the EEPROM
void LoadFreq(void){
	D1MHz=eeprom_read(1);
	if (D1MHz>9) D1MHz=0;
	D100KHz=eeprom_read(2);
	if (D100KHz>9) D100KHz=0;
	D10KHz=eeprom_read(3);
	if (D10KHz>9) D10KHz=0;
	D1KHz=eeprom_read(4);
	if (D1KHz>9) D1KHz=0;
	D100Hz=eeprom_read(5);
	if (D100Hz>9) D100Hz=0;
	D10Hz=eeprom_read(6);
	if (D10Hz>9) D10Hz=0;
	D1Hz=eeprom_read(7);
	if (D1Hz>9) D1Hz=0;
}

//*****************************************************************
//update DDS frequency
void UpdateDDS(void){
	unsigned long int freqreg;
	unsigned long int freq;         //signal frequency
	freq=0;                 //convert digits to frequency
	freq=freq+D1Hz;
	freq=freq+(10*D10Hz);
	freq=freq+(100*D100Hz);
	freq=freq+(1000*D1KHz);
	freq=freq+(10000*D10KHz);
	freq=freq+(100000*D100KHz);
	freq=freq+(1000000*D1MHz);
		
	freqreg=ConvertFrequency(freq); //convert to register value
	SendFreqRegDDS(freqreg);        //send to DDS chip
}

//*****************************************************************
// key bounce delay of a few hundred ms while displaying LED
void KeyDelay(void) {
	int i;
	for(i=0;i<20;i++){
		NOP();
	}
}

void delay_ms(unsigned int delay_length) {
	for(int i=0; i<delay_length; i++){
		//Do something that takes one ms.
		for(unsigned int j=0; j<F_CPU; j++){
			NOP();
		}
	}
}	

//*****************************************************************
// send the required commands to initialise the 9835
void InitialiseDDS(void){
	SendWordDDS(0xF800);
	SendWordDDS(0xB000);
	SendWordDDS(0x5000);
	SendWordDDS(0x4000);
	SendWordDDS(0x1800);	//clear phase register
	SendWordDDS(0x0900);
	SendFreqRegDDS(ConvertFrequency(1000));	//initial frequency
	SendWordDDS(0xC000);
}

//*****************************************************************
// sweep those frequencies bitch!
void DDSSweep(unsigned int start_freq, unsigned int end_freq, unsigned char step_size, unsigned int step_delay){
	char direction = 0; //0=down 1=up;
	unsigned int sweep_length;
	unsigned int current_freq = start_freq;
	

	SendFreqRegDDS(ConvertFrequency(current_freq)); //set start freq
	
	if(start_freq > end_freq){
		while(current_freq >= end_freq){
			SendFreqRegDDS(ConvertFrequency(current_freq));
			current_freq -= step_size;
			delay_ms(step_delay);
		}
	}else if(start_freq < end_freq){	
		while(current_freq <= end_freq){
			SendFreqRegDDS(ConvertFrequency(current_freq));
			current_freq += step_size;
			delay_ms(step_delay);
		}
	}else
		return;
}

//*****************************************************************
void main(void){
	//CMCON=0b00000111;               //disable comparator
	PORTA=0;
	PORTB=0;
	TRISA=0;                        //PORTA are all outputs
	TRISB=0;                        //PORTB are all outputs
	TRISB0=TRUE;                    //switch input
	TRISA7=TRUE;                    //switch input
	TRISA6=TRUE;                    //switch input
	//RBPU=FALSE;                     //enable portb pullups
	GIE=FALSE;                      //disable all interrupts

	InitialiseDDS();
	
	LoadFreq();                     //recover saved frequency
	UpdateDDS();
			
	while(TRUE){
	DDSSweep(10,1000000,100,10000); //Sweep Up
	DDSSweep(1000000,10,100,10000); //Sweep Down
	}
}
