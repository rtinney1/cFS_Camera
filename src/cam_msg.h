
#ifndef _CAM_MSG_H_
#define _CAM_MSG_H_

#include "cfe_sb.h"

/*
** CAM App command codes
*/
// \camcmd CAM NOOP Command
#define CAM_NOOP_CC                 0
// \camcmd CAM Reset Counter Command
#define CAM_RESET_COUNTERS_CC       1

#define CAM_TAKE_PIC_CC			2
#define CAM_SEND_PIC_CC			3

#define MAX_IMAGE_LENGTH 1080

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
    uint8		data[MAX_IMAGE_LENGTH];
    uint16      length;
    
} CAM_Pic_tlm_t;
#define CAM_PIC_TLM_LNGTH  sizeof ( CAM_Pic_tlm_t )

#endif 
