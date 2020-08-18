//////////////////////////////////////////////////////////////////////////////////	 
//  �� �� ��   : main.c
//  �� �� ��   : v2.0
//  ��������   : OLED 4�ӿ���ʾ����(51ϵ��)
//              ˵��: 
//              ----------------------------------------------------------------
//              GND    ��Դ��
//              VCC  ��5V��3.3v��Դ
//              D0   ��PA8��SCL��
//              D1   ��PB15��SDA��
//              RES  ��PB14
//              DC   ��PB13
//              CS   ��PA3               
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


void meau()//mode0��ˢ��
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
		
		delay_init();	    	 //��ʱ������ʼ��	  
		NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� 	
		OLED_Init();			//��ʼ��OLED  
//		KEY_Init();          //��ʼ���밴�����ӵ�Ӳ���ӿ�
		EXTIX_Init();
		uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
		
	 meau();
	while(1) 
	{		
		if(USART_RX_STA&0x8000)
		{					  
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			for(t=0;t<len;t++)
			{
//				USART_SendData(USART1, USART_RX_BUF[t]);         //�򴮿�1��������
				RevString[t] = USART_RX_BUF[t];//�������յ������ݴ�ŵ�RevString�ַ���������
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
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
				if(times%200 == 0)printf("A����%f B����%f C����%f\r\n",dA, dB, dC); 
			delay_ms(10);
			
		}
//========================================================================Meau
		
//========================================================================Measure
		
//========================================================================��ѹ��������
	
//=========================================================================��������

	}	  
}

