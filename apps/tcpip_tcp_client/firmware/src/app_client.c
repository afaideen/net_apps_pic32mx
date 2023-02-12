

#include "system_definitions.h"
#include "peripheral/gpio/plib_gpio.h"

void APP_CLIENT_Initialize ( void );
void APP_CLIENT_Tasks ( void );

#define MY_SERVER_IP_ADDR_BYTE1        192ul
#define MY_SERVER_IP_ADDR_BYTE2        168ul
#define MY_SERVER_IP_ADDR_BYTE3        137ul
#define MY_SERVER_IP_ADDR_BYTE4        1ul


typedef struct
{
    /* The application's current state */
    APP_STATES state;

    /* TODO: Define any additional data used by the application. */
    TCP_SOCKET              socket;

    char *            host;
    char             hostname[20];

    char *            path;
    char              pathname[20];
    uint16_t          port;

} APP_DATA_CLIENT;

APP_DATA_CLIENT appDataClient;
IPV4_ADDR serverAddr;
void APP_CLIENT_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appDataClient.state = APP_STATE_INIT;
//    strcpy(appDataClient.hostname, "192.168.137.1");
//    appDataClient.host = &appDataClient.hostname[0];
//    memset((void*)appDataClient.path, 0, 1);
    serverAddr.Val = MY_SERVER_IP_ADDR_BYTE1 | MY_SERVER_IP_ADDR_BYTE2<<8ul | MY_SERVER_IP_ADDR_BYTE3<<16ul | MY_SERVER_IP_ADDR_BYTE4<<24ul;
    TCPIP_Helper_IPAddressToString((IPV4_ADDR*)&serverAddr, (char*)appDataClient.hostname, sizeof(appDataClient.hostname));
    appDataClient.host = &appDataClient.hostname[0];
    strcpy(appDataClient.pathname, "testing");
    appDataClient.path = &appDataClient.pathname[0];
    Nop();

}

void APP_CLIENT_Tasks ( void )
{
     static uint32_t     startTick = 0, t_sw = 0;
    char buffer[255];
   
    
    
    if(SYS_TMR_TickCountGet() - startTick >= SYS_TMR_TickCounterFrequencyGet()/2ul)
    {
                startTick = SYS_TMR_TickCountGet();
//                BSP_LEDToggle(1);
                LED1_Toggle();
                
    }
//    if(!BSP_SwitchStateGet(0)){
    if(!SWITCH1_Get()){
        if(SYS_TMR_TickCountGet() - t_sw >= SYS_TMR_TickCounterFrequencyGet() * 0.5)
        {
            t_sw = SYS_TMR_TickCountGet();
            appDataClient.state = APP_STATE_INIT;
            
        }
    }
    else
        t_sw = SYS_TMR_TickCountGet();
    switch(appDataClient.state)
    {
        case APP_STATE_INIT:
            
            appDataClient.socket = TCPIP_TCP_ClientOpen(IP_ADDRESS_TYPE_IPV4, 5000,(IP_MULTI_ADDRESS*) &serverAddr);
            if (appDataClient.socket == INVALID_SOCKET)
            {
//                SYS_CONSOLE_MESSAGE("Could not start connection with server 192.168.137.1\r\n");
                SYS_CONSOLE_PRINT("Could not start connection with server %s\r\n", appDataClient.hostname);
                appDataClient.state = APP_TCPIP_WAITING_FOR_COMMAND;
            }
            SYS_CONSOLE_PRINT("Starting connection...server %s\r\n", appDataClient.hostname);
            appDataClient.state = APP_TCPIP_WAIT_FOR_CONNECTION;
            TCPIP_TCP_WasReset(appDataClient.socket);     // clear the reset flag
            break;
        
//        case APP_TCPIP_WAITING_FOR_COMMAND:
//            
//            break;
        case APP_TCPIP_WAIT_FOR_CONNECTION:
            
            if (!TCPIP_TCP_IsConnected(appDataClient.socket))
            {
                break;
            }
            if(TCPIP_TCP_PutIsReady(appDataClient.socket) == 0)
            {
                break;
            }
//            BSP_LEDOn(2);
//            LED2_Set();
            LED2_On();
            sprintf(buffer, "GET /%s HTTP/1.1\r\n"
                    "Host: %s\r\n"
                    "Connection: close\r\n\r\n", appDataClient.path, appDataClient.host);
            TCPIP_TCP_ArrayPut(appDataClient.socket, (uint8_t*)buffer, strlen(buffer));
            appDataClient.state = APP_TCPIP_WAIT_FOR_RESPONSE;
            break;
        case APP_TCPIP_WAIT_FOR_RESPONSE:
            memset(buffer, 0, sizeof(buffer));
            while (TCPIP_TCP_GetIsReady(appDataClient.socket))
            {
                TCPIP_TCP_ArrayGet(appDataClient.socket, (uint8_t*)buffer, sizeof(buffer) - 1);
                SYS_CONSOLE_PRINT("%s", buffer);
            }

//            if (!TCPIP_TCP_IsConnected(appDataClient.socket) || TCPIP_TCP_WasDisconnected(appDataClient.socket))
            if (!TCPIP_TCP_IsConnected(appDataClient.socket) )
            {
                SYS_CONSOLE_MESSAGE("\r\nConnection Closed\r\n");
                TCPIP_TCP_Close(appDataClient.socket);
                appDataClient.socket = INVALID_SOCKET;
                appDataClient.state = APP_TCPIP_WAITING_FOR_COMMAND;
                break;
            }
            break;
        default:
            break;
    }
    
    
}