/*
 * Amazon FreeRTOS OTA V1.0.3
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


/* Standard library include. */
#include <stdbool.h>
#include <string.h>

/* Error handling from C-SDK. */
#include "iot_error.h"

/* HTTP includes. */
#include "iot_https_client.h"
#include "iot_https_utils.h"

/* OTA includes. */
#include "aws_iot_ota_agent.h"
#include "aws_iot_ota_agent_internal.h"

/* Logging includes. */
#ifdef IOT_LOG_LEVEL_GLOBAL
    #define LIBRARY_LOG_LEVEL               IOT_LOG_LEVEL_GLOBAL
#else
    #define LIBRARY_LOG_LEVEL               IOT_LOG_INFO
#endif
#define LIBRARY_LOG_NAME                    ( "OTA" )
#include "iot_logging_setup.h"

/* Jump to cleanup section. */
#define OTA_GOTO_CLEANUP()                  IOT_GOTO_CLEANUP()

/* Start of the cleanup section. */
#define OTA_FUNCTION_CLEANUP_BEGIN()        IOT_FUNCTION_CLEANUP_BEGIN()

/* End of the cleanup section. */
#define OTA_FUNCTION_CLEANUP_END()

/* Empty cleanup section. */
#define OTA_FUNCTION_NO_CLEANUP()           OTA_FUNCTION_CLEANUP_BEGIN(); OTA_FUNCTION_CLEANUP_END()

/* Maximum OTA file size string in byte. */
#define OTA_MAX_FILE_SIZE_STR               "16777216"

/* String length of the maximum OTA file size string, not including the null character. */
#define OTA_MAX_FILE_SIZE_STR_LEN           sizeof( OTA_MAX_FILE_SIZE_STR ) - 1

/* TLS port for HTTPS. */
#define HTTPS_PORT                          ( ( uint16_t ) 443 )

/* Baltimore Cybertrust associated with the S3 server certificate. */
#define HTTPS_TRUSTED_ROOT_CA                               \
"-----BEGIN CERTIFICATE-----\n"                                      \
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n"                             \
"-----END CERTIFICATE-----\n"

/* Buffer size for HTTP connection context. This is the minimum size from HTTP library, we cannot
 * use it directly because it's only availble at runtime. */
#define HTTPS_CONNECTION_USER_BUFFER_SIZE           256

/* Buffer size for HTTP request context and header.*/
#define HTTPS_REQUEST_USER_BUFFER_SIZE              2048

/* Buffer size for HTTP reponse context and header.*/
#define HTTPS_RESPONSE_USER_BUFFER_SIZE             1024

/* Buffer size for HTTP reponse body.*/
#define HTTPS_RESPONSE_BODY_BUFFER_SIZE             OTA_FILE_BLOCK_SIZE

/* Default timeout for HTTP synchronous request. */
#define HTTP_SYNC_TIMEOUT                           3000

/**
 * The maximum length of the "Range" field in HTTP header.
 *
 * The maximum file size of OTA download is OTA_MAX_FILE_SIZE_STR bytes. The header value string is
 * of the form: "bytes=N-M". So the length is len("bytes=-") + len(N) + len(M) + the NULL terminator.
 * The maximum length is 7 + OTA_MAX_FILE_SIZE_STR_LEN * 2 + 1.
 */
#define HTTP_HEADER_RANGE_VALUE_MAX_LEN             ( 7 + ( OTA_MAX_FILE_SIZE_STR_LEN ) * 2 + 1)

/**
 * The maximum length of the "Connection" field in HTTP header.
 *
 * The value could be "close" or "keep-alive", so the maximum length is sizeof("keep-alive"), this
 * includes the NULL terminator.
 */
#define HTTP_HEADER_CONNECTION_VALUE_MAX_LEN        ( sizeof( "keep-alive" ) )

/* Struct for HTTP callback data. */
typedef struct
{
    char pRangeValueStr[ HTTP_HEADER_RANGE_VALUE_MAX_LEN ]; /* Buffer to write the HTTP "range" header value string. */
} _httpCallbackData_t;

/* Struct for HTTP connection configuration and handle. */
typedef struct
{
    IotHttpsConnectionInfo_t connectionConfig;      /* Configurations for the HTTPS connection. */
    IotHttpsConnectionHandle_t connectionHandle;    /* Handle identifying the HTTPS connection. */
} _httpConnection_t;

/* Struct for HTTP request configuration and handle. */
typedef struct
{
    IotHttpsAsyncInfo_t asyncInfo;                      /* Asynchronous request configurations. */
    IotHttpsRequestInfo_t requestConfig;                /* Configurations for the HTTPS request. */
    IotHttpsRequestHandle_t requestHandle;              /* Handle identifying the HTTP request. */
} _httpRequest_t;

/* Struct for HTTP response configuration and handle. */
typedef struct
{
    IotHttpsResponseInfo_t responseConfig;              /* Configurations for the HTTPS response. */
    IotHttpsResponseHandle_t responseHandle;            /* Handle identifying the HTTP response. */
} _httpResponse_t;

/* Struct for HTTP download information. */
typedef struct
{
    const char * pPath;         /* Resrouce path to the firmware in HTTP URL. */
    size_t pathLength;          /* Length of the resrouce path. */
    const char * pAddress;      /* Address to the server in HTTP URL. */
    size_t addressLength;       /* Length of the address. */
} _httpUrlInfo_t;

/* Struct to keep track of the internal HTTP downloader state. */
typedef enum
{
    OTA_HTTP_OK = 0,
    OTA_HTTP_GENERIC_ERR = 101,
    OTA_HTTP_NEED_RECONNECT = 102,
    OTA_HTTP_URL_EXPIRED = 103,
} _httpState;

/* Struct for OTA HTTP downloader. */
typedef struct
{
    _httpState state;                           /* HTTP downloader state. */
    _httpUrlInfo_t httpUrlInfo;                 /* HTTP url of the file to download. */
    _httpConnection_t httpConnection;           /* HTTP connection data. */
    _httpRequest_t httpRequest;                 /* HTTP request data. */
    _httpResponse_t httpResponse;               /* HTTP response data. */
    _httpCallbackData_t httpCallbackData;       /* Data used in the HTTP callback. */
    bool isDownloading;                         /* Flag to keep track of the status. */
    uint32_t currBlock;                         /* Current requesting block in bitmap. */
    uint32_t currBlockSize;                     /* Size of current requesting block. */
    OTA_AgentContext_t * pAgentCtx;             /* OTA agent context. */
} _httpDownloader_t;

/* Global HTTP downloader instance. */
static _httpDownloader_t _httpDownloader = { 0 };

/* Buffers for HTTP library. */
uint8_t pConnectionUserBuffer[ HTTPS_CONNECTION_USER_BUFFER_SIZE ];     /* Buffer to store the HTTP connection context. */
uint8_t pRequestUserBuffer[ HTTPS_REQUEST_USER_BUFFER_SIZE ];           /* Buffer to store the HTTP request context and header. */
uint8_t pResponseUserBuffer[ HTTPS_RESPONSE_USER_BUFFER_SIZE ];         /* Buffer to store the HTTP response context and header. */
uint8_t pResponseBodyBuffer[ HTTPS_RESPONSE_BODY_BUFFER_SIZE ];         /* Buffer to store the HTTP response body. */


/*-----------------------------------------------------------*/

/* Process the HTTP response body, copy to another buffer and signal OTA agent the file block
 * download is complete. */
static void _httpProcessResponseBody( OTA_AgentContext_t * pAgentCtx, uint8_t * pResponseBodyBuffer, uint32_t bufferSize )
{
    IotLogDebug( "Invoking _httpProcessResponseBody" );

    OTAEventData_t * pMessage;
    OTAEventMsg_t eventMsg = { 0 };

    if( pAgentCtx->xOTA_EventQueue == NULL )
    {
        IotLogWarn( "Event queue for OTA agent task is NULL." );
        pAgentCtx->xStatistics.ulOTA_PacketsDropped++;
        OTA_GOTO_CLEANUP();
    }

    pAgentCtx->xStatistics.ulOTA_PacketsReceived++;

    /* Try to get OTA data buffer. */
    pMessage = prvOTAEventBufferGet();
    if( pMessage == NULL )
    {
        pAgentCtx->xStatistics.ulOTA_PacketsDropped++;
        IotLogError( "Could not get a free buffer to copy callback data." );
    }
    else
    {
        pMessage->ulDataLength = bufferSize;

        memcpy( pMessage->ucData, pResponseBodyBuffer, pMessage->ulDataLength );
        eventMsg.xEventId = eOTA_AgentEvent_ReceivedFileBlock;
        eventMsg.pxEventData = pMessage;
        /* Send job document received event. */
        OTA_SignalEvent( &eventMsg );
    }

    OTA_FUNCTION_NO_CLEANUP();
}

/* Error handler for HTTP response code. */
static void _httpErrorHandler( uint16_t httpResponseCode )
{
    if( httpResponseCode == IOT_HTTPS_STATUS_FORBIDDEN )
    {
        IotLogInfo( "Pre-signed URL may have expired, requesting new job document." );
        _httpDownloader.state = OTA_HTTP_URL_EXPIRED;
    }
    else
    {
        _httpDownloader.state = OTA_HTTP_GENERIC_ERR;
    }
}

/* HTTP async callback for adding header to HTTP request, called after IotHttpsClient_SendAsync. */
static void _httpAppendHeaderCallback( void * pPrivateData,
                                       IotHttpsRequestHandle_t requestHandle )
{
    IotLogDebug( "Invoking _httpAppendHeaderCallback." );

    /* Value of the "Range" field in HTTP GET request header, set when requesting the file block. */
    char * pRangeValueStr = ( ( _httpCallbackData_t * ) ( pPrivateData ) )->pRangeValueStr;

    /* Set the header for this range request. */
    IotHttpsReturnCode_t status = IotHttpsClient_AddHeader( requestHandle,
                                                            "Range",
                                                            sizeof( "Range" ) - 1,
                                                            pRangeValueStr,
                                                            strlen( pRangeValueStr ) );
    if( status != IOT_HTTPS_OK )
    {
        IotLogError( "Failed to add HTTP header. Error code: %d. Canceling current request.", status );
        IotHttpsClient_CancelRequestAsync( requestHandle );
        _httpDownloader.state = OTA_HTTP_GENERIC_ERR;
    }
}

static void _httpReadReadyCallback( void * pPrivateData,
                                    IotHttpsResponseHandle_t responseHandle,
                                    IotHttpsReturnCode_t returnCode,
                                    uint16_t responseStatus )
{
    IotLogDebug( "Invoking _httpReadReadyCallback." );

    /* HTTP callback data is not used. */
    ( void ) pPrivateData;

    /* HTTP return status. */
    IotHttpsReturnCode_t httpsStatus = IOT_HTTPS_OK;

    /* HTTP connection data. */
    _httpConnection_t * pConnection = &_httpDownloader.httpConnection;

    /* The content length of this HTTP response. */
    uint32_t contentLength = 0;

    /* Size of the response body returned from HTTP API. */
    uint32_t responseBodyLength = 0;

    /* Buffer to read the "Connection" field in HTTP header. */
    char connectionValueStr[ HTTP_HEADER_CONNECTION_VALUE_MAX_LEN ] = { 0 };

    /* The HTTP response should be partial content with response code 206. */
    if( responseStatus != IOT_HTTPS_STATUS_PARTIAL_CONTENT )
    {
        IotLogError( "Expect a HTTP partial response, but received code %d", responseStatus );
        _httpErrorHandler( responseStatus );
        OTA_GOTO_CLEANUP();
    }

    /* Read the "Content-Length" field from HTTP header. */
    httpsStatus = IotHttpsClient_ReadContentLength( responseHandle, &contentLength );
    if( ( httpsStatus != IOT_HTTPS_OK ) || ( contentLength == 0 ) )
    {
        IotLogError( "Failed to retrieve the Content-Length from the response. " );
        _httpDownloader.state = OTA_HTTP_GENERIC_ERR;
        OTA_GOTO_CLEANUP();
    }

    /* Check if the value of "Content-Length" matches what we have requested. */
    if( contentLength != _httpDownloader.currBlockSize )
    {
        IotLogError( "Content-Length value in HTTP header does not match what we requested. " );
        _httpDownloader.state = OTA_HTTP_GENERIC_ERR;
        OTA_GOTO_CLEANUP();
    }

    /* Read the data from the network. */
    responseBodyLength = sizeof( pResponseBodyBuffer );
    httpsStatus = IotHttpsClient_ReadResponseBody( responseHandle,
                                                   pResponseBodyBuffer,
                                                   &responseBodyLength );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Failed to read the response body. Error code: %d.", httpsStatus );
        _httpDownloader.state = OTA_HTTP_GENERIC_ERR;
        OTA_GOTO_CLEANUP();
    }

    /* The connection could be closed by S3 after 100 requests, so we need to check the value
     * of the "Connection" filed in HTTP header to see if we need to reconnect. */
    memset( connectionValueStr, 0, sizeof( connectionValueStr ) );
    httpsStatus = IotHttpsClient_ReadHeader( responseHandle,
                                             "Connection",
                                             sizeof( "Connection") - 1 ,
                                             connectionValueStr,
                                             sizeof( connectionValueStr ) );

    /* Exit if there is any other error besides not found when parsing the http header. */
    if( ( httpsStatus != IOT_HTTPS_OK ) && ( httpsStatus != IOT_HTTPS_NOT_FOUND ) )
    {
        IotLogError( "Failed to read header Connection. Error code: %d.", httpsStatus );
        _httpDownloader.state = OTA_HTTP_GENERIC_ERR;
        OTA_GOTO_CLEANUP();
    }

    /* Check if the server returns a response with connection field set to "close". */
    if( strncmp( "close", connectionValueStr, sizeof( "close" ) ) == 0 )
    {
        /* Reconnect. */
        httpsStatus = IotHttpsClient_Connect( &pConnection->connectionHandle, &pConnection->connectionConfig );

        if( httpsStatus != IOT_HTTPS_OK )
        {
            IotLogError( "Failed to reconnect to the S3 server. Error code: %d.", httpsStatus );
            _httpDownloader.state = OTA_HTTP_NEED_RECONNECT;
            OTA_GOTO_CLEANUP();
        }
    }

    OTA_FUNCTION_NO_CLEANUP();
}

static void _httpResponseCompleteCallback( void * pPrivateData,
                                           IotHttpsResponseHandle_t responseHandle,
                                           IotHttpsReturnCode_t returnCode,
                                           uint16_t responseStatus )
{
    IotLogDebug( "Invoking _httpResponseCompleteCallback." );

    /* HTTP callback data is not used. */
    ( void ) pPrivateData;

    /* OTA Event. */
    OTAEventMsg_t eventMsg = { 0 };

    if( _httpDownloader.state == OTA_HTTP_GENERIC_ERR )
    {
        IotLogError( "Fail to download block %d. ", _httpDownloader.currBlock );
        OTA_GOTO_CLEANUP();
    }
    if( _httpDownloader.state == OTA_HTTP_NEED_RECONNECT )
    {
        eventMsg.xEventId = eOTA_AgentEvent_StartFileTransfer;
        IotLogInfo( "Reconnection required for HTTP, going back to init file transfer." );
        OTA_SignalEvent( &eventMsg );
        OTA_GOTO_CLEANUP();
    }
    if( _httpDownloader.state == OTA_HTTP_URL_EXPIRED )
    {
        eventMsg.xEventId = eOTA_AgentEvent_RequestJobDocument;
        IotLogInfo( "Requesting a new job document." );
        OTA_SignalEvent( &eventMsg );
        OTA_GOTO_CLEANUP();
    }

    /* Process the HTTP response body. */
    _httpProcessResponseBody( _httpDownloader.pAgentCtx, pResponseBodyBuffer, _httpDownloader.currBlockSize );

    OTA_FUNCTION_NO_CLEANUP();
}

static void _httpErrorCallback( void * pPrivateData,
                                IotHttpsRequestHandle_t requestHandle,
                                IotHttpsResponseHandle_t responseHandle,
                                IotHttpsReturnCode_t returnCode )
{
    IotLogDebug( "Invoking _httpErrorCallback." );
    IotLogError( "An error occurred for HTTP async request: %d", returnCode );
}

static void _httpConnectionClosedCallback( void * pPrivateData,
                                           IotHttpsConnectionHandle_t connectionHandle,
                                           IotHttpsReturnCode_t returnCode)
{
    IotLogDebug( "Invoking _httpConnectionClosedCallback." );

    /* HTTP API return status. */
    IotHttpsReturnCode_t httpsStatus = IOT_HTTPS_OK;

    /* HTTP callback data is not used. */
    ( void ) pPrivateData;

    IotLogInfo( "Connection has been closed by the HTTP client due to an error, reconnecting to server..." );

    httpsStatus = IotHttpsClient_Connect( &connectionHandle, &_httpDownloader.httpConnection.connectionConfig );

    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Failed to reconnect to the S3 server. Error code: %d.", httpsStatus );
        _httpDownloader.state = OTA_HTTP_NEED_RECONNECT;
    }

}

static IotHttpsReturnCode_t _httpConnect( const char * pURL,
                                          const IotNetworkInterface_t * pNetworkInterface,
                                          IotNetworkCredentials_t * pNetworkCredentials )
{
    /* HTTP API return status. */
    IotHttpsReturnCode_t httpsStatus = IOT_HTTPS_OK;

    /* HTTP connection data. */
    _httpConnection_t * pConnection = &_httpDownloader.httpConnection;

    /* HTTP connection configuration. */
    IotHttpsConnectionInfo_t * pConnectionConfig = &pConnection->connectionConfig;

    /* HTTP request data. */
    _httpRequest_t * pRequest = &_httpDownloader.httpRequest;

    /* HTTP response data. */
    _httpResponse_t * pResponse = &_httpDownloader.httpResponse;

    /* HTTP URL information. */
    _httpUrlInfo_t * pUrlInfo = &_httpDownloader.httpUrlInfo;

    /* Retrieve the resource path from the HTTP URL. pPath will point to the start of this part. */
    httpsStatus = IotHttpsClient_GetUrlPath( pURL,
                                             strlen( pURL ),
                                             &pUrlInfo->pPath,
                                             &pUrlInfo->pathLength );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to parse the resource path from given HTTP URL. Error code: %d.", httpsStatus );
        OTA_GOTO_CLEANUP();
    }
    /* pathLength is set to the length of path component, but we also need the query part that
     * comes after that. */
    pUrlInfo->pathLength = strlen(pUrlInfo->pPath);

    /* Retrieve the authority part and length from the HTTP URL. */
    httpsStatus = IotHttpsClient_GetUrlAddress( pURL,
                                                strlen( pURL ),
                                                &pUrlInfo->pAddress,
                                                &pUrlInfo->addressLength );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to parse the server address from given HTTP URL. Error code: %d.", httpsStatus );
        OTA_GOTO_CLEANUP();
    }

    /* Set the connection configurations. */
    pConnectionConfig->pAddress = pUrlInfo->pAddress;
    pConnectionConfig->addressLen = pUrlInfo->addressLength;
    pConnectionConfig->port = HTTPS_PORT;
    pConnectionConfig->pCaCert = HTTPS_TRUSTED_ROOT_CA;
    pConnectionConfig->caCertLen = sizeof( HTTPS_TRUSTED_ROOT_CA );
    pConnectionConfig->userBuffer.pBuffer = pConnectionUserBuffer;
    pConnectionConfig->userBuffer.bufferLen = sizeof( pConnectionUserBuffer );
    pConnectionConfig->pClientCert = pNetworkCredentials->pClientCert;
    pConnectionConfig->clientCertLen = pNetworkCredentials->clientCertSize;
    pConnectionConfig->pPrivateKey = pNetworkCredentials->pPrivateKey;
    pConnectionConfig->privateKeyLen = pNetworkCredentials->privateKeySize;
    pConnectionConfig->pNetworkInterface = pNetworkInterface;

    /* Initialize HTTP request configuration. */
    pRequest->requestConfig.pPath = pUrlInfo->pPath;
    pRequest->requestConfig.pathLen = pUrlInfo->pathLength;
    pRequest->requestConfig.pHost = pUrlInfo->pAddress;
    pRequest->requestConfig.hostLen = pUrlInfo->addressLength;
    pRequest->requestConfig.method = IOT_HTTPS_METHOD_GET;
    pRequest->requestConfig.userBuffer.pBuffer = pRequestUserBuffer;
    pRequest->requestConfig.userBuffer.bufferLen = sizeof( pRequestUserBuffer );
    pRequest->requestConfig.isAsync = true;
    pRequest->requestConfig.u.pAsyncInfo = &pRequest->asyncInfo;

    /* Initialize HTTP response configuration. */
    pResponse->responseConfig.userBuffer.pBuffer = pResponseUserBuffer;
    pResponse->responseConfig.userBuffer.bufferLen = sizeof( pResponseUserBuffer );
    pResponse->responseConfig.pSyncInfo = NULL;

    /* Initialize HTTP asynchronous configuration. */
    pRequest->asyncInfo.callbacks.appendHeaderCallback = _httpAppendHeaderCallback;
    pRequest->asyncInfo.callbacks.readReadyCallback = _httpReadReadyCallback;
    pRequest->asyncInfo.callbacks.responseCompleteCallback = _httpResponseCompleteCallback;
    pRequest->asyncInfo.callbacks.errorCallback = _httpErrorCallback;
    pRequest->asyncInfo.callbacks.connectionClosedCallback = _httpConnectionClosedCallback;
    // NOT USED, pRequest->asyncInfo.callbacks.writeCallback;
    pRequest->asyncInfo.pPrivData = ( void * ) ( &_httpDownloader.httpCallbackData );

    httpsStatus = IotHttpsClient_Connect( &pConnection->connectionHandle, pConnectionConfig );

    OTA_FUNCTION_NO_CLEANUP();

    return httpsStatus;
}

static int _httpGetFileSize( uint32_t * pFileSize )
{
    /* Return status. */
    int status = EXIT_SUCCESS;
    IotHttpsReturnCode_t httpsStatus = IOT_HTTPS_OK;
    uint16_t responseStatus = IOT_HTTPS_STATUS_OK;

    /* HTTP request and response configurations. We're creating local variables here because this is
     * a temporary synchronous request. */
    IotHttpsRequestInfo_t requestConfig = { 0 };
    IotHttpsResponseInfo_t responseConfig = { 0 };

    /* Synchronous request and response configurations. */
    IotHttpsSyncInfo_t requestSyncInfo = { 0 };
    IotHttpsSyncInfo_t responseSyncInfo = { 0 };

    /* Handle for HTTP request and response. */
    IotHttpsRequestHandle_t requestHandle = NULL;
    IotHttpsResponseHandle_t responseHandle = NULL;

    /* HTTP URL information. */
    _httpUrlInfo_t * pUrlInfo = &_httpDownloader.httpUrlInfo;

    /* Value of the "Content-Range" field in HTTP response header. The format is "bytes 0-0/FILESIZE". */
    char pContentRange[ sizeof( "bytes 0-0/" ) + OTA_MAX_FILE_SIZE_STR_LEN ] = { 0 };

    /* Pointer to the location of the file size in pContentRange. */
    char * pFileSizeStr = NULL;

    /* There's no message body in this GET request. */
    requestSyncInfo.pBody = NULL;
    requestSyncInfo.bodyLen = 0;
    /* No need to store the response body since we only need the file size in HTTP response header. */
    responseSyncInfo.pBody = NULL;
    responseSyncInfo.bodyLen = 0;

    /* Set the request configurations. */
    requestConfig.pPath = pUrlInfo->pPath;
    requestConfig.pathLen = pUrlInfo->pathLength;
    requestConfig.pHost = pUrlInfo->pAddress;
    requestConfig.hostLen = pUrlInfo->addressLength;
    requestConfig.method = IOT_HTTPS_METHOD_GET;
    requestConfig.userBuffer.pBuffer = pRequestUserBuffer;
    requestConfig.userBuffer.bufferLen = sizeof( pRequestUserBuffer );
    requestConfig.isAsync = false;
    requestConfig.u.pSyncInfo = &requestSyncInfo;

    /* Set the response configurations. */
    responseConfig.userBuffer.pBuffer = pResponseUserBuffer;
    responseConfig.userBuffer.bufferLen = sizeof( pResponseUserBuffer );
    responseConfig.pSyncInfo = &responseSyncInfo;

    /* Initialize the request to retrieve a request handle. */
    httpsStatus = IotHttpsClient_InitializeRequest( &requestHandle, &requestConfig );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to initialize the HTTP request context. Error code: %d.", httpsStatus );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }

    /* Set the "Range" field in HTTP header to "bytes=0-0" since we just want the file size. */
    httpsStatus = IotHttpsClient_AddHeader( requestHandle,
                                            "Range",
                                            sizeof( "Range" ) - 1,
                                            "bytes=0-0",
                                            sizeof ( "bytes=0-0" ) - 1 );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to populate the HTTP header for request. Error code: %d", httpsStatus );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }

    /* Send the request synchronously. */
    httpsStatus = IotHttpsClient_SendSync( _httpDownloader.httpConnection.connectionHandle,
                                           requestHandle,
                                           &responseHandle,
                                           &responseConfig,
                                           HTTP_SYNC_TIMEOUT );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to send the HTTP request synchronously. Error code: %d", httpsStatus );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }

    httpsStatus = IotHttpsClient_ReadResponseStatus( responseHandle, &responseStatus );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to read the HTTP response status. Error code: %d", httpsStatus );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }
    if( responseStatus != IOT_HTTPS_STATUS_PARTIAL_CONTENT )
    {
        IotLogError( "Fail to get the object size from HTTP server, HTTP response code from server: %d", responseStatus );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }

    /* Parse the HTTP header and retrieve the file size. */
    httpsStatus = IotHttpsClient_ReadHeader( responseHandle,
                                             "Content-Range",
                                             sizeof( "Content-Range" ) - 1,
                                             pContentRange,
                                             sizeof( pContentRange ) );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to read the \"Content-Range\" field from HTTP header. Error code: %d", httpsStatus );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }

    pFileSizeStr = strstr( pContentRange, "/" );
    if( pFileSizeStr == NULL )
    {
        IotLogError( "Could not find '/' from \"Content-Range\" field: %s", pContentRange );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }
    else
    {
        pFileSizeStr += sizeof( char );
    }

    *pFileSize = ( uint32_t ) strtoul( pFileSizeStr, NULL, 10 );
    if( ( *pFileSize == 0 ) || ( *pFileSize == UINT32_MAX ) )
    {
        IotLogError( "Failed to convert \"Content-Range\" value %s to integer. strtoul returned %d", pFileSizeStr, *pFileSize );
        status = EXIT_FAILURE;
        OTA_GOTO_CLEANUP();
    }

    OTA_FUNCTION_NO_CLEANUP();

    return status;
}

OTA_Err_t _AwsIotOTA_InitFileTransfer_HTTP( OTA_AgentContext_t * pAgentCtx )
{
    /* Return status. */
    OTA_Err_t status = kOTA_Err_None;
    IotHttpsReturnCode_t httpsStatus = IOT_HTTPS_OK;

    /* Network interface and credentials from OTA agent. */
    IotNetworkCredentials_t * pNetworkCredentials = ( IotNetworkCredentials_t * ) ( pAgentCtx->pNetworkCredentialInfo );
    const IotNetworkInterface_t * pNetworkInterface = pAgentCtx->pNetworkInterface;

    // Get pre-signed URL from pAgentCtx.
    const char * pURL = ( const char * )( pAgentCtx->pxOTA_Files[ pAgentCtx->ulServerFileID ].pucUpdateUrlPath );

    /* File context from OTA agent. */
    OTA_FileContext_t* fileContext = pAgentCtx->pxOTA_Files;

    /* OTA download file size from OTA agent (parsed from job document). */
    uint32_t otaFileSize = 0;

    /* OTA download file size from the HTTP server, this should match otaFileSize. */
    uint32_t httpFileSize = 0;

    /* Store the OTA agent for later access. */
    _httpDownloader.pAgentCtx = pAgentCtx;

    /* Get the file size from OTA agent (parsed from job document). */
    if( fileContext == NULL )
    {
        IotLogError( "File context from OTA agent is NULL." );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }
    otaFileSize = fileContext->ulFileSize;

    /* Initialize the HTTPS library. */
    httpsStatus = IotHttpsClient_Init();
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to initialize HTTP library." );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }

    /* Connect to the HTTP server and initialize download information. */
    httpsStatus = _httpConnect( pURL, pNetworkInterface, pNetworkCredentials );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Failed to connect to %.*s", _httpDownloader.httpUrlInfo.addressLength, _httpDownloader.httpUrlInfo.pAddress );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }
    IotLogInfo( "Successfully connected to %.*s", _httpDownloader.httpUrlInfo.addressLength, _httpDownloader.httpUrlInfo.pAddress );

    /* Check if the file size from HTTP server matches the file size from OTA job document. */
    if( _httpGetFileSize( &httpFileSize ) )
    {
        IotLogError( "Cannot retrieve the file size from HTTP server." );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }
    if( httpFileSize != otaFileSize )
    {
        IotLogError( "File size from the HTTP server (%u bytes) does not match the size from OTA "
                     "job document (%u bytes).",
                     ( unsigned int ) httpFileSize,
                     ( unsigned int ) otaFileSize);
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }
    IotLogInfo( "Start requesting %u bytes from HTTP server.", ( unsigned int ) httpFileSize );

    OTA_FUNCTION_NO_CLEANUP();

    return status;
}


OTA_Err_t _AwsIotOTA_RequestDataBlock_HTTP( OTA_AgentContext_t * pAgentCtx )
{
    IotLogDebug( "Invoking _AwsIotOTA_RequestDataBlock_HTTP" );

    /* Return status. */
    OTA_Err_t status = kOTA_Err_None;
    IotHttpsReturnCode_t httpsStatus = IOT_HTTPS_OK;

    /* HTTP connection data. */
    _httpConnection_t * pConnection = &_httpDownloader.httpConnection;

    /* HTTP request data. */
    _httpRequest_t * pRequest = &_httpDownloader.httpRequest;

    /* HTTP response data. */
    _httpResponse_t * pResponse = &_httpDownloader.httpResponse;

    /* Values for the "Range" field in HTTP header. */
    uint32_t rangeStart = 0;
    uint32_t rangeEnd = 0;
    int numWritten = 0;

    /* File context from OTA agent. */
    OTA_FileContext_t* fileContext = pAgentCtx->pxOTA_Files;

    if( _httpDownloader.isDownloading )
    {
        IotLogInfo( "Current download is not finished, skipping the request." );
        OTA_GOTO_CLEANUP();
    }
    else
    {
        _httpDownloader.isDownloading = true;
        _httpDownloader.state = OTA_HTTP_OK;
    }

    if( fileContext == NULL )
    {
        IotLogError( "File context from OTA agent is NULL." );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }

    /* Calculate ranges. */
    rangeStart = _httpDownloader.currBlock * OTA_FILE_BLOCK_SIZE;
    if( fileContext->ulBlocksRemaining == 1)
    {
        rangeEnd = fileContext->ulFileSize - 1;
    }
    else
    {
        rangeEnd = rangeStart + OTA_FILE_BLOCK_SIZE - 1;
    }
    _httpDownloader.currBlockSize = rangeEnd - rangeStart + 1;

    /* Creating the "range" field in HTTP header. */
    numWritten = snprintf( _httpDownloader.httpCallbackData.pRangeValueStr,
                           HTTP_HEADER_RANGE_VALUE_MAX_LEN,
                           "bytes=%u-%u",
                           ( unsigned int ) rangeStart,
                           ( unsigned int ) rangeEnd );
    if( numWritten < 0 || numWritten >= HTTP_HEADER_RANGE_VALUE_MAX_LEN )
    {
        IotLogError( "Fail to write the \"Range\" value for HTTP header." );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }

    /* Re-initialize the request handle as it could be changed when handling last reponse. */
    httpsStatus = IotHttpsClient_InitializeRequest( &pRequest->requestHandle, &pRequest->requestConfig );
    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to initialize the HTTP request. Error code: %d.", httpsStatus );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }

    /* Send the request asynchronously. Receiving is handled in a callback. */
    IotLogInfo( "Sending HTTP request to download block %d.", _httpDownloader.currBlock );
    httpsStatus = IotHttpsClient_SendAsync( pConnection->connectionHandle,
                                            pRequest->requestHandle,
                                            &pResponse->responseHandle,
                                            &pResponse->responseConfig );

    if( httpsStatus != IOT_HTTPS_OK )
    {
        IotLogError( "Fail to send the HTTP request asynchronously. Error code: %d.", httpsStatus );
        status = kOTA_Err_Panic;
        OTA_GOTO_CLEANUP();
    }

    OTA_FUNCTION_CLEANUP_BEGIN();

    /* Reset the isDownloading flag if there's any error occurred, i.e. the request is not sent. */
    if( status != kOTA_Err_None)
    {
        _httpDownloader.isDownloading = false;
    }

    OTA_FUNCTION_CLEANUP_END();

    return status;
}

OTA_Err_t _AwsIotOTA_DecodeFileBlock_HTTP( uint8_t * pMessageBuffer,
                                           size_t messageSize,
                                           int32_t * pFileId,
                                           int32_t * pBlockId,
                                           int32_t * pBlockSize,
                                           uint8_t ** pPayload,
                                           size_t * pPayloadSize)
{
    /* Unused parameters. */
    (void) messageSize;

    *pPayload = pMessageBuffer;
    *pFileId = 0;
    *pBlockId = _httpDownloader.currBlock;
    *pBlockSize = _httpDownloader.currBlockSize;
    *pPayloadSize = _httpDownloader.currBlockSize;

    /* Reset the isDownloading flag. */
    _httpDownloader.isDownloading = false;
    /* Current block is processed, Set the file block to next one. */
    _httpDownloader.currBlock += 1;

    return kOTA_Err_None;
}


OTA_Err_t _AwsIotOTA_Cleanup_HTTP( OTA_AgentContext_t * pAgentCtx )
{
    memset( &_httpDownloader, 0, sizeof( _httpDownloader_t ) );
    return kOTA_Err_None;
}
