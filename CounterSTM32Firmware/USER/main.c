//////////////////////////////////////////////////////////////////////////////////	 
//  文 件 名   : main.c
//  版 本 号   : v2.0
//  功能描述   : OLED 4接口演示例程(51系列)
//              说明: 
//              ----------------------------------------------------------------
//              GND    电源地
//              VCC  接5V或3.3v电源
//              D0   接PA8（SCL）
//              D1   接PB15（SDA）
//              RES  接PB14
//              DC   接PB13
//              CS   接PA3               
//              ----------------------------------------------------------------
//******************************************************************************/
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "key.h"
#include "exti.h" 
#include "usart.h"
#include "string.h"
#include "math.h"

#define rssi_A 27
#define rssi_n 3

uint8_t flag = 1;


void meau()//mode0不刷新
{
		OLED_Clear();
		OLED_DrawBMP(96,0,128,2,BMP2);	
		OLED_ShowString(0,6,"Meau");
		OLED_ShowString(40,0,"Measure");
		OLED_ShowString(40,2,"Voltage");
		OLED_ShowString(40,4,"Animation");
		OLED_ShowString(72,6,"Lycraft");
}

int main(void)
 {	
		vu8 key=0;
		u16 t;  
		u16 len;	
		u16 times=0;  
		u16 rssiA = 0, rssiB = 0, rssiC = 0;
		double dA = 0, dB = 0, dC = 0;
		char RevString[128] = {32,32,32,32,32,32,32,32,32};
		
		delay_init();	    	 //延时函数初始化	  
		NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级 	
		OLED_Init();			//初始化OLED  
//		KEY_Init();          //初始化与按键连接的硬件接口
		EXTIX_Init();
		uart_init(115200);	 //串口初始化为115200
		
	 meau();
	while(1) 
	{		
		if(USART_RX_STA&0x8000)
		{					  
			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			for(t=0;t<len;t++)
			{
//				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				RevString[t] = USART_RX_BUF[t];//将串口收到的数据存放到RevString字符串容器中
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			USART_RX_STA=0;
			rssiA = (u16)(RevString[1] - 0x30) * 100;
			rssiA = (u16)(RevString[2] - 0x30) * 10 + rssiA;
			rssiA = (u16)(RevString[3] - 0x30) + rssiA;
			dA=pow(10,(float)(rssiA-rssi_A)/10/rssi_n); 
			
			rssiB = (u16)(RevString[5] - 0x30) * 100;
			rssiB = (u16)(RevString[6] - 0x30) * 10 + rssiB;
			rssiB = (u16)(RevString[7] - 0x30) + rssiB;
			dB=pow(10,(float)(rssiB-rssi_A)/10/rssi_n); 
			
			rssiC = (u16)(RevString[9] - 0x30) * 100;
			rssiC = (u16)(RevString[10] - 0x30) * 10 + rssiC;
			rssiC = (u16)(RevString[11] - 0x30) + rssiC;
			dC=pow(10,(float)(rssiC-rssi_A)/10/rssi_n); 
		}else
		{
			times++;	
			if(times%200==0)printf("%s %d %d A=%d,B=%d,C=%d\r\n\r\n",RevString,strlen(RevString),sizeof(RevString),rssiA, rssiB, rssiC); 
				if(times%200 == 0)printf("A距离%f B距离%f C距离%f\r\n",dA, dB, dC); 
			delay_ms(10);
			
		}
//========================================================================Meau
		
//========================================================================Measure
		
//========================================================================电压电流测量
	
//=========================================================================方波发生

	}	  
}

