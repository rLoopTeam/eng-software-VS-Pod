/**
 * @file		FCU_CORE__ENUMS.H
 * @brief		Flight control code constants
 * @author		Lachlan Grogan, Marek Gutt-Mostowy, gsweriduk
 * @copyright	rLoop Inc.
 */
#ifndef RLOOP_LCCM655__RLOOP__FCU_CORE_FCU_CORE__ENUMS_H_
#define RLOOP_LCCM655__RLOOP__FCU_CORE_FCU_CORE__ENUMS_H_

	//////////////////////////////////////////////////////
	//FCTL: COOLING
	//////////////////////////////////////////////////////

	typedef enum
	{
		COOLING_CTL_DO_NOTHING = 0U,
		CLOSE_ALL_VALVES,
		SET_VALVE
	}E_FCU__COOLING_GS_COMM_T;

	//////////////////////////////////////////////////////
	//FCTL: LIFTING MECHANISM
	//////////////////////////////////////////////////////

	typedef enum
	{
		LIFTMECH_DIR_DOWN = 0U,
		LIFTMECH_DIR_UP = 1U

	}E_FCU__LIFTMECH_DIRECTION;

	typedef enum
	{
		LIFTMECH_AftLeft = 0U,
		LIFTMECH_AftRight = 1U,
		LIFTMECH_ForwardLeft = 2U,
		LIFTMECH_ForwardRight = 3U
	}E_FCU__LIFTMECH_ACTUATOR;

	typedef enum
	{
		LIFT_MECH_STATE__RETRACTED,
		LIFT_MECH_STATE__EXTENDED
	}E_FCU_LIFTMECH_STATE;

	//////////////////////////////////////////////////////
	//FCTL: EDDY BRAKES
	//////////////////////////////////////////////////////

	typedef enum
	{
		EDDY_BRAKES_OUTWARD = 0U,
		EDDY_BRAKES_INWARD = 1U

	}E_FCU__FCTL_EDDY_BRAKES_DIRECTION;

	typedef enum
	{
		EDDY_BRAKES_Left = 0U,
		EDDY_BRAKES_Right = 1U

	}E_FCU__FCTL_EDDY_BRAKES_ACTUATOR;

	typedef enum
	{
		EDDY_BRAKES_STATE__RETRACTED,
		EDDY_BRAKES_STATE__CONTROLLED_BRAKING,
		EDDY_BRAKES_STATE__BRAKING
	}E_FCU__FCTL_EDDY_BRAKES_STATE;

	//////////////////////////////////////////////////////
	//FCTL_MAINSM: SPACEX TELEMETRY
	//////////////////////////////////////////////////////

	/** Enum for SpaceX Telemetry */
	typedef enum
	{
		/** If tests failed */
		POD_STATUS__FAULT = 0U,

		/** Otherwise */
		POD_STATUS__IDLE = 1U,

		/** If ready for push */
		POD_STATUS__READY = 2U,

		/** If pushing */
		POD_STATUS__PUSHING = 3U,

		/** If coasting */
		POD_STATUS__COAST = 4U,

		/** If braking */
		POD_STATUS__BRAKING = 5U

	}E_FCU__POD_STATUS;

	//////////////////////////////////////////////////////
	//FCTL: STATE MACHINE
	//////////////////////////////////////////////////////

	/** Enum for GS commands to State Machine */
	typedef enum
	{
		MAINSM_GS_NO_CMD = 0U,
		MAINSM_GS_ENTER_PRE_RUN_PHASE = 1U
	}E_FCU__MAINSM_GS_COMM;

	//////////////////////////////////////////////////////
	//FCTL: DRIVE POD
	//////////////////////////////////////////////////////

	/**Drive Pod GS commands */
	typedef enum
	{
		DRIVEPOD_GS_NO_CMD = 0U,
		DRIVEPOD_GS_POD_STOP = 1U
	}E_FCU__DRIVEPOD_GS_COMM;

	/**Drive Pod State Machine */
	typedef enum
	{
		DRIVEPOD_PRERUN_INITIAL_STATE = 0U,
		DRIVEPOD_PRERUN_START_HE_STATE = 1U,
		DRIVEPOD_PRERUN_RETRACT_LIFTMECH = 2U,
		DRIVEPOD_PRERUN_GIMBAL_BACKWARD = 3U,
		DRIVEPOD_PRERUN_GIMBAL_MAINTAIN = 4U
	}E_FCU__DRIVEPOD_PRERUN_STATE;

	//////////////////////////////////////////////////////
	//FCTL: GIMBALS
	//////////////////////////////////////////////////////

	typedef enum
	{
		GIMBAL_NEUTRAL_LEVEL = 0U,
		GIMBAL_BACKWARD_LEVEL = 1U,
		GIMBAL_FORWARD_LEVEL = 2U
	}E_FCU__GIMBAL_LEVEL;

	//////////////////////////////////////////////////////
	//LANDING GEAR UNIT COMMS
	//////////////////////////////////////////////////////

	/** For transmitting to the LGU*/
	typedef enum
	{

		/** IDLE */
		LGU_COMMS_STATE__IDLE = 0U,

		/** beign a transmission */
		LGU_COMMS_STATE__TX_BEGIN,

		/** process the transmision */
		LGU_COMMS_STATE__TX_PROCESS,

		/** process the new Rx packet*/
		LGU_COMMS_STATE__PROCESS_RX

	}E_LGU_COMMS_STATE_T;

	//////////////////////////////////////////////////////
	//OPTONCDT LASERS
	//////////////////////////////////////////////////////

	/** OptoNCDT Laser Positions
	 *  must be in order from A0:2, B0:2
	 *  */
	typedef enum
	{

		LASER_OPTO__HEIGHT_FL = 0U,
		LASER_OPTO__HEIGHT_FR = 1U,
		LASER_OPTO__HEIGHT_RL = 2U,
		LASER_OPTO__HEIGHT_RR = 3U,
		LASER_OPTO__IBEAM_FWD = 4U,
		LASER_OPTO__IBEAM_AFT = 5U,
		LASER_OPTO__NUM = 6U

	}E_FCU__LASER_OPTO__INDEX_T;

	//////////////////////////////////////////////////////
	//CONTRAST SENSORS
	//////////////////////////////////////////////////////

	/** Laser contrast sensors */
	typedef enum
	{
		LASER_CONT__FWD = 0U,

		LASER_CONT__MID = 1U,

		LASER_CONT__AFT = 2U,

		LASER_CONT__MAX = 3U

	}E_FCU__LASER_CONT_INDEX_T;

	//////////////////////////////////////////////////////
	//BRAKES
	//////////////////////////////////////////////////////

	/** enum type for the left or right brakes */
	typedef enum
	{
		/** left brake */
		FCU_BRAKE__LEFT = 0U,

		/** Right hand brake */
		FCU_BRAKE__RIGHT = 1U,


		FCU_BRAKE__MAX_BRAKES = 2U


	}E_FCU__BRAKE_INDEX_T;

	/** Brake limit switch enums */
	typedef enum
	{
		/** Fully extended limit switch */
		BRAKE_SW__EXTEND = 0U,

		/** Fully retracted switch */
		BRAKE_SW__RETRACT = 1U,

		BRAKE_SW__MAX_SWITCHES = 2U

	}E_FCU__BRAKE_LIMSW_INDEX_T;

	//////////////////////////////////////////////////////
	//LIMIT SWITCHES
	//////////////////////////////////////////////////////

	/** The final switch state, and indeed any switch state */
	typedef enum
	{

		/** Big issue, unknown state type */
		SW_STATE__UNKNOWN = 0U,

		/** The switch is closed, physically but maybe not electrically */
		SW_STATE__CLOSED,

		/** The switch is open */
		SW_STATE__OPEN


	}E_FCU__SWITCH_STATE_T;

	//////////////////////////////////////////////////////
	//THROTTLES
	//////////////////////////////////////////////////////

	/** Ground station commands for throttle layer  */
	// (added by @gsweriduk on 23 NOV 2016)
	// Partial list, see http://confluence.rloop.org/display/SD/Commands+from+Ground+Station+to+rPod

	typedef enum
	{
		START_STATIC_HOVERING,
		STOP_STATIC_HOVERING,
		SET_HEX_SPEED,
		SET_ALL_HE_SPEED
	}E_GS_COMMANDS;

	/** State types for the THROTTLE state machine */
	typedef enum
	{

		/** Waiting here */
		THROTTLE_STATE__IDLE = 0U,

		/** Run mode, allow the HE's to throttle up */
		THROTTLE_STATE__RUN,

		/** Step command(s) to the HEs */
		THROTTLE_STATE__STEP,

		/** Ramp command(s) to the HEs */
		THROTTLE_STATE__RAMP_UP,

		/** Ramp command(s) to the HEs */
		THROTTLE_STATE__RAMP_DOWN,

		/** Mode to the next scanning index */
		THROTTLE_STATE__INC_INDEX,

		/** an error was encountered */
		THROTTLE_STATE__ERROR,

	} E_THROTTLE_STATES_T;

 #ifdef WIN32
	//////////////////////////////////////////////////////
	//FCTL: ERROR INJECT
	//////////////////////////////////////////////////////
	typedef enum
	{
		INJECT_NONE = 0x00

		INJECT_UNLIFTED,

		INJECT_LIFTED,

		INJECT_STATIC_HOVERING,

		INJECT_READY_FOR_PUSH,

		INJECT_PUSHING,

		INJECT_COASTING,

		INJECT_BRAKING,

		INJECT_CTL_EMERGENCY_BRAKING,

	}E_FCU__ERROR_INJECT;
 #endif // WIN32

#endif /* RLOOP_LCCM655__RLOOP__FCU_CORE_FCU_CORE__ENUMS_H_ */
