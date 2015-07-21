#include "main.h"

int main(void)  
{  
	WDTCTL = WDTPW + WDTHOLD;                     //关闭看门狗
	Board_init();                                 //初始化GPIO
	Init_Clock();
	SFRIFG1 = 0;                                 //清中断标志
	SFRIE1 |= OFIE;                              //使能晶振失效中断
	__enable_interrupt();                        //使能全局中断
	Dogs102x6_init();                            //初始化LCD
	Dogs102x6_backlightInit();                   //背光初始化
	Dogs102x6_clearScreen();                     //清屏
	Dogs102x6_setBacklight(11);          //设置初始背光值
	Dogs102x6_setContrast(11);             //设置初始对比度值
	Dogs102x6_clearScreen();
	//Buttons Init
	Buttons_init(BUTTON_ALL);                    //初始化按键
	Buttons_interruptEnable(BUTTON_ALL);         //使能所有按键中断
	buttonsPressed = 0;                          //键值清零
	Dogs102x6_imageDraw(tiBug, 0, 16);           //显示TI图案
    while (!buttonsPressed)                     //等待按键被按下，或者超时退出等待
    {
        for (timeoutCounter = 0; timeoutCounter < 0xFFFF; timeoutCounter++)
        {
            if (buttonsPressed)
                break;
            __delay_cycles(300);
        }

        //Timeout break
        break;                                   //超时退出
    }
    Dogs102x6_clearScreen();
    buttonsPressed = 0;
    //显示文字说明：
    Dogs102x6_stringDraw(2, 0, "   Welcome to    ", DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(3, 0, " USE DIGITAL ECG ", DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(5, 0, "Wait for a moment", DOGS102x6_DRAW_INVERT);
    Dogs102x6_stringDraw(6, 0, "or press S1 | S2 ", DOGS102x6_DRAW_INVERT);
    while (!buttonsPressed)                    //等待按键被按下，或者超时退出等待
    {
        for (timeoutCounter = 0; timeoutCounter < 0x9FFF; timeoutCounter++)
        {
            if (buttonsPressed)
                break;
            __delay_cycles(600);
        }
        break;
    }
	Dogs102x6_clearScreen();
    buttonsPressed = 0;
	//	delay_ms(800);
	I2C();
	Init_TA1();
    Init_ADC12();
    TA1CCTL0 = CCIE;                            
    TA1CCR0 =16000;   
    TA1CTL = TASSEL_2 + MC_1 + TACLR;            
    _EINT();                                   

    P1DIR |= (BIT1 + BIT2 + BIT3);
	while(1)
    {  
    	//delay_ms(2);
		Test();
    	//delay_ms(2);
    }
}





void Select_XT1(void)
{
    // 启动XT1
    P7SEL |= 0x03;
    UCSCTL6 &= ~(XT1OFF);
    UCSCTL6 |= XCAP_3;
    do
    {
        UCSCTL7 &= ~XT1LFOFFG;
    }
    while (UCSCTL7&XT1LFOFFG);
}
void Dco_Config(void)
{
    __bis_SR_register(SCG0);
    UCSCTL0 = 0x0000;
    UCSCTL1 = DCORSEL_5;
    UCSCTL2 = FLLD_1 + 243;

    __bic_SR_register(SCG0);                    // 使能FLL功能
    __delay_cycles(250000);

    do
    {
    	UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + 0x0004 + DCOFFG);
        SFRIFG1 &= ~OFIFG;
    }
    while (SFRIFG1&OFIFG);
}
void Init_Clock(void)
{
    Select_XT1();                               // 选择XT1
    Dco_Config();
}
void Init_ADC12(void)
{
	P7SEL |= BIT0 + BIT1;
	ADC12CTL0 &=~ ADC12ENC;
	ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_0;
	ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1 + ADC12SSEL1 ;//multi channel single convert
	ADC12MCTL0 = ADC12INCH_12;  //ADC input pin 7.0
	ADC12MCTL1 = ADC12INCH_13 + ADC12EOS;  //ADC input pin 7.1
	ADC12IE = ADC12IE1;
	ADC12CTL0 |= ADC12ENC;
}
void Init_TA1()
{
    TA1CCTL0 = CCIE;
    TA1CCR0 =16000;
    TA1CTL = TASSEL_2 + MC_1 + TACLR;
}
void Test(void)
{
//	ShowMenu();
	if(buttonsPressed&BUTTON_S1)
	{
		HeartRate();
	}
	else if(buttonsPressed&BUTTON_S2)
	{
		ECG();
	}
	else
	{
		ShowMenu();
	}
}

void HeartRate(void)
{
    buttonsPressed = 0;
    Dogs102x6_clearScreen();
	while(!buttonsPressed)
	{
		Dogs102x6_stringDraw(0, 0, "Pulse Rate:", DOGS102x6_DRAW_NORMAL);
		if(i==102)
		{
			i=0;
			Dogs102x6_clearScreen();
		}
		k++;
		if(k==7)
		{
			//Dogs102x6_pixelDraw(i,63-Signal/32 , DOGS102x6_DRAW_NORMAL);
			Dogs102x6_pixelDraw(i,Signal/12 , DOGS102x6_DRAW_NORMAL);
			i++;k=0;
		}
		if (QS == true)
		{
			delay_ms(5);
			char str1[5];
			str1[0]=BPM/1000+48;
			str1[1]=BPM/100%10+48;
			str1[2]=BPM/10%10+48;
			str1[3]=BPM%10+48;
			str1[4]='\0';
			Dogs102x6_stringDraw(0, 60, str1, DOGS102x6_DRAW_NORMAL);
			Dogs102x6_stringDraw(0, 85, "BMP", DOGS102x6_DRAW_NORMAL);
			P1OUT ^= (BIT1 + BIT2 + BIT3);
			QS = false;
		}
	}
    buttonsPressed = 0;
    P1OUT ^= BIT1;
    P1OUT ^= BIT2;
    P1OUT ^= BIT3;
    Dogs102x6_clearScreen();
}
void ECG(void)
{
	buttonsPressed = 0;
	Dogs102x6_clearScreen();
	while(!buttonsPressed)
	{
		Dogs102x6_stringDraw(0, 0, "ECG Graph:", DOGS102x6_DRAW_NORMAL);
		char str1[5];
		str1[0]=ADCCon2/1000+48;
		str1[1]=ADCCon2/100%10+48;
		str1[2]=ADCCon2/10%10+48;
		str1[3]=ADCCon2%10+48;
		str1[4]='\0';
		Dogs102x6_stringDraw(0, 60, str1, DOGS102x6_DRAW_NORMAL);
		if(i==102)
		{
			i=0;
			Dogs102x6_clearScreen();
		}
		k++;
		if(k==7)
		{
			//Dogs102x6_pixelDraw(i,63-Signal/32 , DOGS102x6_DRAW_NORMAL);
			Dogs102x6_pixelDraw(i,ADCCon2/48 , DOGS102x6_DRAW_NORMAL);
			i++;k=0;
		}
	}
	buttonsPressed = 0;
	Dogs102x6_clearScreen();
}
void ShowMenu()
{
	I2C();
	ReadTemp();
	int temp = g_objectTemp * 100;
	char str[10];
	str[0]=temp/1000+48;
	str[1]=temp/100%10+48;
	str[2]='.';
	str[3]=temp/10%10+48;
	str[4]=temp%10+48;
	str[5]='^';
	str[6]='C';
	str[7]='\0';
	Dogs102x6_stringDraw(0, 0, "BodyTemp:", DOGS102x6_DRAW_NORMAL);
	Dogs102x6_stringDraw(1, 60, str, DOGS102x6_DRAW_NORMAL);
	I2C();
	ReadAmbi();
	temp = g_ambientTemp * 100;
	str[0]=temp/1000+48;
	str[1]=temp/100%10+48;
	str[2]='.';
	str[3]=temp/10%10+48;
	str[4]=temp%10+48;
	str[5]='^';
	str[6]='C';
	str[7]='\0';
	Dogs102x6_stringDraw(2, 0, "SensorTemp:", DOGS102x6_DRAW_NORMAL);
	Dogs102x6_stringDraw(3, 60, str, DOGS102x6_DRAW_NORMAL);
	Dogs102x6_stringDraw(5, 0, "Press to select", DOGS102x6_DRAW_NORMAL);
	Dogs102x6_stringDraw(6, 0, "S1=Pulse Rate", DOGS102x6_DRAW_NORMAL);
	Dogs102x6_stringDraw(7, 0, "S2=ECG", DOGS102x6_DRAW_NORMAL);
}


#pragma vector = ADC12_VECTOR
__interrupt void  ADC10(void)
{ 
    Signal = ADC12MEM0 / 4;
    ADCCon2 = ADC12MEM1;
}


#pragma vector=TIMER1_A0_VECTOR  
__interrupt void TIMER1_A0_ISR(void)  
{
	unsigned int runningTotal = 0;
	unsigned char i =0;
	ADC12CTL0 |= ADC12SC;    //start conversion
	_DINT();
	sampleCounter += 2;
	time=sampleCounter - lastBeatTime;
	if((Signal < thresh) && (time > (IBI/5)*3))
	{
		if (Signal < Trough)
		{
			Trough = Signal;
		}
	}

	if((Signal > thresh) && (Signal > Peak))
	{
		Peak = Signal;
	}

	if (time > 250)
	{
		if ( (Signal > thresh) && (Pulse == false) && (time > (IBI/5)*3) )
		{
			Pulse = true;
			IBI = sampleCounter - lastBeatTime;
			lastBeatTime = sampleCounter;
			if(secondBeat)
			{
				secondBeat = false;
				for(i=0; i<=9; i++)
				{
					rate[i] = IBI;
				}
			}
			if(firstBeat)
			{
				firstBeat = false;
				secondBeat = true;
				_EINT();
				return;
			}
			for(i=0; i<=8; i++)
			{
				rate[i] = rate[i+1];
				runningTotal += rate[i];
			}
			rate[9] = IBI;
			runningTotal += rate[9];
			runningTotal /= 10;
			BPM = 60000/runningTotal;
			if(BPM>200)BPM=200;		
			if(BPM<30)BPM=30;				
			QS = true;
   		}
	}
	if ((Signal < thresh) && (Pulse == true))
	{
		Pulse = false;
    	amp = Peak - Trough;
		thresh = amp/2 + Trough;
		Peak = thresh;
		Trough = thresh;
	}
	if (time > 2500)
	{
		thresh = 512;
		Peak = 512;
		Trough = 512;
		lastBeatTime = sampleCounter;
		firstBeat = true;
		secondBeat = false;
	}
 _EINT();  
}
