#include "Hardware.h"

namespace fssystems
{

	// VALVES for generic valve simulation

	VALVE::VALVE()
	{
		valvestate actual_state = sSHUT;
	}

	void VALVE::run_machine(bool open_command, bool power)
	{
		switch (actual_state) {
		case sCLOSING:
			if (open_command && power) actual_state = sOPENING;
			else actual_state = sSHUT;
			break;
		case sSHUT:
			if (open_command && power) actual_state = sOPENING;
			break;
		case sOPEN:
			if (!open_command && power) actual_state = sCLOSING;
			break;
		case sOPENING:
			if (!open_command && power) actual_state = sCLOSING;
			else actual_state = sOPEN;
			break;
		default:
			actual_state = sSHUT;
			break;
		}
	}
	
	void VALVE::setstate(valvestate new_state)
	{
		actual_state = new_state;
	}

	// PUMPS for generic pump simulation
	PUMP::PUMP()
	{
		pumpstate actual_state = sOFF;
	}

	void PUMP::run_machine(bool _switch, bool power, bool fail, bool fluid)
	{
		/*bool state_finalized = false;
		while (!state_finalized)
		{
		pumpstate old_state = actual_state;
		state_finalized = false;
		*/
		switch (actual_state) {
		case sON:
			if (!_switch || !power) actual_state = sOFF;
			if (!fluid) actual_state = sNOFLUID;
			if (fail) actual_state = sFAIL;
			break;
		case sOFF:
			if (fail) actual_state = sFAIL;
			if (_switch && power) actual_state = sON;
			break;
		case sFAIL:
			break;
		case sNOFLUID:
			if (fluid && _switch && power) actual_state = sON;
			if (fail) actual_state = sFAIL;
			break;
		default:
			actual_state = sOFF;
			break;
		}
			/*state_finalized = (actual_state == old_state);
		}*/
	}
	void PUMP::setstate(pumpstate new_state)
	{
		actual_state = new_state;
	}

	// PIPELINES, special use for Fuel System
	PIPELINE::PIPELINE()
	{
		actual_state = sSHUTOFF;
	}
	void PIPELINE::run_machine(pumpstate ownpumps[], pumpstate foreignpumps[], valvestate xfeed, valvestate shutoff)
	{
		bool pump_available = false;
		for (int i = 0; i == (sizeof(ownpumps) / sizeof(ownpumps[0])); i++)
		{
			if (ownpumps[i] == sON || (foreignpumps[i] == sON && xfeed)) pump_available = true;
		}
		switch (actual_state) {
		case sSHUTOFF:
			if (shutoff == sSHUT) actual_state = sSHUTOFF;
			else if (pump_available) actual_state = sPRESSURIZED;
			else actual_state = sUNPRESSURIZED;
			break;
		case sUNPRESSURIZED:
			if (shutoff == sSHUT) actual_state = sSHUTOFF;
			else if (pump_available) actual_state = sPRESSURIZED;
			break;
		case sPRESSURIZED:				
			if (shutoff == sSHUT) actual_state = sSHUTOFF;
			else if (!pump_available) actual_state = sUNPRESSURIZED;
			break;
		default:
			actual_state = sSHUTOFF;
			break;
		}
	}

	void PIPELINE::setstate(pipelinestate new_state)
	{
		actual_state = new_state;
	}
}