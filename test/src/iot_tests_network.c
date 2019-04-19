/*
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
 */

/**
 * @file iot_tests_network.c
 * @brief Common network function implementations for the tests.
 */

/* The config header is always included first. */
#include "iot_config.h"

#include "private/iot_error.h"
#include "stdbool.h"

static uint16_t _IotTestNetworkType = AWSIOT_NETWORK_TYPE_WIFI;
static bool bleEnabled = false;
/*-----------------------------------------------------------*/



#if !defined( WIFI_SUPPORTED ) || ( WIFI_SUPPORTED != 0 )
    #include "platform/iot_network_afr.h"
    #include "private/iot_mqtt_internal.h"
    static const IotMqttSerializer_t _mqttSerializer =
    {
        .getPacketType      = _IotMqtt_GetPacketType,
        .getRemainingLength = _IotMqtt_GetRemainingLength,
        .freePacket         = _IotMqtt_FreePacket,
        .serialize          =
        {
            .connect        = _IotMqtt_SerializeConnect,
            .publish        = _IotMqtt_SerializePublish,
            .publishSetDup  = _IotMqtt_PublishSetDup,
            .puback         = _IotMqtt_SerializePuback,
            .subscribe      = _IotMqtt_SerializeSubscribe,
            .unsubscribe    = _IotMqtt_SerializeUnsubscribe,
            .pingreq        = _IotMqtt_SerializePingreq,
            .disconnect     = _IotMqtt_SerializeDisconnect
        },
        .deserialize        =
        {
            .connack        = _IotMqtt_DeserializeConnack,
            .publish        = _IotMqtt_DeserializePublish,
            .puback         = _IotMqtt_DeserializePuback,
            .suback         = _IotMqtt_DeserializeSuback,
            .unsuback       = _IotMqtt_DeserializeUnsuback,
            .pingresp       = _IotMqtt_DeserializePingresp
        }
    };
#endif /* if ( WIFI_SUPPORTED == 1 ) */

#if ( BLE_SUPPORTED == 1 )
    #include "iot_ble_mqtt.h"
    /*-----------------------------------------------------------*/

    static void _BLEConnectionCallback( BTStatus_t status,
                                        uint16_t connId,
                                        bool connected,
                                        BTBdaddr_t * pBa )
    {
        if( connected == true )
        {
            IotBle_StopAdv();
        }
        else
        {
            ( void ) IotBle_StartAdv( NULL );
        }
    }
    /*-----------------------------------------------------------*/

    static BaseType_t _BLEEnable( void )
    {
        IotBleEventsCallbacks_t xEventCb;
        BaseType_t xRet = pdTRUE;
        static bool bInitBLE = false;
        BTStatus_t xStatus;

        if( bInitBLE == false )
        {
            xStatus = IotBle_Init();

            if( xStatus == eBTStatusSuccess )
            {
                bInitBLE = true;
            }
        }
        else
        {
            xStatus = IotBle_On();
        }

        /* Register BLE Connection callback */
        if( xStatus == eBTStatusSuccess )
        {
            xEventCb.pConnectionCb = _BLEConnectionCallback;

            if( IotBle_RegisterEventCb( eBLEConnection, xEventCb ) != eBTStatusSuccess )
            {
                xStatus = eBTStatusFail;
            }
        }

        if( xStatus != eBTStatusSuccess )
        {
            xRet = pdFALSE;
        }

        return xRet;
    }

/*-----------------------------------------------------------*/

    static BaseType_t _BLEDisable( void )
    {
        bool ret = true;
        IotBleEventsCallbacks_t xEventCb;

        xEventCb.pConnectionCb = _BLEConnectionCallback;

        if( IotBle_UnRegisterEventCb( eBLEConnection, xEventCb ) != eBTStatusSuccess )
        {
            ret = false;
        }

        if( ret == true )
        {
            if( IotBle_StopAdv() != eBTStatusSuccess )
            {
                ret = false;
            }
        }

        if( ret == true )
        {
            if( IotBle_Off() != eBTStatusSuccess )
            {
                ret = false;
            }
        }

        return ret;
    }
#endif /* if ( BLE_SUPPORTED == 1 ) */


/*-----------------------------------------------------------*/

const IotNetworkInterface_t * IotTestNetwork_GetNetworkInterface( void )
{
	const IotNetworkInterface_t * pNetworkInterface = NULL;

    switch( _IotTestNetworkType )
    {
        #if ( BLE_SUPPORTED == 1 )
            case AWSIOT_NETWORK_TYPE_BLE:
                pNetworkInterface = ( IotNetworkInterface_t * ) &IotNetworkBle;
                break;
        #endif
        #if !defined( WIFI_SUPPORTED ) || ( WIFI_SUPPORTED != 0 )
            case AWSIOT_NETWORK_TYPE_WIFI:
                pNetworkInterface = IOT_NETWORK_INTERFACE_AFR;
                break;
        #endif
        default:
            break;
    }

    return pNetworkInterface;
}

/*-----------------------------------------------------------*/

void IotTestNetwork_SelectNetworkType( uint16_t networkType )
{
    switch( networkType )
    {
        #if ( BLE_SUPPORTED == 1 )
            case AWSIOT_NETWORK_TYPE_BLE:
            	if( bleEnabled == false)
            	{
            		bleEnabled = true;
            		_BLEEnable();
            	}
                break;
        #endif
        #if !defined( WIFI_SUPPORTED ) || ( WIFI_SUPPORTED != 0 )
            case AWSIOT_NETWORK_TYPE_WIFI:
                break;
        #endif
        default:
            break;
    }

    _IotTestNetworkType = networkType;
}

/*-----------------------------------------------------------*/

const IotMqttSerializer_t * IotTestNetwork_GetSerializer( void )
{
	const IotMqttSerializer_t * pSerializer = NULL;

    switch( _IotTestNetworkType )
    {
        #if ( BLE_SUPPORTED == 1 )
            case AWSIOT_NETWORK_TYPE_BLE:
            	pSerializer = &IotBleMqttSerializer;
                break;
        #endif
        #if !defined( WIFI_SUPPORTED ) || ( WIFI_SUPPORTED != 0 )
            case AWSIOT_NETWORK_TYPE_WIFI:
            	pSerializer = &_mqttSerializer;
                break;
        #endif
        default:
            break;
    }
    return ( IotMqttSerializer_t * ) pSerializer;
}
