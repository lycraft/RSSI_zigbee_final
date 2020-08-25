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
#include "tcp.h"
#include "esp8266.h"

#define rssi_A 37
#define rssi_n 2.5

uint8_t flag = 1;
uint16_t MessCount = 0;


int main(void)
 {	
		vu8 key=0;
		u16 t;  
		u16 len;	
	  uint8_t res;
//	  char str[100]={0};
		char rsstr[100]={0};
		u16 times=0;  
		u16 rssiA = 0, rssiB = 0, rssiC = 0;
//		double dA = 0, dB = 0, dC = 0;
		char RevString[128] = {32,32,32,32,32,32,32,32,32};
		
		delay_init();	    	 //��ʱ������ʼ��	  
		NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� 	
		OLED_Init();			//��ʼ��OLED  
//		KEY_Init();          //��ʼ���밴�����ӵ�Ӳ���ӿ�
		EXTIX_Init();
		uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
		ESP8266_Init(115200);
		
		
		OLED_ShowString(0,0, "configing_0");	
    ESP8266_AT_Test();
			OLED_ShowString(0,0, "configing_1");	 
		printf("��������ESP8266\r\n");
    ESP8266_Net_Mode_Choose(STA);
			OLED_ShowString(0,0, "configing_2");
    while(!ESP8266_JoinAP(User_ESP8266_SSID, User_ESP8266_PWD));
			OLED_ShowString(0,0, "configing_3");
    ESP8266_Enable_MultipleId ( DISABLE );
			OLED_ShowString(0,0, "configing_4");
    while(!ESP8266_Link_Server(enumTCP, User_ESP8266_TCPServer_IP, User_ESP8266_TCPServer_PORT, Single_ID_0));
			OLED_ShowString(0,0, "configing_5");
    while(!ESP8266_UnvarnishSend());
			OLED_ShowString(0,0, "configing_6");
		printf("\r\n�������");
			OLED_ShowString(0,0, "config is ok!");	
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
//			dA=pow(10,(float)(rssiA-rssi_A)/10/rssi_n); 
			
			rssiB = (u16)(RevString[5] - 0x30) * 100;
			rssiB = (u16)(RevString[6] - 0x30) * 10 + rssiB;
			rssiB = (u16)(RevString[7] - 0x30) + rssiB;
//			dB=pow(10,(float)(rssiB-rssi_A)/10/rssi_n); 
			
			rssiC = (u16)(RevString[9] - 0x30) * 100;
			rssiC = (u16)(RevString[10] - 0x30) * 10 + rssiC;
			rssiC = (u16)(RevString[11] - 0x30) + rssiC;
//			dC=pow(10,(float)(rssiC-rssi_A)/10/rssi_n); 
			
			
//			printf("A:%f B:%f C:%f\r\n",dA, dB, dC);
			
//			sprintf (str,"_%f_%f_%f\r\n" ,dA, dB, dC);//��ʽ�������ַ�����TCP������
//			ESP8266_SendString ( ENABLE, str, 0, Single_ID_0 );
			MessCount++;
			OLED_ShowString(0,2, "send a mess");
			OLED_ShowNum(0,4,MessCount,4,16);
			
			sprintf (rsstr,"#%d#%d#%d\r\n" ,rssiA, rssiB, rssiC);//��ʽ�������ַ�����TCP������
			ESP8266_SendString ( ENABLE, rsstr, 0, Single_ID_0 );
			
			if(MessCount == 2000)
			{
				MessCount = 0;
			}
			
		}else
		{
			times++;	
//			if(times%200==0)printf("%s %d %d A=%d,B=%d,C=%d\r\n\r\n",RevString,strlen(RevString),sizeof(RevString),rssiA, rssiB, rssiC); 
//				if(times%200 == 0)
//				{
//					printf("A����%f B����%f C����%f\r\n",dA, dB, dC); 
//					sprintf (str,"A����%f B����%f C����%f\r\n" ,dA, dB, dC);//��ʽ�������ַ�����TCP������
//					ESP8266_SendString ( ENABLE, str, 0, Single_ID_0 );
//				}
					
			delay_ms(10);
			
			if(times == 5000)
				times = 0;
			
		}
//========================================================================��֤ʱ������

        if(TcpClosedFlag) //�ж��Ƿ�ʧȥ����
        {
            ESP8266_ExitUnvarnishSend(); //�˳�͸��ģʽ
            do
            {
                res = ESP8266_Get_LinkStatus();     //��ȡ����״̬
            }   
            while(!res);

            if(res == 4)                     //ȷ��ʧȥ���ӣ�����
            {
                
                
                while (!ESP8266_JoinAP(User_ESP8266_SSID, User_ESP8266_PWD ) );
                while (!ESP8266_Link_Server(enumTCP, User_ESP8266_TCPServer_IP, User_ESP8266_TCPServer_PORT, Single_ID_0 ) );        
            } 
            while(!ESP8266_UnvarnishSend());                    
        }


	}	  
}

