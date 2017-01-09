/**
 * @file		FCU__BRAKES__ETHERNET.C
 * @brief		Eth diagnostics for brakes system
 * @author		Lachlan Grogan
 * @copyright	rLoop Inc.
 */
/**
 * @addtogroup RLOOP
 * @{ */
/**
 * @addtogroup FCU
 * @ingroup RLOOP
 * @{ */
/**
 * @addtogroup FCU__BRAKES__ETHERNET
 * @ingroup FCU
 * @{ */

#include "../fcu_core.h"

#if C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE == 1U
#if C_LOCALDEF__LCCM655__ENABLE_BRAKES == 1U
#if C_LOCALDEF__LCCM655__ENABLE_ETHERNET == 1U

//the structure
extern struct _strFCU sFCU;


/***************************************************************************//**
 * @brief
 * Init the eth portion of the brakes system
 * 
 * @st_funcMD5		3F55F58A66BBC89F7D26261947D1B02E
 * @st_funcID		LCCM655R0.FILE.066.FUNC.001
 */
void vFCU_BRAKES_ETH__Init(void)
{
	sFCU.sBrakesGlobal.sBrakesDev.u8DevMode = 0U;
	sFCU.sBrakesGlobal.sBrakesDev.u32DevKey = 0U;

}


/***************************************************************************//**
 * @brief
 * Process any new eth packets (transmit)
 * 
 * @param[in]		ePacketType			The packet type
 * @st_funcMD5		FB0CF1588BB680EA182940D2655E3159
 * @st_funcID		LCCM655R0.FILE.066.FUNC.002
 */
void vFCU_BRAKES_ETH__Transmit(E_NET__PACKET_T ePacketType)
{
	Lint16 s16Return;
	Luint8 * pu8Buffer;
	Luint8 u8BufferIndex;
	Luint16 u16Length;
	Luint8 u8Counter;

	pu8Buffer = 0;

	//setup length based on packet.
	switch(ePacketType)
	{
		case NET_PKT__FCU_BRAKES__TX_DATA:
			u16Length = (2U * (24U + 5U + 16U + 12U)) + 1U;
			break;

		case NET_PKT__FCU_BRAKES__TX_MOTOR_PARAM:
			u16Length = (20U * 2U) + 1U;
			break;

		default:
			u16Length = 0U;
			break;

	}//switch(ePacketType)

	//pre-comit
	s16Return = s16SAFEUDP_TX__PreCommit(u16Length, (SAFE_UDP__PACKET_T)ePacketType, &pu8Buffer, &u8BufferIndex);
	if(s16Return == 0)
	{
		//handle the packet
		switch(ePacketType)
		{

			case NET_PKT__FCU_BRAKES__TX_MOTOR_PARAM:
				//transmit the stepper motor parameters
				vNUMERICAL_CONVERT__Array_U32(pu8Buffer, u32EEPARAM__Read(C_LOCALDEF__LCCM231__M0_MICROSTEP_RESOLUTION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_S32(pu8Buffer, s32EEPARAM__Read(C_LOCALDEF__LCCM231__M0_MAX_ACCELERATION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_S32(pu8Buffer, s32EEPARAM__Read(C_LOCALDEF__LCCM231__M0_MICRONS_PER_REVOLUTION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_U32(pu8Buffer, u32EEPARAM__Read(C_LOCALDEF__LCCM231__M0_STEPS_PER_REVOLUTION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_S32(pu8Buffer, s32EEPARAM__Read(C_LOCALDEF__LCCM231__M0_MAX_ANGULAR_VELOCITY__PARAM_INDEX));
				pu8Buffer += 4;

				vNUMERICAL_CONVERT__Array_U32(pu8Buffer, u32EEPARAM__Read(C_LOCALDEF__LCCM231__M1_MICROSTEP_RESOLUTION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_S32(pu8Buffer, s32EEPARAM__Read(C_LOCALDEF__LCCM231__M1_MAX_ACCELERATION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_S32(pu8Buffer, s32EEPARAM__Read(C_LOCALDEF__LCCM231__M1_MICRONS_PER_REVOLUTION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_U32(pu8Buffer, u32EEPARAM__Read(C_LOCALDEF__LCCM231__M1_STEPS_PER_REVOLUTION__PARAM_INDEX));
				pu8Buffer += 4;
				vNUMERICAL_CONVERT__Array_S32(pu8Buffer, s32EEPARAM__Read(C_LOCALDEF__LCCM231__M1_MAX_ANGULAR_VELOCITY__PARAM_INDEX));
				pu8Buffer += 4;

				//timer
				pu8Buffer[0] = C_LOCALDEF__LCCM124__RTI_COMPARE_2_PERIOD_US;
				pu8Buffer += 1;

				break;

			case NET_PKT__FCU_BRAKES__TX_DATA:

				//for each brake
				for(u8Counter = 0U; u8Counter < FCU_BRAKE__MAX_BRAKES; u8Counter++)
				{
					//fault flags (general, duplicated)
					vNUMERICAL_CONVERT__Array_U32(pu8Buffer, sFCU.sBrakesGlobal.sFaultFlags.u32Flags[0]);
					pu8Buffer += 4U;

					vNUMERICAL_CONVERT__Array_F32(pu8Buffer,  sFCU.sBrakes[u8Counter].sTarget.f32IBeam_mm);
					pu8Buffer += 4U;
					vNUMERICAL_CONVERT__Array_F32(pu8Buffer, sFCU.sBrakes[u8Counter].sTarget.f32LeadScrew_mm);
					pu8Buffer += 4U;
					vNUMERICAL_CONVERT__Array_U32(pu8Buffer, sFCU.sBrakes[u8Counter].sTarget.u32LeadScrew_um);
					pu8Buffer += 4U;

					//spares
					vNUMERICAL_CONVERT__Array_U32(pu8Buffer, 0U);
					pu8Buffer += 4U;
					vNUMERICAL_CONVERT__Array_U32(pu8Buffer, 0U);
					pu8Buffer += 4U;

					//switches area
					pu8Buffer[0] = (Luint8)sFCU.sBrakes[u8Counter].sLimits[BRAKE_SW__EXTEND].eSwitchState;
					pu8Buffer[1] = (Luint8)sFCU.sBrakes[u8Counter].sLimits[BRAKE_SW__RETRACT].eSwitchState;
					pu8Buffer[2] = sFCU.sBrakes[u8Counter].sLimits[BRAKE_SW__EXTEND].u8EdgeSeen;
					pu8Buffer[3] = sFCU.sBrakes[u8Counter].sLimits[BRAKE_SW__RETRACT].u8EdgeSeen;
					pu8Buffer[4] = sFCU.sBrakes[u8Counter].u8BrakeSWErr;
					pu8Buffer += 5U;

					//MLP
					vNUMERICAL_CONVERT__Array_U16(pu8Buffer, sFCU.sBrakes[u8Counter].sMLP.u16ADC_Sample);
					pu8Buffer += 2U;
					vNUMERICAL_CONVERT__Array_U16(pu8Buffer, sFCU.sBrakes[u8Counter].sMLP.u16ADC_Zero);
					pu8Buffer += 2U;
					vNUMERICAL_CONVERT__Array_S32(pu8Buffer, sFCU.sBrakes[u8Counter].sMLP.s32ADC_Minus_Zero);
					pu8Buffer += 4U;
					vNUMERICAL_CONVERT__Array_F32(pu8Buffer, sFCU.sBrakes[u8Counter].sMLP.f32SystemSpan);
					pu8Buffer += 4U;
					vNUMERICAL_CONVERT__Array_F32(pu8Buffer, sFCU.sBrakes[u8Counter].sMLP.f32BrakePosition_mm);
					pu8Buffer += 4U;

					//stepper system
					vNUMERICAL_CONVERT__Array_S32(pu8Buffer, sFCU.sBrakes[u8Counter].sMove.s32LinearVeloc);
					pu8Buffer += 4U;
					vNUMERICAL_CONVERT__Array_S32(pu8Buffer, sFCU.sBrakes[u8Counter].sMove.s32LinearAccel);
					pu8Buffer += 4U;
					vNUMERICAL_CONVERT__Array_S32(pu8Buffer, sFCU.sBrakes[u8Counter].sMove.s32currentPos);
					pu8Buffer += 4U;


				}//for(u8Counter = 0U; u8Counter < FCU_BRAKE__MAX_BRAKES; u8Counter++)


				pu8Buffer[0] = (Luint8)sFCU.sBrakesGlobal.eBrakeStates;
				pu8Buffer += 1U;

				break;

			default:
				//fall on
				break;

		}//switch(ePacketType)

		//send it
		vSAFEUDP_TX__Commit(u8BufferIndex, u16Length, C_LOCALDEF__LCCM528__ETHERNET_PORT_NUMBER, C_LOCALDEF__LCCM528__ETHERNET_PORT_NUMBER);

	}//if(s16Return == 0)
	else
	{
		//fault

	}//else if(s16Return == 0)

}


/***************************************************************************//**
 * @brief
 * Raw motor position movement
 *
 * @note
 * Huge caution, this can kill the magnets
 *
 * @param[in]		u32Position				The new position in microns
 * @param[in]		u32Index				The motor index
 * @st_funcMD5		71622FBE47C415EB55B42BE035CCFBD8
 * @st_funcID		LCCM655R0.FILE.066.FUNC.003
 */
void vFCU_BRAKES_ETH__MoveMotor_RAW(Luint32 u32Index, Luint32 u32Position)
{

	if(sFCU.sBrakesGlobal.sBrakesDev.u8DevMode == 1U)
	{
		//check the safety key
		if(sFCU.sBrakesGlobal.sBrakesDev.u32DevKey == 0xABCD0987U)
		{

			//make the last flag WELL known that we have modded something
			vFAULTTREE__Set_Flag(&sFCU.sBrakesGlobal.sFaultFlags, 31U);


			switch(u32Index)
			{
				case 0:
					vFCU_BRAKES_STEP__Move(u32Position, 0U);
					break;
				case 1:
					vFCU_BRAKES_STEP__Move(0U, u32Position);
					break;
				case 2:
					vFCU_BRAKES_STEP__Move(u32Position, u32Position);
					break;
				default:
					//do nothing.
					break;
			}//switch(u32Index)

		}
		else
		{
			//key wrong
		}
	}
	else
	{
		//not enabled
	}

}

/***************************************************************************//**
 * @brief
 * In development mode, move the brakes to an I-Beam distance
 * 
 * @param[in]		f32Value				The distance in mm
 * @st_funcMD5		73FE0E25EFA8200BCFB6C8AF305BFF36
 * @st_funcID		LCCM655R0.FILE.066.FUNC.004
 */
void vFCU_BRAKES_ETH__MoveMotor_IBeam(Lfloat32 f32Value)
{

	if(sFCU.sBrakesGlobal.sBrakesDev.u8DevMode == 1U)
	{
		//check the safety key
		if(sFCU.sBrakesGlobal.sBrakesDev.u32DevKey == 0xABCD0987U)
		{

			//make the last flag WELL known that we have modded something
			vFAULTTREE__Set_Flag(&sFCU.sBrakesGlobal.sFaultFlags, 31U);

			sFCU.sBrakes[0].sTarget.f32IBeam_mm = f32Value;
			sFCU.sBrakes[1].sTarget.f32IBeam_mm = f32Value;

			//change state
			//Note you can bugger things up easy because there are no checks here.
			sFCU.sBrakesGlobal.eBrakeStates = BRAKE_STATE__BEGIN_MOVE;

		}
		else
		{
			//key wrong
		}
	}
	else
	{
		//not enabled
	}

}

/***************************************************************************//**
 * @brief
 * Permit brake development/testing mode
 *
 * @param[in]		u32Key1				Key1 should be 0xABCD0987U
 * @param[in]		u32Key0				Key0 shoul dbe 0x1293847
 * @st_funcMD5		483A9F934DE9E882908E3DD1A86B9157
 * @st_funcID		LCCM655R0.FILE.007.FUNC.010
 */
void vFCU_BRAKES_ETH__Enable_DevMode(Luint32 u32Key0, Luint32 u32Key1)
{

	if(u32Key0 == 0x01293847U)
	{

		//make the last flag WELL known that we have modded something
		vFAULTTREE__Set_Flag(&sFCU.sBrakesGlobal.sFaultFlags, 31U);

		//activate mode
		sFCU.sBrakesGlobal.sBrakesDev.u8DevMode = 1U;
		sFCU.sBrakesGlobal.sBrakesDev.u32DevKey = u32Key1;
	}
	else
	{
		//disable mode
		sFCU.sBrakesGlobal.sBrakesDev.u8DevMode = 0U;
		sFCU.sBrakesGlobal.sBrakesDev.u32DevKey = 0U;
	}



}

#endif //C_LOCALDEF__LCCM655__ENABLE_ETHERNET

#endif //C_LOCALDEF__LCCM655__ENABLE_BRAKES
#endif //#if C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE == 1U
//safetys
#ifndef C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE
	#error
#endif
/** @} */
/** @} */
/** @} */

