/*
 * Amazon FreeRTOS V1.4.7
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo includes */
#include "iot_demo_runner.h"
#include "aws_demo_config.h"

/* AWS library includes. */
#include "aws_system_init.h"
#include "aws_logging_task.h"
#include "aws_wifi.h"
#include "aws_clientcredential.h"

#include "aws_dev_mode_key_provisioning.h"

/* Logging Task Defines. */
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 15 )
#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 8 )


/* MediaTek */
#include "sys_init.h"
#include "bsp_gpio_ept_config.h"
#ifdef HAL_GPIO_LOW_POWER_ENABLED
#include "hal_lp.h"
#endif

#define WIFI_FW_DEBUG_LOG_PORT

/**
 * @brief Application task startup hook for applications using Wi-Fi. If you are not
 * using Wi-Fi, then start network dependent applications in the vApplicationIPNetorkEventHook
 * function. If you are not using Wi-Fi, this hook can be disabled by setting
 * configUSE_DAEMON_TASK_STARTUP_HOOK to 0.
 */
void vApplicationDaemonTaskStartupHook( void );

#if ( configUSE_DAEMON_TASK_STARTUP_HOOK != 1 )

/**
 * @brief Wrapper for vApplicationDaemonTaskStartupHook.
 */
    static void vApplicationWrapperTask( void * pvParameters );
#endif

#ifdef MTK_MINICLI_ENABLE

/**
 * @brief CLI task startup function.
 */
    void vCliTaskStartup( void );
#endif

/**
 * @brief Connects to WiFi.
 */
static void prvWifiConnect( void );

/**
 * @brief Initializes the board.
 */
static void prvMiscInitialization( void );

/*-----------------------------------------------------------*/

/**
 * @brief Application runtime entry point.
 */
int main( void )
{
    /* Perform any hardware initialization that does not require the RTOS to be
     * running.  */
    prvMiscInitialization();

    /* console print can be used after console UART ports have been configured
     * in prvMiscInitialization(). */

#if ( configUSE_DAEMON_TASK_STARTUP_HOOK != 1 )
        configASSERT( xTaskCreate( vApplicationWrapperTask,
                                   "wrap",
                                   configTIMER_TASK_STACK_DEPTH,
                                   NULL,
                                   configTIMER_TASK_PRIORITY,
                                   NULL ) == pdPASS );
#endif

    /* Create tasks that are not dependent on the WiFi being initialized. */
    xLoggingTaskInitialize( mainLOGGING_TASK_STACK_SIZE,
                            tskIDLE_PRIORITY + 2,
                            mainLOGGING_MESSAGE_QUEUE_LENGTH );

    /* Start the scheduler.  Initialization that requires the OS to be running,
     * including the WiFi initialization, is performed in the RTOS daemon task
     * startup hook. */
    vTaskStartScheduler();

    return 0;
}

/*-----------------------------------------------------------*/
static void prvMiscInitialization( void )
{
    /* Do system initialization, eg: hardware, nvdm, logging and random seed. */
    system_init();

    /* init GPIO settings generated by easy pinmux tool (ept). */
    bsp_ept_gpio_setting_init();

    #ifdef HAL_GPIO_LOW_POWER_ENABLED
        hal_lp_set_gpio_sleep_mode();
    #endif

    #ifdef WIFI_FW_DEBUG_LOG_PORT
        volatile unsigned int reg;

        reg = *( ( volatile unsigned int * ) 0x81023030 );
        reg &= 0xFFFF0FFF;
        *( ( volatile unsigned int * ) ( 0x81023030 ) ) = reg;

        reg = *( ( volatile unsigned int * ) 0x80025110 );
        reg &= 0xFFFF0FFF;
        *( ( volatile unsigned int * ) 0x80025110 ) = reg;
    #endif
}

/*-----------------------------------------------------------*/

#if ( configUSE_DAEMON_TASK_STARTUP_HOOK != 1 )
    static void vApplicationWrapperTask( void * pvParameters )
    {
        #ifdef MTK_MINICLI_ENABLE
            vCliTaskStartup();
        #endif

        vApplicationDaemonTaskStartupHook();

        vTaskDelete( NULL );
    }
#endif /* if ( configUSE_DAEMON_TASK_STARTUP_HOOK != 1 ) */

/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
    configPRINTF( ( "Starting up\n" ) );
    /* A simple example to demonstrate key and certificate provisioning in
     * microcontroller flash using PKCS#11 interface. This should be replaced
     * by production ready key provisioning mechanism. */
    vDevModeKeyProvisioning();

    if( SYSTEM_Init() == pdPASS )
    {
        /* Connect to the wifi before running real jobs. */
        prvWifiConnect();

        /* Run all demos. */
        DEMO_RUNNER_RunDemos();
    }
    else
    {
        configPRINTF( ( "SYSTEM_Init failed\n" ) );
    }
}

/*-----------------------------------------------------------*/

void prvWifiConnect( void )
{
    WIFINetworkParams_t xJoinAPParams;
    WIFIReturnCode_t xWifiStatus;

    xWifiStatus = WIFI_On();

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "WiFi module initialized. Connecting to AP...\r\n" ) );
    }
    else
    {
        configPRINTF( ( "WiFi module failed to initialize.\r\n" ) );

        while( 1 )
        {
        }
    }

    /* Setup parameters. */
    xJoinAPParams.pcSSID = clientcredentialWIFI_SSID;
    xJoinAPParams.ucSSIDLength = sizeof( clientcredentialWIFI_SSID );
    xJoinAPParams.pcPassword = clientcredentialWIFI_PASSWORD;
    xJoinAPParams.ucPasswordLength = sizeof( clientcredentialWIFI_PASSWORD );
    xJoinAPParams.xSecurity = clientcredentialWIFI_SECURITY;

    xWifiStatus = WIFI_ConnectAP( &( xJoinAPParams ) );

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "WiFi Connected to AP. Creating tasks which use network...\r\n" ) );
    }
    else
    {
        configPRINTF( ( "WiFi failed to connect to AP.\r\n" ) );

        while( 1 )
        {
            taskYIELD();
        }
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief User defined Idle task function.
 *
 * @note Do not make any blocking operations in this function.
 */
void vApplicationIdleHook( void )
{
    static TickType_t xLastPrint = 0;
    TickType_t xTimeNow;
    const TickType_t xPrintFrequency = pdMS_TO_TICKS( 5000 );

    xTimeNow = xTaskGetTickCount();

    if( ( xTimeNow - xLastPrint ) > xPrintFrequency )
    {
        configPRINTF( ( "." ) );
        xLastPrint = xTimeNow;
    }
}
