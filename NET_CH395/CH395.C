/********************************** (C) COPYRIGHT *********************************
* File Name          : CH395.C
* Author             : WCH
* Version            : V1.1
* Date               : 2014/8/1
* Description        : CH395������ʾ
**********************************************************************************/

/**********************************************************************************
CH395 TCP/IP Э����ӿ�
MSC51 ��ʾ����������ʾSocket0������TCP Clientģʽ�£���Ƭ���յ����ݺ󣬰�λȡ����
�ϴ���MCS51@24MHZ,KEIL 3.51
**********************************************************************************/
/* ͷ�ļ�����*/

#include "stdio.h"
#include "string.h"
#include "CH395INC.H"
#include "CH395.H"

extern void xprintf (const char* fmt, ...);

/**********************************************************************************

CH395_OP_INTERFACE_MODE����Ϊ1-5
1��Ӳ�����߲������ӷ�ʽ
2������ģ�Ⲣ�����ӷ�ʽ
3: Ӳ��SPI���ӷ�ʽ
4: ����ģ��SPI��ʽ
5: Ӳ���첽�������ӷ�ʽ
*/
#define   CH395_OP_INTERFACE_MODE             3                      
#if   (CH395_OP_INTERFACE_MODE == 1)                                 /* SEL = 0, TX = 1*/
#include "../PUB/CH395PARA_HW.C"                                           
#elif (CH395_OP_INTERFACE_MODE == 2)                                 /* SEL = 0, TX = 1*/
#include "../PUB/CH395PARA_SW.C"                                            
#elif (CH395_OP_INTERFACE_MODE == 3)                                 /* SEL = 1, TX = 0*/
#include "CH395SPI_HW.C"
#elif (CH395_OP_INTERFACE_MODE == 4)                                 /* SEL = 1, TX = 0*/
#include "../PUB/CH395SPI_SW.C"
#elif (CH395_OP_INTERFACE_MODE == 5)                                 /* SEL = 1, TX = 1*/
#include "../PUB/CH395UART.C"
#else
#error "Please Select Correct Communication Interface "
#endif

/**********************************************************************************/
/* ���������ļ� */
#include "CH395CMD.C"

#define    CH395_DEBUG           1
#define DEF_KEEP_LIVE_IDLE                           (15*1000)        /* ����ʱ�� */
#define DEF_KEEP_LIVE_PERIOD                         (20*1000)        /* ���Ϊ15�룬����һ��KEEPLIVE���ݰ� */                  
#define DEF_KEEP_LIVE_CNT                            200                /* ���Դ���  */

/* ���ñ������� */
UINT8  MyBuffer[512];                                           /* ���ݻ����� */
UINT8  SMyBuffer[512];                                           /* �������ݻ����� */ //jdb2018-04-20
unsigned int buflen, sbuflen = 0;				//jdb2018-04-02 ��������Ч���ݳ���
struct _SOCK_INF  SockInf;                                      /* ����Socket��Ϣ */
struct _CH395_SYS  CH395Inf;                                    /* ����CH395��Ϣ */

struct _SOCK_INF  UDPSockInf;         //jdb2018-05-24                             /* ����UDPSocket��Ϣ */

/* CH395��ض��� */
//const UINT8 CH395IPAddr[4] = {192,168,99,222};//{192,168,111,100};                         /* CH395IP��ַ */
UINT8 CH395IPAddr[4] = {192,168,99,222};//{192,168,111,100};                         /* CH395IP��ַ *///jdb2018-05-03
const UINT8 CH395GWIPAddr[4] = {192,168,99,1};                        /* CH395���� */
const UINT8 CH395IPMask[4] = {255,255,255,0};                        /* CH395�������� */

/* socket ��ض���*/
const UINT8  Socket0DesIP[4] = {192,168,1,103};//{192,168,111,78};                      /* Socket 0Ŀ��IP��ַ */
const UINT16 Socket0DesPort = 6000;                                  /* Socket 0Ŀ�Ķ˿� */
const UINT16 Socket0SourPort = 6000;                                 /* Socket 0Դ�˿� */
const UINT8  BroadcastIP[4] = {255,255,255,255};                   //jdb2018-05-24  /* UDP �㲥��ַ */

const UINT16 Socket1SourPort = 11024;                                 /* Socket 1Դ�˿� */


/**********************************************************************************
* Function Name  : mStopIfError
* Description    : ����ʹ�ã���ʾ������룬��ͣ��
* Input          : iError
* Output         : None
* Return         : None
**********************************************************************************/
void mStopIfError(UINT8 iError)
{	
		#if 1
    if (iError == CMD_ERR_SUCCESS) return;                           /* �����ɹ� */
		#else
		//jdb2018-03-16
		if (iError == CH395_ERR_OPEN) return;
		#endif
#if CH395_DEBUG
    xprintf("Error: %02X\n", (UINT16)iError);                         /* ��ʾ���� */
#endif
    while ( 1 ) 
    {
        mDelaymS(200);
        mDelaymS(200);
    }
}

/**********************************************************************************
* Function Name  : InitCH395InfParam
* Description    : ��ʼ��CH395Inf����
* Input          : None
* Output         : None
* Return         : None
**********************************************************************************/
//jdb2018-03-16
#if 0
void InitCH395InfParam(void)
#else
void InitCH395InfParam(UINT8 IP1, UINT8 IP2, UINT8 IP3, UINT8 IP4)
#endif
{
	//jdb2018-05-03 IP��ַ������
	CH395IPAddr[0] = IP1;
	CH395IPAddr[1] = IP2;
	CH395IPAddr[2] = IP3;
	CH395IPAddr[3] = IP4;
	
    memset(&CH395Inf,0,sizeof(CH395Inf));                            /* ��CH395Infȫ������*/
    memcpy(CH395Inf.IPAddr,CH395IPAddr,sizeof(CH395IPAddr));         /* ��IP��ַд��CH395Inf�� */
    memcpy(CH395Inf.GWIPAddr,CH395GWIPAddr,sizeof(CH395GWIPAddr));   /* ������IP��ַд��CH395Inf�� */
    memcpy(CH395Inf.MASKAddr,CH395IPMask,sizeof(CH395IPMask));       /* ����������д��CH395Inf�� */
}

/**********************************************************************************
* Function Name  : InitSocketParam
* Description    : ��ʼ��socket
* Input          : None
* Output         : None
* Return         : None
**********************************************************************************/
void InitSocketParam(void)
{
    memset(&SockInf,0,sizeof(SockInf));                              /* ��SockInf[0]ȫ������*/
		
		#if 0
    memcpy(SockInf.IPAddr,Socket0DesIP,sizeof(Socket0DesIP));        /* ��Ŀ��IP��ַд�� */
    SockInf.DesPort = Socket0DesPort;                                /* Ŀ�Ķ˿� */
    SockInf.SourPort = Socket0SourPort;                              /* Դ�˿� */
    SockInf.ProtoType = PROTO_TYPE_TCP;                              /* TCPģʽ */
    SockInf.TcpMode = TCP_CLIENT_MODE;
		#else
		//jdb2018-03-16
    SockInf.SourPort = Socket1SourPort;                              /* Դ�˿� */
    SockInf.ProtoType = PROTO_TYPE_TCP;                              /* TCPģʽ */
		SockInf.TcpMode = TCP_SERVER_MODE;															
		#endif

		//jdb2018-05-24   UDPģʽ
	memset(&UDPSockInf,0,sizeof(UDPSockInf));                        /* ��SockInf[0]ȫ������*/
    memcpy(UDPSockInf.IPAddr,BroadcastIP,sizeof(BroadcastIP));     /* ��Ŀ��IP��ַд�� */
    UDPSockInf.DesPort = Socket0DesPort;                             /* Ŀ�Ķ˿� */
    UDPSockInf.SourPort = Socket0SourPort;                           /* Դ�˿� */
    UDPSockInf.ProtoType = PROTO_TYPE_UDP;                           /* UDPģʽ */
}

/**********************************************************************************
* Function Name  : CH395SocketInitOpen
* Description    : ����CH395 socket ��������ʼ������socket
* Input          : None
* Output         : None
* Return         : None
**********************************************************************************/
void CH395SocketInitOpen(void)
{
    UINT8 i;

	//jdb2018-05-24  UDPģʽ
	    /* socket 0ΪUDPģʽ */
    CH395SetSocketDesIP(0,UDPSockInf.IPAddr);                        /* ����socket 0Ŀ��IP��ַ */         
    CH395SetSocketProtType(0,PROTO_TYPE_UDP);                        /* ����socket 0Э������ */
    CH395SetSocketDesPort(0,UDPSockInf.DesPort);                     /* ����socket 0Ŀ�Ķ˿� */
    CH395SetSocketSourPort(0,UDPSockInf.SourPort);                   /* ����socket 0Դ�˿� */
    i = CH395OpenSocket(0);                                          /* ��socket 0 */
	if (i == CH395_ERR_OPEN){
			i = CH395CloseSocket(0);
			mStopIfError(i);
			i = CH395OpenSocket(0);
		}
    mStopIfError(i);

		#if 0
    /* socket 0ΪTCP �ͻ���ģʽ */
    CH395SetSocketDesIP(0,SockInf.IPAddr);                           /* ����socket 0Ŀ��IP��ַ */         
    CH395SetSocketProtType(0,SockInf.ProtoType);                     /* ����socket 0Э������ */
    CH395SetSocketDesPort(0,SockInf.DesPort);                        /* ����socket 0Ŀ�Ķ˿� */
    CH395SetSocketSourPort(0,SockInf.SourPort);                      /* ����socket 0Դ�˿� */
	
    i = CH395OpenSocket(0);                                          /* ��socket 0 */
    mStopIfError(i);                                                 /* ����Ƿ�ɹ� */
	
    i = CH395TCPConnect(0);                                          /* TCP���� */
    mStopIfError(i);                                                 /* ����Ƿ�ɹ� */
		#else
		//jdb2018-03-16
    CH395SetSocketProtType(1,PROTO_TYPE_TCP); 
    CH395SetSocketSourPort(1,SockInf.SourPort);
    i = CH395OpenSocket(1);
		if (i == CH395_ERR_OPEN){
			i = CH395CloseSocket(1);
			mStopIfError(i);
			i = CH395OpenSocket(1);
		}
    mStopIfError(i); 
    i = CH395TCPListen (1);                                 
    mStopIfError(i);                                           
		#endif
}



/*********************************************************************************
* Function Name  : keeplive_set
* Description    : ���ʱ����������
* Input          : None
* Output         : None
* Return         : None
**********************************************************************************/
void keeplive_set(void)
{
	CH395KeepLiveCNT(DEF_KEEP_LIVE_CNT);
	CH395KeepLiveIDLE(DEF_KEEP_LIVE_IDLE);
	CH395KeepLiveINTVL(DEF_KEEP_LIVE_PERIOD);
}

/**********************************************************************************
* Function Name  : CH395SocketInterrupt
* Description    : CH395 socket �ж�,��ȫ���ж��б�����
* Input          : sockindex
* Output         : None
* Return         : None
**********************************************************************************/
void CH395SocketInterrupt(UINT8 sockindex)
{
   UINT8  sock_int_socket;
   UINT8  i;
   UINT16 len;
   UINT16 tmp;
	 UINT8 buf[10];

	 //return; //jdb2018-04-22 ����Ϊ��ѯ��ʽ

	 //if(sockindex != 1){ //jdb2018-04-20
	 	//return;
	 //}

   sock_int_socket = CH395GetSocketInt(sockindex);                   /* ��ȡsocket ���ж�״̬ */
	#if 1 //jdb2018-04-22 ����Ϊ��ѯ��ʽ
   if(sock_int_socket & SINT_STAT_SENBUF_FREE)                       /* ���ͻ��������У����Լ���д��Ҫ���͵����� */
   {
#if CH395_DEBUG
       xprintf("SINT_STAT_SENBUF_FREE \n");
#endif
	   //if(sbuflen > 0)
	   		//CH395SendData(sockindex,SMyBuffer,sbuflen); //jdb2018-04-20
   }
   #endif
   #if 1
   if(sock_int_socket & SINT_STAT_SEND_OK)                           /* ��������ж� */
   {
#if CH395_DEBUG
       xprintf("SINT_STAT_SEND_OK \n");
#endif
	   sbuflen = 0; //jdb2018-04-20
   }
   #endif
   #if 1 //����Ϊ��ѯ��ʽ
   if(sock_int_socket & SINT_STAT_RECV)                              /* �����ж� */
   {
       len = CH395GetRecvLength(sockindex);                          /* ��ȡ��ǰ�����������ݳ��� */
#if CH395_DEBUG
       xprintf("receive len = %d\n",len);
#endif
       if(len == 0)return;
       if(len > 512)len = 512;                                       /* MyBuffer����������Ϊ512 */
       CH395GetRecvData(sockindex,len,MyBuffer);                     /* ��ȡ���� */
	   buflen = len; //jdb2018-04-02
	   #if 1 //jdb2018-04-02
       for(tmp =0; tmp < len; tmp++)                                 /* ���������ݰ�λȡ�� */
       {
          MyBuffer[tmp] = ~MyBuffer[tmp];
       }
       CH395SendData(sockindex,MyBuffer,len);
	   #endif

   }
   #endif
   #if 1
	if(sock_int_socket & SINT_STAT_CONNECT)                          /* �����жϣ�����TCPģʽ����Ч*/
	{
		#if 0
		CH395SetKeepLive(sockindex,1);								 /*��KEEPALIVE���ʱ��*/
		CH395SetTTLNum(sockindex,40); 								 /*����TTL*/
		#else
		//jdb2018-03-16
    if(SockInf.TcpMode == TCP_SERVER_MODE)             
    {
			CH395CMDGetRemoteIPP(sockindex,buf);
			tmp = (UINT16)(buf[5]<<8) + buf[4];
#if CH395_DEBUG
			xprintf("IP address = %d.%d.%d.%d\n",(UINT16)buf[0],(UINT16)buf[1],(UINT16)buf[2],(UINT16)buf[3]);                    
			xprintf("Port = %d\n",tmp);    
#endif
		}
		#endif
	}
	#endif
   /*
   **�����Ͽ������жϺͳ�ʱ�ж�ʱ��CH395Ĭ���������ڲ������رգ��û�����Ҫ�Լ��رո�Socket����������óɲ������ر�Socket��Ҫ����
   **SOCK_CTRL_FLAG_SOCKET_CLOSE��־λ��Ĭ��Ϊ0��������ñ�־Ϊ1��CH395�ڲ�����Socket���йرմ������û��������жϺͳ�ʱ�ж�ʱ����
   **CH395CloseSocket������Socket���йرգ�������ر����SocketһֱΪ���ӵ�״̬����ʵ���Ѿ��Ͽ������Ͳ�����ȥ�����ˡ�
   */
	if(sock_int_socket & SINT_STAT_DISCONNECT)                        /* �Ͽ��жϣ�����TCPģʽ����Ч */
	{
		//jdb2018-03-16
#if CH395_DEBUG		 
		xprintf("SINT_STAT_DISCONNECT \n");
#endif
		//i = CH395CloseSocket(sockindex);                             
		//mStopIfError(i);
		mDelaymS(200); 
    i = CH395OpenSocket(sockindex);
    mStopIfError(i);
    i = CH395TCPListen (sockindex);                                 
    mStopIfError(i);
	}
	#if 1
   if(sock_int_socket & SINT_STAT_TIM_OUT)                           /* ��ʱ�ж� */
   { 
#if CH395_DEBUG		 
		xprintf("time out \n");
#endif
		 //		i = CH395CloseSocket(sockindex);                             
//		mStopIfError(i);
		//jdb2018-03-16
		if(SockInf.TcpMode == TCP_CLIENT_MODE)             
		{
			mDelaymS(200);                                       
      i = CH395OpenSocket(sockindex);
      mStopIfError(i);
      CH395TCPConnect(sockindex);                        
      mStopIfError(i);
		}
		if(SockInf.TcpMode == TCP_SERVER_MODE)             
		{
			mDelaymS(200); 
			i = CH395OpenSocket(sockindex);
			mStopIfError(i);
			i = CH395TCPListen (sockindex);                                 
			mStopIfError(i);
		}
   }
   #endif
}

//jdb2018-04-02
int GetFromBuf(char *buf, unsigned int size)
{
	int ret = 0;
	char tmpbuf[512] = {0};

	#if 0 //test
	if(buflen > size){
		memcpy(buf, MyBuffer, size);
		buflen -= size;
		memcpy(tmpbuf, MyBuffer+size, buflen);
		memcpy(MyBuffer, tmpbuf, buflen);
		ret = size;
	}else if(buflen > 0){
		memcpy(buf, MyBuffer, buflen);
		ret = buflen;
		buflen = 0;
	}else{
		ret = -1;
	}
	#endif

	return ret;
}

//jdb2018-04-02
void SendData(char *buf, unsigned int len)
{
	while(1){
		if(sbuflen == 0){
			//memcpy(SMyBuffer, buf, len);
			CH395SendData(1,buf,len);
			sbuflen = len;
			break;
		}
	}
	
}

//jdb2018-04-02
int CheckConnect()
{
	int ret = 0;
	
	if(CH395CMDGetPHYStatus() == PHY_DISCONN)
	{
		ret = -1;
	}

	return ret;
}

/**********************************************************************************
* Function Name  : CH395GlobalInterrupt
* Description    : CH395ȫ���жϺ���
* Input          : None
* Output         : None
* Return         : None
**********************************************************************************/
void CH395GlobalInterrupt(void)
{
   UINT16  init_status;
   UINT8   buf[10]; 
 
    init_status = CH395CMDGetGlobIntStatus_ALL();
    if(init_status & GINT_STAT_UNREACH)                              /* ���ɴ��жϣ���ȡ���ɴ���Ϣ */
    {
        CH395CMDGetUnreachIPPT(buf);                                
    }
    if(init_status & GINT_STAT_IP_CONFLI)                            /* ����IP��ͻ�жϣ����������޸�CH395�� IP������ʼ��CH395*/
    {
    }
    if(init_status & GINT_STAT_PHY_CHANGE)                           /* ����PHY�ı��ж�*/
    {
        xprintf("Init status : GINT_STAT_PHY_CHANGE\n");
    }
    if(init_status & GINT_STAT_SOCK0)
    {
        CH395SocketInterrupt(0);                                     /* ����socket 0�ж�*/
    }
    if(init_status & GINT_STAT_SOCK1)                                
    {
        CH395SocketInterrupt(1);                                     /* ����socket 1�ж�*/
    }
    if(init_status & GINT_STAT_SOCK2)                                
    {
        CH395SocketInterrupt(2);                                     /* ����socket 2�ж�*/
    }
    if(init_status & GINT_STAT_SOCK3)                                
    {
        CH395SocketInterrupt(3);                                     /* ����socket 3�ж�*/
    }
    if(init_status & GINT_STAT_SOCK4)
    {
        CH395SocketInterrupt(4);                                     /* ����socket 4�ж�*/
    }
    if(init_status & GINT_STAT_SOCK5)                                
    {
        CH395SocketInterrupt(5);                                     /* ����socket 5�ж�*/
    }
    if(init_status & GINT_STAT_SOCK6)                                
    {
        CH395SocketInterrupt(6);                                     /* ����socket 6�ж�*/
    }
    if(init_status & GINT_STAT_SOCK7)                                
    {
        CH395SocketInterrupt(7);                                     /* ����socket 7�ж�*/
    }
}

/*********************************************************************************
* Function Name  : CH395Init
* Description    : ����CH395��IP,GWIP,MAC�Ȳ���������ʼ��
* Input          : None
* Output         : None
* Return         : ����ִ�н��
**********************************************************************************/
UINT8  CH395Init(void)
{
    UINT8 i;
    
    i = CH395CMDCheckExist(0x65);                      
    if(i != 0x9a)return CH395_ERR_UNKNOW;                            /* �����������޷�ͨ������0XFA */
                                                                     /* ����0XFAһ��ΪӲ��������߶�дʱ�򲻶� */
#if (CH395_OP_INTERFACE_MODE == 5)                                   
#ifdef UART_WORK_BAUDRATE
    CH395CMDSetUartBaudRate(UART_WORK_BAUDRATE);                     /* ���ò����� */   
    mDelaymS(1);
    SetMCUBaudRate();
#endif
#endif
    CH395CMDSetIPAddr(CH395Inf.IPAddr);                              /* ����CH395��IP��ַ */
    CH395CMDSetGWIPAddr(CH395Inf.GWIPAddr);                          /* �������ص�ַ */
    CH395CMDSetMASKAddr(CH395Inf.MASKAddr);                          /* �����������룬Ĭ��Ϊ255.255.255.0*/ 	

    //CH395SetStartPara(FUN_PARA_FLAG_LOW_PWR|SOCK_DISABLE_SEND_OK_INT); //jdb2018-05-16/* CH395������ܺ�ģʽ,����SINT_STAT_SEND_OK�ж�*/
	
    mDelaymS(100);
    i = CH395CMDInitCH395();                                         /* ��ʼ��CH395оƬ */
    return i;
}

//jdb2018-03-16
#if 0
void Ethernet_Start(void)
#else
void Ethernet_Start(UINT8 IP1, UINT8 IP2, UINT8 IP3, UINT8 IP4)
#endif
{
	UINT8 i;
	
	CH395_PORT_INIT();

//jdb2018-03-16
#if 0
	InitCH395InfParam();                                             /* ��ʼ��CH395��ر��� */
#else
	InitCH395InfParam(IP1, IP2, IP3, IP4);
#endif
	i = CH395Init();                                                 /* ��ʼ��CH395оƬ */
	mStopIfError(i);
	//CH395CMDSetPHY(PHY_10M_FLL); //jdb2018-04-23 10Mȫ˫��ģʽ

	#if 1
	keeplive_set(); 												/* ���ʱ����������  */  
	#else
	//jdb2018-03-16
    while(1)
    {               
       if(CH395CMDGetPHYStatus() == PHY_DISCONN)                    
       {
           mDelaymS(200);                                           
       }
       else 
       {
#if CH395_DEBUG
           xprintf("CH395 Connect Ethernet\n");                    
#endif
           break;
       }
    }
	#endif

	//jdb2018-03-16
	InitSocketParam();
	CH395SocketInitOpen();
}


