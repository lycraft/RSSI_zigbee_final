/**************************************************************************************************
  Filename:       SampleApp.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Sample Application (no Profile).


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends it's messages either as broadcast or
  broadcast filtered group messages.  The other (more normal)
  message addressing is unicast.  Most of the other sample
  applications are written to support the unicast message model.

  Key control:
    SW1:  Sends a flash command to all devices in Group 1.
    SW2:  Adds/Removes (toggles) this device in and out
          of Group 1.  This will enable and disable the
          reception of the flash command.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "SampleApp.h"
#include "SampleAppHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "MT_UART.h"
#include "MT_APP.h"
#include "MT.h"

#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
//uint8 FunctionProfession = ReferenceProfession;//���豸�ǲο��ڵ�
uint8 FunctionProfession = BlindProfession;//���豸��ä�ڵ�
//uint8 FunctionProfession = CoorProfession;//���豸��Э����

uint16 rssiA = 0, rssiB = 0, rssiC = 0;
uint8 SendCount = 0;
//���ڽ�������
//#define MY_DEFINI_UART_PORT 0 
//#ifndef RX_MAX_LENGTH
//#define RX_MAX_LENGTH 32
//#endif
//uint8 RX_BUFFER[RX_MAX_LENGTH];
//uint8 RX_Length;
//void UART_CallBackFunction(uint8 port, uint8 event);//�����������ݵ���ʱ�����ô˺���

// This list should be filled with Application specific Cluster IDs.
const cId_t SampleApp_ClusterList[SAMPLEAPP_MAX_CLUSTERS] =
{
  SAMPLEAPP_BLIND_CLUSTERID,
  SAMPLEAPP_FLASH_CLUSTERID,
  SAMPLEAPP_ROUT_ANODE_CLUSTERID,
  SAMPLEAPP_ROUT_BNODE_CLUSTERID,
  SAMPLEAPP_ROUT_CNODE_CLUSTERID
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  uint8 *pAppInClusterList;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SampleApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t SampleApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8 SampleApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // SampleApp_Init() is called.
devStates_t SampleApp_NwkState;

uint8 SampleApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SampleApp_Periodic_DstAddr;//ä�ڵ�㲥
afAddrType_t SampleApp_Flash_DstAddr;
afAddrType_t SampleApp_p2p_DstAddr;//�ο��ڵ�㲥��ַ

aps_Group_t SampleApp_Group;

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;

//���������ڲο��ڵ���յ�����ʱ���м���Ȼ�󷢸�Э������
uint8 rssi_val[10];
uint8 rssi_index = 0;
uint8 rssi_s[3] = {'\0'};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage( void );//ä�ڵ�㲥
void SampleApp_SendFlashMessage( uint16 flashTime );
void SampleApp_P2P_SendMessage(void);//�ο��ڵ�㲥
/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */
bool REV_Ack_OK(void);
extern void macRadioUpdateTxPower(void);
/*********************************************************************
 * @fn      SampleApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SampleApp_Init( uint8 task_id )
{ 
  SampleApp_TaskID = task_id;
  SampleApp_NwkState = DEV_INIT;
  SampleApp_TransID = 0;
  
  macRadioUpdateTxPower();
//  halUARTCfg_t uartConfig;//��ʼ������ʹ��
  //------------------------���ô���---------------------------------
  MT_UartInit();                    //���ڳ�ʼ��
  MT_UartRegisterTaskID(task_id);   //ע�ᴮ������
  HalUARTWrite(0,"UartInit OK\n", sizeof("UartInit OK\n"));
  //-----------------------------------------------------------------
  //����ESP8266��ʼ��------------------------------------------------
  //ESP8266����  RST��������500ms
  
  //�ָ���������
//  HalUARTWrite(0,"AT+RESTORE\r\n", sizeof("AT+RESTORE\r\n"));
  //�ȴ��յ�ok�ַ�
//  AT+CWMODE=1
//  AT+CWJAP=
//  AT+CIPMUX=0
//  AT+CIPSTART=
//  AT+CIPMODE=1
//  AT+CIPSEND
  
//  HalUARTWrite(0,"AT+CIPSTART=\"TCP\",\"192.168.31.194\",80\r\n", sizeof("AT+CIPSTART=\"TCP\",\"192.168.31.194\",6000\r\n"));
  
  
  //------------------------------------------------------------------
  
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in SampleAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES

#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif

  // Setup for the periodic message's destination address
  // Broadcast to everyone
  SampleApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  SampleApp_Periodic_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;//�㲥

  
  // �㲥���á���λʱ���ο��ڵ���Э����֮���ͨѶ��Ӧ���ǵ㲥���Ӷ���Э�����ܹ�����
  
  SampleApp_p2p_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  SampleApp_p2p_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_p2p_DstAddr.addr.shortAddr = 0x0000;//������Ϣ��Э����
  
  // Setup for the flash command's destination address - Group 1
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;

  // Fill out the endpoint description.
  SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_epDesc.task_id = &SampleApp_TaskID;
  SampleApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;
  SampleApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &SampleApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( SampleApp_TaskID );

  // By default, all devices start out in Group 1
  SampleApp_Group.ID = 0x0001;
  osal_memcpy( SampleApp_Group.name, "Group 1", 7  );
  aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );

  
  //���ڳ�ʼ��
//  uartConfig.configured       = TRUE;
//  uartConfig.baudRate         = HAL_UART_BR_115200;
//  uartConfig.flowControl      = FALSE;
//  uartConfig.flowControlThreshold = MT_UART_THRESHOLD;//???????
//  uartConfig.rx.maxBufSize     = 200;
//  uartConfig.tx.maxBufSize     =200;
//  uartConfig.idleTimeout       = MT_UART_IDLE_TIMEOUT;
//  uartConfig.intEnable          =TRUE;
//  uartConfig.callBackFunc       =UART_CallBackFunction;
//  HalUARTOpen(MY_DEFINI_UART_PORT,&uartConfig);//ʹ���Զ���Ĵ��ڳ�ʼ����������������ͨѶ
  

  
  
  
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "SampleApp", HAL_LCD_LINE_1 );
#endif
}

/*********************************************************************
 * @fn      SampleApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 SampleApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        // ��⵽�а������µ�ʱ��ִ������ĺ���
        case KEY_CHANGE:
          SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        // �豸�յ�������Ϣ��ʱ��ִ������ĺ���
        case AF_INCOMING_MSG_CMD:
          SampleApp_MessageMSGCB( MSGpkt );

          break;

        // �豸���緢���ı�ʱִ������ĺ��� ����ִֻ��һ��
        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( //(SampleApp_NwkState == DEV_ZB_COORD)|| //Э����ֻ���ղ�����
              (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
            // Start sending the periodic message in a regular interval.
            osal_start_timerEx( SampleApp_TaskID,
                              SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
                              SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );
          }
          else
          {
            // Device is no longer in the network
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SampleApp_Init()).
  if ( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )//�ο��ڵ㲻�㲥�ģ�
  {
    if(zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER && FunctionProfession == BlindProfession)//�����·����������ä�ڵ�͹㲥
    {
      // ���������Ե���Ϣ
      SampleApp_SendPeriodicMessage();

      // Setup to send message again in normal period (+ a little jitter)
      osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
        (SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

      // return unprocessed events
      return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
    }
    
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */
/*********************************************************************
 * @fn      SampleApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter
  
  if ( keys & HAL_KEY_SW_1 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
//    SampleApp_SendFlashMessage( SAMPLEAPP_FLASH_DURATION );
  }

  if ( keys & HAL_KEY_SW_2 )
  {
    
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  uint16 flashTime;

  switch ( pkt->clusterId )
  {
    case SAMPLEAPP_BLIND_CLUSTERID://�ο��ڵ����ä�ڵ����ݽ��д���
      if(zgDeviceLogicalType == ZG_DEVICETYPE_ROUTER && FunctionProfession == ReferenceProfession)//�ο��ڵ�·������������
      {
        rssi_val[rssi_index++] = ((~pkt->rssi)+1); // �õ�rssiֵ������   
        if(rssi_index == 6)//�����յ�6��rssi�����ƽ��ֵ����
        {   
          uint8 i,max_i,min_i; 
          uint16 val = 0;
          //ȥ�������Сֵ��ʣ�µ���ȡƽ��ֵ
          max_i = min_i = 0;          
          for(i = 1; i<6; i++)
          {
            if(rssi_val[i]>rssi_val[max_i])
              max_i = i;              
            if(rssi_val[i]<rssi_val[min_i])
              min_i = i;
        }          
          for(i = 0; i<6; i++)
          {
            if(i!=max_i && i!=min_i)
              val+=rssi_val[i];//ȥ�������Сֵ֮���4��rssiֵ���ܺ�
          }          
          val >>= 2;//����Ϊ������������ά��ʾ����4��valΪ4��RSSIֵ�ĺͣ�
        HalLcdWriteValue( val, 10, HAL_LCD_LINE_3);//�ο��ڵ�OLED ��Ļ��ʾ������ƽ���ź�ֵ
          //      rssi_s[0] ='A';
          rssi_s[0] = val/100+0x30;//ASCII���� 0x30��Ӧ����0 ����������������Ҫ��ʾ�����ַ�
          rssi_s[1] = val%100/10+0x30;
          rssi_s[2] = val%10+0x30;//��������ֱ����õ����ף����ף����� 

          SampleApp_P2P_SendMessage();
          rssi_index = 0;//ÿ�յ�6��RSSIֵ����һ����Ҫ��RSSI��ֵ

      
        }
      }

      
      
      break;
        //ȷ��ֻ��Э�������б���
  case SAMPLEAPP_ROUT_ANODE_CLUSTERID://Э������ê�ڵ��������
    if(zgDeviceLogicalType == ZG_DEVICETYPE_COORDINATOR)
    {
      uint16 rssi = 0, lcdrs = 0;
      uint8 *process;
      uint8 temp[14]={32,32,32,32,32,32,32,32,32,32,32,32,32,32};
      
//      HalUARTWrite(0,"A",1);
//      HalUARTWrite(0,pkt->cmd.Data,pkt->cmd.DataLength); //������յ������� 
//      HalUARTWrite(0,"\n", 1); // �س����� 
      
      process = pkt->cmd.Data;
      rssi = (process[0] - 0x30) * 100;
      rssi = (process[1] - 0x30) * 10 + rssi;
      rssi = (process[2] - 0x30) + rssi;
      
      rssiA = rssi;
       
      SendCount++;
      if(SendCount == 3)
      {
        lcdrs = rssiA * 1000 + rssiB;
        HalLcdWriteValue( lcdrs, 10, HAL_LCD_LINE_3);//xietiaoqi�ڵ�OLED ��Ļ��ʾ������ƽ���ź�ֵ
        HalLcdWriteValue( rssiC, 10, HAL_LCD_LINE_4);//xietiaoqi�ڵ�OLED ��Ļ��ʾ������ƽ���ź�ֵ
        sprintf(temp,"A%03dB%03dC%03d\r\n", rssiA, rssiB, rssiC);
        HalUARTWrite(0,temp,14 );//����stm32
        SendCount = 0;
      }
    }
    
    break;
  case SAMPLEAPP_ROUT_BNODE_CLUSTERID://Э������ê�ڵ��������
    if(zgDeviceLogicalType == ZG_DEVICETYPE_COORDINATOR)
    {   
      uint16 rssi = 0, lcdrs = 0;
      uint8 *process;
      uint8 temp[14]={32,32,32,32,32,32,32,32,32,32,32,32,32,32};
      
      
      process = pkt->cmd.Data;
      rssi = (process[0] - 0x30) * 100;
      rssi = (process[1] - 0x30) * 10 + rssi;
      rssi = (process[2] - 0x30) + rssi;
      
      rssiB = rssi;
      SendCount++;
      if(SendCount == 3)
      {
        lcdrs = rssiA * 1000 + rssiB;
        HalLcdWriteValue( lcdrs, 10, HAL_LCD_LINE_3);//xietiaoqi�ڵ�OLED ��Ļ��ʾ������ƽ���ź�ֵ
        HalLcdWriteValue( rssiC, 10, HAL_LCD_LINE_4);//xietiaoqi�ڵ�OLED ��Ļ��ʾ������ƽ���ź�ֵ
        sprintf(temp,"A%03dB%03dC%03d\r\n", rssiA, rssiB, rssiC);
        HalUARTWrite(0,temp,14 );//����stm32
        SendCount = 0;
      } 
    }
    break;
  case SAMPLEAPP_ROUT_CNODE_CLUSTERID://Э������ê�ڵ��������
    if(zgDeviceLogicalType == ZG_DEVICETYPE_COORDINATOR)
    {
      uint16 rssi = 0, lcdrs = 0;
      uint8 *process;
      uint8 temp[14]={32,32,32,32,32,32,32,32,32,32,32,32,32,32};
      
      process = pkt->cmd.Data;
      rssi = (process[0] - 0x30) * 100;
      rssi = (process[1] - 0x30) * 10 + rssi;
      rssi = (process[2] - 0x30) + rssi;
      
      rssiC = rssi;
      SendCount++;
      if(SendCount == 3)
      {
        lcdrs = rssiA * 1000 + rssiB;
        HalLcdWriteValue( lcdrs, 10, HAL_LCD_LINE_3);//xietiaoqi�ڵ�OLED ��Ļ��ʾ������ƽ���ź�ֵ
        HalLcdWriteValue( rssiC, 10, HAL_LCD_LINE_4);//xietiaoqi�ڵ�OLED ��Ļ��ʾ������ƽ���ź�ֵ
        sprintf(temp,"A%03dB%03dC%03d\r\n", rssiA, rssiB, rssiC);
        HalUARTWrite(0,temp,14 );//���͸�stm32
        SendCount = 0;
      } 
    }
    break;
      

    case SAMPLEAPP_FLASH_CLUSTERID:
      flashTime = BUILD_UINT16(pkt->cmd.Data[1], pkt->cmd.Data[2] );
      HalLedBlink( HAL_LED_4, 4, 50, (flashTime / 4) );
      break;
  }
}

/*********************************************************************
 * @fn      SampleApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPeriodicMessage( void )//ä�ڵ������Թ㲥
{
  byte state[]="Blind node";
  if ( AF_DataRequest( &SampleApp_Periodic_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_BLIND_CLUSTERID,
                       sizeof("Blind node"),
                       state,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      SampleApp_SendFlashMessage
 *
 * @brief   Send the flash message to group 1.
 *
 * @param   flashTime - in milliseconds
 *
 * @return  none
 */
void SampleApp_SendFlashMessage( uint16 flashTime )
{
  uint8 buffer[3];
  buffer[0] = (uint8)(SampleAppFlashCounter++);
  buffer[1] = LO_UINT16( flashTime );
  buffer[2] = HI_UINT16( flashTime );

  if ( AF_DataRequest( &SampleApp_Flash_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_FLASH_CLUSTERID,
                       3,
                       buffer,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
*********************************************************************/
void SampleApp_P2P_SendMessage( void )//�ο��ڵ�㲥��Э����������
{
   if ( AF_DataRequest( &SampleApp_p2p_DstAddr, 
                      &SampleApp_epDesc,
                      SAMPLEAPP_ROUT_CNODE_CLUSTERID,
                      3,
                      rssi_s,
                      &SampleApp_TransID,
                      AF_DISCV_ROUTE,
                      AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

//static void UART_CallBackFunction(uint8 port, uint8 event)
//{
//  uint8 RX_Flag = 0;
//  RX_Length = 0;//�����ַ�������
//  RX_Flag = RX_Length = Hal_UART_RxBufLen(MY_DEFINI_UART_PORT);// �ַ�������
//  
//  if(RX_Flag != 0)// �����ݴ���
//  {
//    //��ȡ��������
//    HalUARTRead(MY_DEFINI_UART_PORT,RX_BUFFER,RX_Length);
//    
//    {
//      //���ݴ��ش��ڵ���hal_uart.h�ӿں���
////      HalUARTWrite(MY_DEFINI_UART_PORT,RX_BUFFER,RX_Length);
//      HalLcdWriteString ( RX_BUFFER, HAL_LCD_LINE_3);
//      if(RX_Length <= RX_MAX_LENGTH)
//      {
//        if(REV_Ack_OK() == true)
//        {
//          HalLcdWriteString ( "okkkkkkkkk", HAL_LCD_LINE_4);
//        }
//      }
//      
//      
//      
//      *RX_BUFFER = NULL;
//    }
//  }
//  
//}
//
//bool REV_Ack_OK(void)
//{
//  uint8 i = 0;
//  for(;i < RX_MAX_LENGTH;i++)
//  {
//    if(RX_BUFFER[i] == 'O')
//    {
//      if(RX_BUFFER[i+1] == 'K')
//      {
//        return true;
//      }
//    }
//  }
//  return false;
//}