/*
 * Amazon FreeRTOS
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

#ifndef _AWS_OTA_PAL_TEST_ACCESS_DECLARE_H_
#define _AWS_OTA_PAL_TEST_ACCESS_DECLARE_H_

#include "aws_ota_types.h"
#include "aws_ota_agent.h"

s32 TEST_OTA_prvAbort( OTA_FileContext_t * const C );
bool_t TEST_OTA_prvCreateFileForRx( OTA_FileContext_t * const C );
s32 TEST_OTA_prvCloseFile( OTA_FileContext_t * const C );
u8 TEST_OTA_prvActivateNewImage( void );
OTA_Err_t prvCheckFileSignature( OTA_FileContext_t * const C );
u8 * TEST_OTA_prvReadAndAssumeCertificate( const u8 * const pucCertName,
                                           s32 * const lSignerCertSize );
s16 TEST_OTA_prvWriteBlock( OTA_FileContext_t * const C,
                            s32 iOffset,
                            u8 * const pacData,
                            u32 iBlockSize );

#endif /* ifndef _OTA_AGENT_TEST_ACCESS_DECLARE_H_ */
