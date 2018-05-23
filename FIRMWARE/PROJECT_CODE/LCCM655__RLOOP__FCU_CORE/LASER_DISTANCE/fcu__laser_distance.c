/**
 * @file		FCU__LASER_DISTANCE.C
 * @brief		Forward looking laser distance
 *
 * @note
 * https://drive.google.com/drive/folders/0B-Gl6KhA0fayOENvbWQtOU0tNWc
 *
 * 2.2 Output at starting of the operation mode
 * Output of the Sensor after mode command (<esc>M<No.><cr>) are following:
 *
 * Mode <No.> 			Output
 * 0 					MOK, no effect
 *
 * 1 					MOK
 * 						Dxxxxx [aaaaa]
 * 						Dxxxxx [aaaaa] ....
 *
 * 2 					MOK
 * 						(Binary format distances)

 *
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
 * @addtogroup FCU__LASER_DISTANCE
 * @ingroup FCU
 * @{ */

#include "../fcu_core.h"
#if C_LOCALDEF__LCCM655__ENABLE_LASER_DISTANCE == 1U

//the structure
extern struct _strFCU sFCU;

//locals
void vFCU_LASERDIST__Append_Byte(Luint8 u8Value);
void vFCU_LASERDIST__Process_Packet(void);


/***************************************************************************//**
 * @brief
 * Init any of the laser distance items
 * 
 * @st_funcMD5		FCC72DBDF50BF7A9B8218E9DE78B2C2B
 * @st_funcID		LCCM655R0.FILE.033.FUNC.001
 */
void vFCU_LASERDIST__Init(void)
{

	//setup eth systems if needed
	vFCU_LASERDIST_ETH__Init();

	//setup the valid checks
	vFCU_LASERDIST_VALID__Init();

	sFCU.sLaserDist.u32BytesSeen_Counter = 0U;
	sFCU.sLaserDist.u32PacketsSeen_Counter = 0U;

	sFCU.sLaserDist.eLaserState = LASERDIST_STATE__RESET;
	sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_D;
	sFCU.sLaserDist.u8NewPacket = 0U;
	sFCU.sLaserDist.u8NewDistanceAvail = 0U;
	sFCU.sLaserDist.u32LaserPOR_Counter = 0U;
	//just set to some obscene distance
	sFCU.sLaserDist.s32Distance_mm = 0;
	sFCU.sLaserDist.s32PrevDistance_mm = 0;
	sFCU.sLaserDist.s32Velocity_mm_s = 0;
	sFCU.sLaserDist.s32PrevVelocity_mm_s = 0;
	sFCU.sLaserDist.s32Accel_mm_ss = 0;
	sFCU.sLaserDist.s32PrevAccel_mm_ss = 0;

	sFCU.sLaserDist.u8AverageCounter = 0U;
	sFCU.sLaserDist.u8AverageCounter_Distance_wait = 0U;
	sFCU.sLaserDist.u8AverageCounter_Velocity_wait = 0U;
	sFCU.sLaserDist.u8AverageCounter_Acceleration_wait = 0U;

	//clear the time dependent arrays
	Luint8 u8Counter;
	for(u8Counter = 0U; u8Counter < 10; u8Counter++) //todo: fix the hardcoded val
	{
		sFCU.sLaserDist.s32PrevDistances_mm[u8Counter] = 0;
		sFCU.sLaserDist.s32PrevVelocities_mm_s[u8Counter] = 0;
	}

	//clear the binary distance mode
	sFCU.sLaserDist.sBinary.unRx.u32 = 0U;
	sFCU.sLaserDist.sBinary.u32Counter__MissedStart = 0U;
	sFCU.sLaserDist.sBinary.u32Counter__BadDistance = 0U;
	sFCU.sLaserDist.sBinary.u32Counter__ErrorCode = 0U;
	//setup the filtering
	vFCU_LASERDIST_FILT__Init();

	vSIL3_FAULTTREE__Init(&sFCU.sLaserDist.sFaultFlags);

}

/***************************************************************************//**
 * @brief
 * Process the laser distance system
 * 
 * @st_funcMD5		68CEF6925DC062A13D4785E601B29F69
 * @st_funcID		LCCM655R0.FILE.033.FUNC.002
 */
void vFCU_LASERDIST__Process(void)
{

	Luint8 u8Counter;
	Luint8 u8Temp;
	Luint8 u8BurstCount;
	Luint8 u8Array[4];


	//check for emulation
	if(sFCU.sLaserDist.sEmu.u8EmulationEnabled == 1U)
	{
		if(sFCU.sLaserDist.sEmu.u32EmuKey == 0x98984343U)
		{
			//assign
			sFCU.sLaserDist.s32Distance_mm = sFCU.sLaserDist.sEmu.s32Distance;
		}
		else
		{
			//fall on
		}
	}
	else
	{
		//fall on
	}


	//handle the LASERDIST laser state
	switch(sFCU.sLaserDist.eLaserState)
	{
		case LASERDIST_STATE__RESET:
			//just fresh out of reset.

			sFCU.sLaserDist.u32LaserPOR_Counter = 0U;

			//vSIL3_FAULTTREE__Set_Flag(&sFCU.sLaserDist.sFaultFlags, 0);

#if 0
			//G - tell the laser to reset
			//<ESC>, G, <CR>
			u8Array[0] = 0x1BU;
			u8Array[1] = 0x47U;
			u8Array[2] = 0x0DU;

			//send it.
			vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 3U);
#endif
			//setup the lasers
			sFCU.sLaserDist.eLaserState = LASERDIST_STATE__WAIT_LASER_RESET;
			break;

		case LASERDIST_STATE__WAIT_LASER_RESET:

			//wait here until the lasers are out of rest.
			//5 seconds onsite hack to wait for the laser up.
			if(sFCU.sLaserDist.u32LaserPOR_Counter > 50U)
			{
				//vSIL3_FAULTTREE__Clear_Flag(&sFCU.sLaserDist.sFaultFlags, 0);

				sFCU.sLaserDist.eLaserState = LASERDIST_STATE__INIT_LASER_TURNON;
			}
			else
			{
				//stay in state
			}
			break;

		case LASERDIST_STATE__INIT_LASER_TURNON:
#if 0
				if(1)
				{
					//tell the pointer to turn on
					//<ESC>, O, 1, <CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x4FU;
					u8Array[2] = 0x31U;
					u8Array[3] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 4U);
				}

				if(0)
				{
					//laser info
					//<ESC>, V, 1, <CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x56U;
					u8Array[2] = 0x31U;
					u8Array[3] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 4U);
				}

				if(0)
				{
					//Run self test - responds 'OK' if good, else 'ERR'
					//<ESC>, V, 2, <CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x56U;
					u8Array[2] = 0x32U;
					u8Array[3] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 4U);
				}

				//Read user parameters in permanent memory
				if(0)
				{
					//Operation mode, No. 1
					//<ESC>, P, 1, <CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x50U;
					u8Array[2] = 0x31U;
					u8Array[3] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 4U);
				}

				if(0)
				{
					//baud, No. 4
					//<ESC>P4<CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x50U;
					u8Array[2] = 0x34U;
					u8Array[3] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 4U);
				}
				if(0)
				{
					//Control Byte2, No. 3 (if extended cm: 128, or mm: 64, or none: 0 )
					//<ESC>P3<CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x50U;
					u8Array[2] = 0x33U;
					u8Array[3] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 4U);
				}

				if(0)
				{
					//binary averaging, No. 22
					//<ESC>P22<CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x50U;
					u8Array[2] = 0x32U;
					u8Array[3] = 0x32U;
					u8Array[4] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 5U);
				}

				if(0)
				{
					//Set measurement mode to continuous ASCII
					//<ESC>, M, 1, <CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x4DU;
					u8Array[2] = 0x31U;
					u8Array[3] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 4U);
				}

				if(0)
				{
					//C - enable continious ascii
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x63U;
					u8Array[2] = 0x0DU;
				}
				if(0)
				{
					//Check all current params
					//<ESC>, L <CR>
					u8Array[0] = 0x1BU;
					u8Array[1] = 0x4CU;
					u8Array[2] = 0x0DU;
					//send it.
					vSIL3_SC16__Tx_ByteArray(C_FCU__SC16_FWD_LASER_INDEX, (Luint8*)&u8Array[0], 3U);
				}
#endif
			sFCU.sLaserDist.eLaserState = LASERDIST_STATE__WAIT_INIT_DONE;
			break;

		case LASERDIST_STATE__WAIT_INIT_DONE:

			//wait until the laser is up
			//nothing to do here.
#ifndef WIN32
			vRM4_DELAYS__Delay_mS(50);
#endif

			//continue to check for new data.
			sFCU.sLaserDist.eLaserState = LASERDIST_STATE__CHECK_NEW_DATA;
			break;

		case LASERDIST_STATE__CHECK_NEW_DATA:

			//do a burst here
			for(u8BurstCount = 0U; u8BurstCount < 3U; u8BurstCount++)
			{

				//see if there is at least one byte of data avail in the FIFO's
				u8Temp = u8SIL3_SC16_USER__Get_ByteAvail(C_FCU__SC16_FWD_LASER_INDEX);
				if(u8Temp == 0U)
				{

				}
				else
				{
					//yep some new laser data avail, what to do with it?

					//clear the counter;
					sFCU.sLaserDist.u32BytesSeen_Counter = 0U;

					//get the byte and send it off for processing if we have enough data
					u8Temp = u8SIL3_SC16_USER__Get_Byte(C_FCU__SC16_FWD_LASER_INDEX);

				//BINARY MODE
					//vFCU_LASERDIST__Append_Byte(u8Temp);


#if 1			    //ASCII MODE
					vFCU_LASERDIST__Append_Byte_ASCII(u8Temp);
#endif
				}

			}//for(u8BurstCount = 0U; u8BurstCount < 3U; u8BurstCount++)


			sFCU.sLaserDist.eLaserState = LASERDIST_STATE__CHECK_NEW_PACKET;
			break;

		case LASERDIST_STATE__CHECK_NEW_PACKET:

			//check the packet state of each laser
			if(sFCU.sLaserDist.u8NewPacket == 1U)
			{
				//we have a new laser packet, process it for distance or error code.

			//Continues binary mode
				//vFCU_LASERDIST__Process_Packet();


#if 1			//Continues ascii mode
				vFCU_LASERDIST__Process_Packet_ASCII();
#endif

				//clear the flag
				sFCU.sLaserDist.u8NewPacket = 0U;

			}
			else
			{
				//no new packet has arrived, loop around
			}

			//back to check for more data
			sFCU.sLaserDist.eLaserState = LASERDIST_STATE__CHECK_NEW_DATA;
			break;

		default:
			//fall on
			break;

	}//switch(sFCU.sLasers.eLaserState)

	//process the laser distance filtering.
	//vFCU_LASERDIST_FILT__Process();

	//no new data
	if(sFCU.sLaserDist.u32BytesSeen_Counter > 5U)
	{
		//500ms expired and no new data, error out
		vSIL3_FAULTTREE__Set_Flag(&sFCU.sLaserDist.sFaultFlags, C_LCCM655__LASER_DISTANCE__FAULT_INDEX__00);
		vSIL3_FAULTTREE__Set_Flag(&sFCU.sLaserDist.sFaultFlags, C_LCCM655__LASER_DISTANCE__FAULT_INDEX__01);
	}
	else
	{
		//seen some bytes
	}

	//no new data
	if(sFCU.sLaserDist.u32PacketsSeen_Counter > 5U)
	{
		//500ms expired and no new data, error out
		vSIL3_FAULTTREE__Set_Flag(&sFCU.sLaserDist.sFaultFlags, C_LCCM655__LASER_DISTANCE__FAULT_INDEX__00);
		vSIL3_FAULTTREE__Set_Flag(&sFCU.sLaserDist.sFaultFlags, C_LCCM655__LASER_DISTANCE__FAULT_INDEX__02);
	}
	else
	{
		//laser is still healthy.
	}

	//finally process the data valid checks
	vFCU_LASERDIST_VALID__Process();

}

/***************************************************************************//**
 * @brief
 * Return the current computed distance.
 * 
 * @st_funcMD5		C2FC97AEBB864B96A218E6F6264B716F
 * @st_funcID		LCCM655R0.FILE.033.FUNC.003
 */
Lint32 s32FCU_LASERDIST__Get_Distance_mm(void)
{
	return sFCU.sLaserDist.s32Distance_mm;
}

/***************************************************************************//**
 * @brief
 * Return the current computed velocity.
 * 
 */
Lint32 s32FCU_LASERDIST__Get_Velocity_mms(void)
{
	return sFCU.sLaserDist.s32Velocity_mm_s;
}

/***************************************************************************//**
 * @brief
 * Return the current computed acceleration.
 * 
 */
Lint32 s32FCU_LASERDIST__Get_Acceleration_mmss(void)
{
	return sFCU.sLaserDist.s32Accel_mm_ss;
}


//process the binary packet.
/***************************************************************************//**
 * @brief
 * ToDo
 * 
 * @st_funcMD5		089FCC7F1278951692A429D26ED21534
 * @st_funcID		LCCM655R0.FILE.033.FUNC.008
 */
void vFCU_LASERDIST__Process_Packet(void)
{
	Lfloat32 f32Delta;
	//update
	sFCU.sLaserDist.s32Distance_mm = (Lint32)sFCU.sLaserDist.sBinary.unRx.u32 / 100;
     /////////////////
	//compute veloc
	f32Delta = (Lfloat32)sFCU.sLaserDist.s32PrevDistance_mm;
	f32Delta -= sFCU.sLaserDist.s32Distance_mm;

	//100Hz
	f32Delta *= .01F;

	//do it.
	sFCU.sLaserDist.s32Velocity_mm_s = (Lint32)f32Delta;

	//////////////////
    //compute accel
    f32Delta = (Lfloat32)sFCU.sLaserDist.s32PrevVelocity_mm_s;
    f32Delta -= sFCU.sLaserDist.s32Velocity_mm_s;

    //100hz
    f32Delta *= .01F;

    //do it.
    sFCU.sLaserDist.s32Accel_mm_ss = (Lint32)f32Delta;

	//save prev
	sFCU.sLaserDist.s32PrevDistance_mm = sFCU.sLaserDist.s32Distance_mm;
	sFCU.sLaserDist.s32PrevVelocity_mm_s = sFCU.sLaserDist.s32Velocity_mm_s;
	sFCU.sLaserDist.s32PrevAccel_mm_ss = sFCU.sLaserDist.s32Accel_mm_ss;

}

/***************************************************************************//**
 * @brief
 * Process the laser packet
 * Take the laser packet byte array that has been captured and process it into a length
 * including fault detection
 * 
 * @st_funcMD5		84A305B5BFCD07B007D11374AA3565E8
 * @st_funcID		LCCM655R0.FILE.033.FUNC.004
 */
void vFCU_LASERDIST__Process_Packet_ASCII(void)
{
	Lfloat32 f32Delta;
	Luint32 u32Distance;
	Luint32 u32Temp;
	Luint8 u8Temp;

	//compute the distance
	//1s
	if(sFCU.sLaserDist.u8NewByteArray[5] == '\r'){
		u8Temp = sFCU.sLaserDist.u8NewByteArray[4];
		u8Temp -= 0x30U;
		u32Distance = (Luint32)u8Temp;

		//10s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[3];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 10U;
		u32Distance += u32Temp;

		//100s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[2];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 100U;
		u32Distance += u32Temp;

		//1000s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[1];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 1000U;
		u32Distance += u32Temp;

		//10000s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[0];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 10000U;
		u32Distance += u32Temp;
	}
	else{
		// there's an extra byte because we're >100m

		u8Temp = sFCU.sLaserDist.u8NewByteArray[5];
		u8Temp -= 0x30U;
		u32Distance = (Luint32)u8Temp;

		//10s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[4];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 10U;
		u32Distance += u32Temp;

		//100s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[3];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 100U;
		u32Distance += u32Temp;

		//1000s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[2];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 1000U;
		u32Distance += u32Temp;

		//10000s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[1];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 10000U;
		u32Distance += u32Temp;

		//100000s
		u8Temp = sFCU.sLaserDist.u8NewByteArray[0];
		u8Temp -= 0x30U;
		u32Temp = (Luint32)u8Temp;
		u32Temp *= 100000U;
		u32Distance += u32Temp;
	}
	//todo: should move to a new function here
	// check for error distance value
	if(u32Distance == 0U){
		//u32Distance == 0 error
		sFCU.sLaserDist.sFaultFlags.u32Flags[0] = 1U;
		sFCU.sLaserDist.u32Counter__BadDistance++;
	}
	else{
		// all good, clear error flag
		sFCU.sLaserDist.sFaultFlags.u32Flags[0] = 0U;

		//update
		sFCU.sLaserDist.s32Distance_mm = (Lint32)u32Distance;
		sFCU.sLaserDist.s32PrevDistances_mm[sFCU.sLaserDist.u8AverageCounter] = sFCU.sLaserDist.s32Distance_mm;

		if(sFCU.sLaserDist.u8AverageCounter_Distance_wait == 9U){
			//enough measurements to compute distance moving avg and vel

			//calculate distance moving average
			Luint8 u8Counter;
			Lint32 s32AvgTemp;
			s32AvgTemp = 0U;
			for(u8Counter = 0U; u8Counter < 10; u8Counter++){ //todo: fix hardcoded value
				s32AvgTemp += sFCU.sLaserDist.s32PrevDistances_mm[u8Counter];
				s32AvgTemp /= 10; //todo: fix hardcoded value
			}

			//done calculating moving average, store it
			sFCU.sLaserDist.s32Distance_Filtered_mm = s32AvgTemp;

			//compute new velocity
			Lint32 s32VelTemp;

			s32VelTemp = sFCU.sLaserDist.s32Distance_mm;

			//move counter forward one temporarily to get the oldest value in the array
			sFCU.sLaserDist.u8AverageCounter++;
			if(sFCU.sLaserDist.u8AverageCounter == 10U){
				sFCU.sLaserDist.u8AverageCounter = 0U;
			}

			s32VelTemp -= sFCU.sLaserDist.s32PrevDistances_mm[sFCU.sLaserDist.u8AverageCounter];
			s32VelTemp *= 5; //.2s time window for velocity calc

			// done calculating velocity
			sFCU.sLaserDist.s32Velocity_mm_s = s32VelTemp;

			//move the counter back one 
			if(sFCU.sLaserDist.u8AverageCounter == 0U){
				sFCU.sLaserDist.u8AverageCounter = 9U;
			}
			else{
				sFCU.sLaserDist.u8AverageCounter--;
			}

			//store velocity
			sFCU.sLaserDist.s32PrevVelocities_mm_s[sFCU.sLaserDist.u8AverageCounter] = sFCU.sLaserDist.s32Velocity_mm_s;

			if(sFCU.sLaserDist.u8AverageCounter_Velocity_wait == 9U){
				//have enough measurements to calc vel moving avg and accel

				//calculate velocity moving average
				s32AvgTemp = 0U;
				for(u8Counter = 0U; u8Counter < 10; u8Counter++){ //todo: fix hardcoded value
					s32AvgTemp += sFCU.sLaserDist.s32PrevVelocities_mm_s[u8Counter];
					s32AvgTemp /= 10; //todo: fix hardcoded value
				}

				//done calculating moving average, store it
				sFCU.sLaserDist.s32Velocity_Filtered_mm_s = s32AvgTemp;

				//calculate acceleration
				Lint32 s32AccelTemp;

				s32AccelTemp = sFCU.sLaserDist.s32Velocity_mm_s;

				//move counter forward one temporarily to get the oldest value in the array
				sFCU.sLaserDist.u8AverageCounter++;
				if(sFCU.sLaserDist.u8AverageCounter == 10U){
					sFCU.sLaserDist.u8AverageCounter = 0U;
				}

				s32AccelTemp -= sFCU.sLaserDist.s32PrevVelocities_mm_s[sFCU.sLaserDist.u8AverageCounter];
				s32AccelTemp *= 5; //.2s time window for acceleration calc

				//done calculating acceleration
				sFCU.sLaserDist.s32Accel_mm_ss = s32AccelTemp;

				//move the counter back one so it can be used for acceleration moving avg
				if(sFCU.sLaserDist.u8AverageCounter == 0U){
					sFCU.sLaserDist.u8AverageCounter = 9U;
				}
				else{
					sFCU.sLaserDist.u8AverageCounter--;
				}

				//store acceleration
				sFCU.sLaserDist.s32PrevAccelerations_mm_ss[sFCU.sLaserDist.u8AverageCounter] = sFCU.sLaserDist.s32Accel_mm_ss;

				if(sFCU.sLaserDist.u8AverageCounter_Acceleration_wait == 9U){
					//calculate acceleration moving average
					s32AvgTemp = 0U;
					for(u8Counter = 0U; u8Counter < 10; u8Counter++){ //todo: fix hardcoded value
						s32AvgTemp += sFCU.sLaserDist.s32PrevAccelerations_mm_ss[u8Counter];
						s32AvgTemp /= 10; //todo: fix hardcoded value
					}

					sFCU.sLaserDist.s32Acceleration_Filtered_mm_ss = s32AvgTemp;
				}
				else{
					//need at least ten measurements to calc a moving avg of accel, increment waiting counter
					sFCU.sLaserDist.u8AverageCounter_Acceleration_wait++;
				}
			}
			else{
				//need at least ten measurements to calc a reasonable accel, increment waiting counter
				sFCU.sLaserDist.u8AverageCounter_Velocity_wait++;
			}
		}
		else{
			// need at least ten measurements to calc a reasonable velocity, increment waiting counter
			sFCU.sLaserDist.u8AverageCounter_Distance_wait++;
		}

		//increment counter for next data point
		sFCU.sLaserDist.u8AverageCounter++; 
		if(sFCU.sLaserDist.u8AverageCounter == 10U){
			sFCU.sLaserDist.u8AverageCounter = 0U;
		}
	}
}


/***************************************************************************//**
 * @brief
 * Append a new byte from the UART into the internal array, handle any error checking
 * 
 * @param[in]		u8Value				The new byte
 * @st_funcMD5		708617CB2763AB7FA8B77C62A942DD43
 * @st_funcID		LCCM655R0.FILE.033.FUNC.005
 */
void vFCU_LASERDIST__Append_Byte(Luint8 u8Value)
{

	//handle the laser distance rx states
	switch(sFCU.sLaserDist.eRxState)
	{
		case LASERDIST_RX__BYTE_D:
			//make sure the first two bits are valid
			//check for the start bit
			if((u8Value & 0x80U) == 0x80U)
			{
				//correct distance measurement
				if((u8Value & 0x40U) == 0x00U)
				{
					//we have the MSB value
					sFCU.sLaserDist.sBinary.unRx.u8[2] = u8Value & 0x3F;
				}
				else
				{
					//error in distance measurement, bit6 = 1
					sFCU.sLaserDist.sBinary.u32Counter__BadDistance++;
						if((u8Value & 0x20U) == 0x20U)
						{
							//we have an error code
							sFCU.sLaserDist.sBinary.u32Counter__ErrorCode++;
						}
				}

				//in any condition, move on
				sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_1;
			}
			else
			{
				//maybe not for us, stay in state
				sFCU.sLaserDist.sBinary.u32Counter__MissedStart++;
			}
			break;

		case LASERDIST_RX__BYTE_1:
            //check if second byte starts with 0 not 1
			if((u8Value & 0x80U) != 0x80U)
			{
			//distance mid
			// = "E" if there is an error
			sFCU.sLaserDist.sBinary.unRx.u8[1] = u8Value;
			}

			//clear the packets seeen counter
			sFCU.sLaserDist.u32PacketsSeen_Counter = 0U;

			sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_2;
			break;

		case LASERDIST_RX__BYTE_2:
            //check if third byte starts with 0 not 1
            if((u8Value & 0x80U) != 0x80U)
            {
            //distance mid
            // = "E" if there is an error
            sFCU.sLaserDist.sBinary.unRx.u8[0] = u8Value;
            }

            //signal that a new packet is ready
            sFCU.sLaserDist.u8NewPacket = 1U;

            //clear the packets seeen counter
            sFCU.sLaserDist.u32PacketsSeen_Counter = 0U;

            sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_D;
            break;

		default:
			//todo, log the error
			break;

	}//switch
}

void vFCU_LASERDIST__Append_Byte_ASCII(Luint8 u8Value)
{

	//handle the laser distance rx states
	switch(sFCU.sLaserDist.eRxState)
	{
		case LASERDIST_RX__BYTE_D:
			//check if first byte is equals "D"
			if(u8Value == 'D')
			{
				//wait for byte next
				sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_1;
			}
			else
			{
				//we are not at the right point for detection of the packet start, loop back
			}
			break;


		case LASERDIST_RX__BYTE_1:
			//check if second byte is equals "Z" for distance > 99m
			//todo, write code to handle z
			if(u8Value == 'Z')
			{
				//wait for next byte and go back to this state
				sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_1;
			}

			sFCU.sLaserDist.u8NewByteArray[0] = u8Value;

			sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_2;
			break;

		case LASERDIST_RX__BYTE_2:

			sFCU.sLaserDist.u8NewByteArray[1] = u8Value;

			sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_3;
			break;

		case LASERDIST_RX__BYTE_3:

			sFCU.sLaserDist.u8NewByteArray[2] = u8Value;
			sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_4;
			break;

		case LASERDIST_RX__BYTE_4:

			sFCU.sLaserDist.u8NewByteArray[3] = u8Value;
			sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_5;
			break;

		case LASERDIST_RX__BYTE_5:

			sFCU.sLaserDist.u8NewByteArray[4] = u8Value;

			//clear the packets seeen counter
			sFCU.sLaserDist.u32PacketsSeen_Counter = 0U;

			//go back and rx the next new packet
			sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_6;
			break;

		case LASERDIST_RX__BYTE_6:

			sFCU.sLaserDist.u8NewByteArray[5] = u8Value;

			//signal that a new packet is ready
			sFCU.sLaserDist.u8NewPacket = 1U;

			//clear the packets seen counter
			sFCU.sLaserDist.u32PacketsSeen_Counter = 0U;

			//go back and rx the next new packet
			sFCU.sLaserDist.eRxState = LASERDIST_RX__BYTE_D;
			break;


		default:
			//todo, log the error
			break;
	}//switch
}

/***************************************************************************//**
 * @brief
 * 100ms Timer interrupt.
 * 
 * @st_funcMD5		229BB8530F4EBC946B1000B889B63533
 * @st_funcID		LCCM655R0.FILE.033.FUNC.006
 */
void vFCU_LASERDIST__100MS_ISR(void)
{

	sFCU.sLaserDist.u32LaserPOR_Counter++;
	sFCU.sLaserDist.u32BytesSeen_Counter++;
	sFCU.sLaserDist.u32PacketsSeen_Counter++;
}


#ifdef WIN32
/***************************************************************************//**
 * @brief
 * On win32, set the raw value only
 * 
 * @param[in]		f32Value				Raw value in laser units
 * @st_funcMD5		DA458286EBCD86D590BBDE6A5D8E372C
 * @st_funcID		LCCM655R0.FILE.033.FUNC.007
 */
void vFCU_LASERDIST_WIN32__Set_DistanceRaw(Lint32 s32Value)
{
	sFCU.sLaserDist.s32Distance_mm = s32Value;
}

#endif


#endif //C_LOCALDEF__LCCM655__ENABLE_LASER_DISTANCE
/** @} */
/** @} */
/** @} */

