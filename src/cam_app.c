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

/*******************************************************************************
** File: cam_app.c
**
** Purpose:
**   This file contains the source code for the Sample STF1 App.
**
*******************************************************************************/

#include "cam_app.h"

/*
** global app data
*/
CAM_AppData_t CAM_AppData;

/*
** CAMERA_AppMain() -- Application entry point and main process loop
*/
void CAMERA_AppMain( void )
{
    int32 status = 0;
    CAM_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_ES_PerfLogEntry(CAM_PERF_ID);

    /* 
    ** initialize the application, register the app, etc 
    */
    status = CAM_AppInit();
    if(status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CAM_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "CAM App: init error %d", status);
        CAM_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** CAM Runloop
    */
    while (CFE_ES_RunLoop(&CAM_AppData.RunStatus) == true)
    {
        /*
        ** Exit performance profiling.  It will be restarted later in this while loop. 
        */
        CFE_ES_PerfLogExit(CAM_PERF_ID);

        /* 
        ** Pend on receipt of command packet -- set timeout to 500ms as cFE default
        ** could also set no timeout - this means that this app
        ** will block until a message is received.  Refer to the header file docs
        ** for more information on using this function
        */
        status = CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&CAM_AppData.MsgPtr,  CAM_AppData.CmdPipe,  CFE_SB_PEND_FOREVER);
        
        /* 
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution
        */
        CFE_ES_PerfLogEntry(CAM_PERF_ID);

        /*
        ** If the RcvMsg() was successful, then continue to process the CommandPacket()
        ** if not successful, then 
        */
        if (status == CFE_SUCCESS)
        {
            CAM_ProcessCommandPacket();
        }
        else if (status == CFE_SB_PIPE_RD_ERR)
        {
            /* This is an example of exiting on an error.
            ** Note that a SB read error is not always going to
            ** result in an app quitting.
            */
            CFE_EVS_SendEvent(CAM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "CAM APP: SB Pipe Read Error, CAM APP will continue with error = %d", status);
            //CAM_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }

    }

    CFE_ES_ExitApp(CAM_AppData.RunStatus);
} 


/* 
** CAM_AppInit() --  initialization
*/
int32 CAM_AppInit(void)
{
    int32 status = OS_SUCCESS;

    while (true)
    {
        /*
        ** Register the events
        */ 
        status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
        if (status != CFE_SUCCESS)
        {
            OS_printf("CAM APP: EVS register error %d", status);
            break;
        }

        /*
        ** Create the Software Bus command pipe 
        */
        status = CFE_SB_CreatePipe(&CAM_AppData.CmdPipe, CAM_PIPE_DEPTH, "CAM_CMD_PIPE");
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CAM_INIT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "CAM APP: Cmd pipe error %d", status);
            break;
        }
        
        /*
        ** Subscribe to "ground commands". Ground commands are those commands with command codes
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CAM_CMD_MID), CAM_AppData.CmdPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CAM_INIT_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "CAM APP: Ground command subscription error %d", status);
            break;
        }

        /*
        ** Subscribe to housekeeping (hk) messages.  HK messages are those messages that request
        ** an app to send its HK telemetry
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CAM_SEND_HK_MID), CAM_AppData.CmdPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CAM_INIT_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR, "CAM APP: HK command subscription error %d", status);
            break;
        }

        /*
        ** Initialize Application Data
        */
        CAM_AppData.HkTelemetryPkt.CommandCount       = 0;
        CAM_AppData.HkTelemetryPkt.CommandErrorCount  = 0;
        CAM_AppData.CamTelemetryPkt.length = 0;


        /* Initialize the published HK message - this HK message will contain the telemetry
        ** that has been defined in the CAM_HkTelemetryPkt for this app
        */
        CFE_MSG_Init(CFE_MSG_PTR(CAM_AppData.HkTelemetryPkt.TlmHeader),
            CFE_SB_ValueToMsgId(CAM_HK_TLM_MID),
            CAM_HK_TLM_LNGTH);
        
        CFE_MSG_Init(CFE_MSG_PTR(CAM_AppData.CamTelemetryPkt.TlmHeader),
            CFE_SB_ValueToMsgId(CAM_PIC_TLM_MID),
            CAM_PIC_TLM_LNGTH);
        
        /* 
        ** Important to send an information event that the app has initialized. this is
        ** useful for debugging the loading of individual apps
        */
        CFE_EVS_SendEvent (CAM_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                "CAM App Initialized. Version %d.%d.%d.%d",
                    CAM_MAJOR_VERSION,
                    CAM_MINOR_VERSION, 
                    CAM_REVISION, 
                    CAM_MISSION_REV);
        break;
    }
    
    return status;
} 


/* 
**  Name:  CAM_ProcessCommandPacket
**
**  Purpose:
**  This routine will process any packet that is received on the CAM command pipe.       
*/
void CAM_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    
    CFE_MSG_GetMsgId(CAM_AppData.MsgPtr, &MsgId);
   
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        /*
        ** Ground Commands with command codes fall under the CAM_APP_CMD_MID
        ** message ID
        */
        case CAM_CMD_MID:
            CAM_ProcessGroundCommand();
            break;

        /*
        ** All other messages, other than ground commands, add to this case statement.
        ** The HK MID comes first, as it is currently the only other messages defined
        ** besides the CAM_APP_CMD_MID message above
        */
        case CAM_SEND_HK_MID:
            CAM_ReportHousekeeping();
            break;

         /*
         ** All other invalid messages that this app doesn't recognize, increment
         ** the command error counter and log as an error event.  
         */
        default:
            CAM_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(CAM_COMMAND_ERR_EID,CFE_EVS_EventType_ERROR, "CAM App: invalid command packet, MID = 0x%x", CFE_SB_MsgIdToValue(MsgId));
            break;
    }

    return;
} 


/*
** CAM_ProcessGroundCommand() -- CAM ground commands
*/
void CAM_ProcessGroundCommand(void)
{
    // Local variables
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case below 
    */
    
        CFE_MSG_GetMsgId(CAM_AppData.MsgPtr, &MsgId);
   

    /*
    ** Ground Commands, by definition, has a command code associated with them.  Pull
    ** this command code from the message and then process the action associated with
    ** the command code.
    */
    
        CFE_MSG_GetFcnCode(CAM_AppData.MsgPtr, &CommandCode);
   
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case CAM_NOOP_CC:
            /* 
            ** notice the usage of the VerifyCmdLength() function call to verify that
            ** the command length is as expected.  
            */
            if (CAM_VerifyCmdLength(CAM_AppData.MsgPtr, sizeof(CAM_NoArgsCmd_t)))
            {
                CAM_AppData.HkTelemetryPkt.CommandCount++;
               
                CFE_EVS_SendEvent(CAM_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM App: NOOP command");
            }
            break;

        /*
        ** Reset Counters Command
        */
        case CAM_RESET_COUNTERS_CC:
            CAM_ResetCounters();
            break;

        case CAM_TAKE_PIC_CC:
            CAM_TakePicture();
            break;
        case CAM_SEND_PIC_CC:
            CAM_SendPicture();

        /*
        ** Invalid Command Codes
        */
        default:
            
                CAM_AppData.HkTelemetryPkt.CommandErrorCount++;
           
            CFE_EVS_SendEvent(CAM_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR, 
                "CAM App: invalid command code for packet MID = 0x%x CC = 0x%x", CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
} 

/* 
**  Name:  CAM_ReportHousekeeping                                             
**                                                                            
**  Purpose:                                                                  
**         This function is triggered in response to a task telemetry request 
**         from the housekeeping task. This function will gather the Apps     
**         telemetry, packetize it and send it to the housekeeping task via   
**         the software bus                                                   
*/
void CAM_ReportHousekeeping(void)
{
    
        CFE_SB_TimeStampMsg((CFE_MSG_Message_t *) &CAM_AppData.HkTelemetryPkt);
        CFE_SB_TransmitMsg((CFE_MSG_Message_t *) &CAM_AppData.HkTelemetryPkt, true);
   
    return;
} 

void CAM_SendPicture(void)
{
    CFE_EVS_SendEvent(CAM_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Sending Picture");
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *) &CAM_AppData.CamTelemetryPkt);
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *) &CAM_AppData.CamTelemetryPkt, true);

    memset(CAM_AppData.CamTelemetryPkt.data, 0, MAX_IMAGE_LENGTH);
    CAM_AppData.CamTelemetryPkt.length = 0;
   
    return;
} 

void CAM_TakePicture(void)
{
    CFE_EVS_SendEvent(CAM_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Taking picture");

    FILE *file = fopen(IMAGE_FILE_PATH, "rb");
    if(file)
    {
        CAM_AppData.CamTelemetryPkt.length = fread(CAM_AppData.CamTelemetryPkt.data, 1, MAX_IMAGE_LENGTH, file);
        fclose(file);
        CFE_EVS_SendEvent(CAM_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Took picture of size %d", CAM_AppData.CamTelemetryPkt.length);
    }
    else
    {
        CFE_EVS_SendEvent(CAM_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Failed to take picture");
    }
}

/*
**  Name:  CAM_ResetCounters                                               
**                                                                            
**  Purpose:                                                                  
**         This function resets all the global counter variables that are    
**         part of the task telemetry.                                        
*/
void CAM_ResetCounters(void)
{
    /* Status of commands processed by the CAM App */
    CAM_AppData.HkTelemetryPkt.CommandCount       = 0;
    CAM_AppData.HkTelemetryPkt.CommandErrorCount  = 0;
    CFE_EVS_SendEvent(CAM_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM App: RESET Counters Command");
    return;
} 

/*
** CAM_VerifyCmdLength() -- Verify command packet length                                                                                              
*/
bool CAM_VerifyCmdLength(CFE_MSG_Message_t * msg, uint16 ExpectedLength)
{     
    bool result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t MessageID = CFE_SB_INVALID_MSG_ID; 
    CFE_MSG_FcnCode_t CommandCode = 0;

    /*
    ** Verify the command packet length.
    */
    CFE_MSG_GetSize(msg, &ActualLength);
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(msg, &MessageID);
        CFE_MSG_GetFcnCode(msg, &CommandCode);

        CFE_EVS_SendEvent(CAM_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
           "Invalid msg length: ID = 0x%X CC = %d Len = %d Expected = %d",
              CFE_SB_MsgIdToValue(MessageID), CommandCode, ActualLength, ExpectedLength);

        result = false;
        CAM_AppData.HkTelemetryPkt.CommandErrorCount++;
    }

    return(result);
} 

