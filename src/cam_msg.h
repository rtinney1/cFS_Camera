/* Copyright (C) 2009 - 2017 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
any warranty that the software will be error free.

In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
documentation or services provided hereunder

ITC Team
NASA IV&V
ivv-itc@lists.nasa.gov
*/

#ifndef _CAM_MSG_H_
#define _CAM_MSG_H_

#include "cam_device.h"
#include "cfe_sb.h"

/*
** CAM App command codes
*/
// \camcmd CAM NOOP Command
#define CAM_NOOP_CC                 0
// \camcmd CAM Reset Counter Command
#define CAM_RESET_COUNTERS_CC       1

/* Generic Science CC */
// \camcmd CAM Stop Science
#define CAM_STOP_CC		        	2		// Stop all science in preperation for immediate shutdown
// \camcmd CAM Pause Science
#define CAM_PAUSE_CC				3		// Pause data transfer
// \camcmd CAM Resume Science 
#define CAM_RESUME_CC				4		// Resume data transfer
// \camcmd CAM Timeout Science      
#define CAM_TIMEOUT_CC              5       // Stop all science due to experiment timeout
// \camcmd CAM Low Voltage
#define CAM_LOW_VOLTAGE_CC          6       // Stop all science due to low voltage

/* Complete Experiment CC */
// \camcmd CAM Experiment 1 - Small
#define CAM_EXP1_CC					10     
// \camcmd CAM Experiment 2 - Medium
#define CAM_EXP2_CC			     	11       
// \camcmd CAM Experiment 3 - Large
#define CAM_EXP3_CC			     	12   
// \camcmd CAM Hardware Check
#define CAM_HW_CHECK_CC             13

/* Debug and Testing CC */
#define CAM_HWLIB_INIT_I2C_CC       20
#define CAM_HWLIB_INIT_SPI_CC       21
#define CAM_HWLIB_CONFIG_CC         22
#define CAM_HWLIB_JPEG_INIT_CC      23
#define CAM_HWLIB_YUV422_CC         24
#define CAM_HWLIB_JPEG_CC           25
#define CAM_HWLIB_SETUP_CC          26
#define CAM_HWLIB_SETSIZE_CC        27
#define CAM_HWLIB_CAPTURE_PREP_CC   28
#define CAM_HWLIB_CAPTURE_CC        29
#define CAM_HWLIB_READ_PREP_CC      30
#define CAM_HWLIB_READ_CC           31
#define CAM_PUBLISH_CC              32

#define CAM_DATA_SIZE               1010 //Necessary to avoid compiler errors

/*
** CAM no argument command
** See also: #CAM_NOOP_CC, #CAM_RESET_COUNTER_CC, #CAM_STOP_CC, 
** #CAM_PAUSE_CC, #CAM_RESUME_CC, #CAM_EXP1_CC, #CAM_EXP2_CC,
** #CAM_EXP3_CC
*/
typedef struct
{
   CFE_MSG_CommandHeader_t CmdHeader;

} CAM_NoArgsCmd_t;
#define CAM_NOARGSCMD_LNGTH sizeof (CAM_NoArgsCmd_t)

/*
** Type definition (CAM housekeeping)
** \camtlm CAM Housekeeping telemetry packet
** #CAM_HK_TLM_MID
*/
typedef struct 
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8       CommandErrorCount;
    uint8       CommandCount;

} CAM_Hk_tlm_t;
#define CAM_HK_TLM_LNGTH  sizeof ( CAM_Hk_tlm_t )

/*
** Type definition (CAM EXP)
** \camtlm CAM Experiment telemetry packet
** #CAM_EXP_TLM_MID
*/
typedef struct 
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8		data[CAM_DATA_SIZE];
    uint16      spare;
    uint32		msg_count;
    uint32      length;
    
} CAM_Exp_tlm_t;
#define CAM_EXP_TLM_LNGTH  sizeof ( CAM_Exp_tlm_t )

#endif 
