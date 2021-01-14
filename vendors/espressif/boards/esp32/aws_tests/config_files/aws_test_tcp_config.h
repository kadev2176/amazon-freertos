/*
 * FreeRTOS V1.1.4
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

#ifndef AWS_INTEGRATION_TEST_TCP_CONFIG_H
#define AWS_INTEGRATION_TEST_TCP_CONFIG_H

/**
 * @file aws_integration_test_tcp_portable.h
 * @brief Port-specific variables for TCP tests. */

/**
 * @brief Indicates how much longer than the specified timeout is acceptable for
 * RCVTIMEO tests.
 *
 * This value can be used to compensate for clock differences, and other
 * code overhead.
 */
#define         integrationtestportableTIMEOUT_OVER_TOLERANCE      200

/**
 * @brief Indicates how much less time than the specified timeout is acceptable for
 * RCVTIMEO tests.
 *
 * This value must be 0 unless networking is performs on a separate processor.
 * If networking and tests are on different CPUs, an "under tolerance" is acceptable.
 * For tests where same clock is used for networking and tests.
 */
#define         integrationtestportableTIMEOUT_UNDER_TOLERANCE     0

/**
 *  @brief Indicates how long  receive needs to wait for data before Timeout happens.
 *
 */
#define         integrationtestportableRECEIVE_TIMEOUT             10000

/**
 * @brief Indicates how long  send needs to wait before Timeout happens.
 *
 */
#define         integrationtestportableSEND_TIMEOUT                10000

/**
 * @brief The timeout for all TCP echo multi-task tests.
 */
#define         tcptestECHO_TEST_SYNC_TIMEOUT                      80000

/**
 * @brief The stack size of the tasks created in all TCP echo multi-task tests.
 */
#define         tcptestTCP_ECHO_TASKS_STACK_SIZE                   ( configMINIMAL_STACK_SIZE * 8 )

/**
 * @brief The priority of the tasks created in all TCP echo multi-task tests.
 */
#define         tcptestTCP_ECHO_TASKS_PRIORITY                     ( tskIDLE_PRIORITY + 5 )


#endif /*AWS_INTEGRATION_TEST_TCP_CONFIG_H */
