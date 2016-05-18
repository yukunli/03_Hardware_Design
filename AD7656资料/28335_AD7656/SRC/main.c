//在定时器内进行采样,采样率1.5KHz
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#define	  AD7656_BASIC    (*((volatile  Uint16 *)0x200000))
//#define	  AD7656_rst      (*((volatile  Uint16 *)0x200008))
#define   AD_BUSY         GpioDataRegs.GPBDAT.bit.GPIO60
#define   SET_ADRST       GpioDataRegs.GPBSET.bit.GPIO61=1
#define   CLEAR_ADRST     GpioDataRegs.GPBCLEAR.bit.GPIO61=1
#define   SET_ADCLK       GpioDataRegs.GPASET.bit.GPIO25=1
#define   CLR_ADCLK       GpioDataRegs.GPACLEAR.bit.GPIO25=1
Uint16 ad[6];
void InitXintf(void);
//int test[256];
long sampleCount=0;
Uint16 testSample1[256],testSample2[256],testSample3[256],//6个数组存放AD数据
       testSample4[256],testSample5[256],testSample6[256]; 
int i;
interrupt void ISRTimer0(void);

void ADInit(void)
{
	EALLOW;
	GpioCtrlRegs.GPAMUX2.bit.GPIO25=0;
	GpioCtrlRegs.GPADIR.bit.GPIO25=1;
	GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 0;  // XWE0
	GpioCtrlRegs.GPBDIR.bit.GPIO61=1;
	GpioCtrlRegs.GPBMUX2.bit.GPIO60=0; //ADBUSY
	GpioCtrlRegs.GPBDIR.bit.GPIO60=0;//INPUT
	GpioCtrlRegs.GPBQSEL2.bit.GPIO60= 0;
	EDIS;
} 

void main(void)
{ 

	InitSysCtrl();
	InitXintf();
	InitXintf16Gpio();
	ADInit();
	DINT;
	InitPieCtrl();
	IER = 0x0000;
	IFR = 0x0000;
	InitPieVectTable();
   EALLOW;  // This is needed to write to EALLOW protected registers
   PieVectTable.TINT0 = &ISRTimer0;
   EDIS;    // This is needed to disable write to EALLOW protected registers
   InitCpuTimers();   // For this example, only initialize the Cpu Timers
   ConfigCpuTimer(&CpuTimer0, 100, 987); //在定时器内进行采样,采样率1.5KHz
	
    IER |= M_INT1;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; 
	EINT; 
	ERTM;  
/*EALLOW;
GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0; // GPIO0 = GPIO0
GpioCtrlRegs.GPADIR.bit.GPIO0 = 1; 
EDIS;
GpioDataRegs.GPADAT.bit.GPIO0 = 0;*/
	SET_ADRST;  
	DELAY_US(100000);
	CLEAR_ADRST; 
	StartCpuTimer0();
	while(1);

}

interrupt void ISRTimer0(void)
{   	   	
   	CLR_ADCLK;   //启动转换信号
	DELAY_US(1); //给予适当的电平延时
	SET_ADCLK;
	DELAY_US(1);
	//GpioDataRegs.GPADAT.bit.GPIO0 = ~GpioDataRegs.GPADAT.bit.GPIO0;
		while(AD_BUSY) //等待转换结束
		{
			
		}

	    for(i=0;i<6;i++) 
		{
	        ad[i]=AD7656_BASIC; //读取6路AD通道数据
		}
		testSample1[sampleCount]=ad[0];//存放在数组里
		testSample2[sampleCount]=ad[1];
		testSample3[sampleCount]=ad[2];
		testSample4[sampleCount]=ad[3];
		testSample5[sampleCount]=ad[4];
		testSample6[sampleCount]=ad[5]; 
		sampleCount++;
		if(sampleCount >= 255) sampleCount=0;

	   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
    CpuTimer0Regs.TCR.bit.TIF=1; // 定时到了指定时间，标志位置位，清除标志      
    CpuTimer0Regs.TCR.bit.TRB=1;  // 重载Timer0的定时数据
} 

