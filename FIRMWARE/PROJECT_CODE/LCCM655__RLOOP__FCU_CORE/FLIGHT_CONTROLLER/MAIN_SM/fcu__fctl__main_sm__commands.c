#include "../../fcu_core.h"

#if C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE == 1U
#if C_LOCALDEF__LCCM655__ENABLE_FLIGHT_CONTROL == 1U
#if C_LOCALDEF__LCCM655__ENABLE_MAIN_SM == 1U

 //the structure
extern struct _strFCU sFCU;


/////////////////////////////////////////////////////////////////////
//  Pod command functions
/////////////////////////////////////////////////////////////////////

void cmd_POD_COMMAND__IDLE(void)
{
	#if DEBUG == 1U
		printf("cmd_POD_COMMAND__IDLE() called\n");
	#endif

	TS_POD_COMMAND_T * cmd = &sFCU.sStateMachine.command;
	cmd->command = POD_COMMAND__IDLE;

}

void cmd_POD_COMMAND__TEST_MODE(void)
{
	#if DEBUG == 1U
		printf("cmd_POD_COMMAND__TEST_MODE() called\n");
	#endif

	TS_POD_COMMAND_T * cmd = &sFCU.sStateMachine.command;
	cmd->command = POD_COMMAND__TEST_MODE;

}

void cmd_POD_COMMAND__DRIVE(void)
{
	#if DEBUG == 1U
		printf("cmd_POD_COMMAND__DRIVE() called\n");
	#endif

	TS_POD_COMMAND_T * cmd = &sFCU.sStateMachine.command;
	cmd->command = POD_COMMAND__DRIVE;

}

void cmd_POD_COMMAND__FLIGHT_PREP(void)
{
	#if DEBUG == 1U
		printf("cmd_POD_COMMAND__FLIGHT_PREP() called\n");
	#endif

	TS_POD_COMMAND_T * cmd = &sFCU.sStateMachine.command;
	cmd->command = POD_COMMAND__FLIGHT_PREP;

}

void cmd_POD_COMMAND__ARMED_WAIT(void)
{
	#if DEBUG == 1U
		printf("cmd_POD_COMMAND__ARMED_WAIT() called\n");
	#endif

	TS_POD_COMMAND_T * cmd = &sFCU.sStateMachine.command;
	cmd->command = POD_COMMAND__ARMED_WAIT;

}

void cmd_POD_COMMAND__READY(void)
{
	#if DEBUG == 1U
		printf("cmd_POD_COMMAND__READY() called\n");
	#endif

	TS_POD_COMMAND_T * cmd = &sFCU.sStateMachine.command;
	cmd->command = POD_COMMAND__READY;

}



void vFCU_FCTL__PutCommand(TE_POD_COMMAND_T command)
{
	// Attempt to execute the command (provided that the interlock timeout has not expired)
	switch (command)
	{
	case POD_COMMAND__IDLE:
		cmd_POD_COMMAND__IDLE();
		break;
	case POD_COMMAND__TEST_MODE:
		cmd_POD_COMMAND__TEST_MODE();
		break;
	case POD_COMMAND__DRIVE:
		cmd_POD_COMMAND__DRIVE();
		break;
	case POD_COMMAND__FLIGHT_PREP:
		cmd_POD_COMMAND__FLIGHT_PREP();
		break;
	case POD_COMMAND__ARMED_WAIT:
		cmd_POD_COMMAND__ARMED_WAIT();
		break;
	case POD_COMMAND__READY:
		cmd_POD_COMMAND__READY();
		break;
	default:
		// do nothing
		break;
	}

}


/////////////////////////////////////////////////////////////////////
//  Interlock guard handling
/////////////////////////////////////////////////////////////////////


// Initialize an existing interlock command
void vFCU_FCTL_MAINSM__InterlockGuard__Init(TS_INTERLOCK_GUARD_T *pInterlockGuard, Luint32 u32Duration_x10ms)
{
	vFCU_FCTL__TIMEOUT__Init(&pInterlockGuard->timeout, u32Duration_x10ms);
	pInterlockGuard->enabled = 0U;
}

// Call this when the first packet is received. Ok to call it multiple times; it will just reset the timer.
void vFCU_FCTL_MAINSM__InterlockGuard__Unlock(TS_INTERLOCK_GUARD_T *pInterlockGuard)
{
	pInterlockGuard->enabled = 1;
	vFCU_FCTL__TIMEOUT__Restart(&pInterlockGuard->timeout);
}

// Call this when the second packet is received to check whether the command can execute (i.e. timeout has not expired)
Luint8 u8FCU_FCTL_MAINSM__InterlockGuard__IsUnlocked(TS_INTERLOCK_GUARD_T *pInterlockGuard)
{
	Luint8 can_execute;

	Luint8 enabled = pInterlockGuard->enabled;
	Luint8 expired = u8FCU_FCTL__TIMEOUT__Is_Expired(&pInterlockGuard->timeout);
	Luint8 is_unlocked = (enabled == 1 && expired == 0);

	return is_unlocked;

}



// Call this if the command was executed and we're ready to listen for the initial packet again
// @todo: do we even need this? if we receive another enable packet, we will restart the timeout. Once its timed out, it will not keep counting, so we're ok.
void vFCU_FCTL_MAINSM__InterlockGuard__Reset(TS_INTERLOCK_GUARD_T *pInterlockGuard)
{
	// Reset the timeout (stop it and set the elapsed time to 0)
	vFCU_FCTL__TIMEOUT__Reset(&pInterlockGuard->timeout);
}

// Call this in one of our timer ISRs. Ok to call this since the timeout has to be started for the update to have any effect.
void vFCU_FCTL_MAINSM__InterlockGuard__UpdateTimeout_x10ms(TS_INTERLOCK_GUARD_T *pInterlockGuard)
{
	// Update the timeout
	vFCU_FCTL__TIMEOUT__Update_x10ms(&pInterlockGuard->timeout);
}


// Convenience functions for interlocked commands
// Interlock command integration functions (depends on sFCU and state machine -- the functions above do not)

Luint8 vFCU_FCTL_MAINSM__NetCommand_IsUnlocked(TE_POD_COMMAND_T command)
{
	return u8FCU_FCTL_MAINSM__InterlockGuard__IsUnlocked(&sFCU.sStateMachine.sInterlockGuards[command]);
}


void vFCU_FCTL_MAINSM__NetCommand_Unlock(TE_POD_COMMAND_T command)
{
	vFCU_FCTL_MAINSM__InterlockGuard__Unlock(&sFCU.sStateMachine.sInterlockGuards[command]);
}


//safetys
#endif //C_LOCALDEF__LCCM655__ENABLE_MAIN_SM
#ifndef C_LOCALDEF__LCCM655__ENABLE_MAIN_SM
	#error
#endif
#endif //C_LOCALDEF__LCCM655__ENABLE_FLIGHT_CONTROL
#ifndef C_LOCALDEF__LCCM655__ENABLE_FLIGHT_CONTROL
	#error
#endif
#endif //C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE
//safetys
#ifndef C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE
	#error
#endif
