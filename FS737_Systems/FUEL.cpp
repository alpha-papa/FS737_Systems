#include "FUEL.h"
#include <string>

namespace fssystems
{
	FUEL * FUEL::instance = nullptr;

	FUEL::FUEL()
	{
		//debug variable
		is_debug = true;

		instance = this;

		//starting FSI Client for FUEL
		FSIcm::inst->RegisterCallback(fsiOnVarReceive);
		FSIID wanted_vars[] =
		{
			FSIID::MBI_FUEL_CROSSFEED_SWITCH,
			FSIID::MBI_FUEL_CTR_LEFT_PUMP_SWITCH,
			FSIID::MBI_FUEL_CTR_RIGHT_PUMP_SWITCH,
			FSIID::MBI_FUEL_LEFT_AFT_PUMP_SWITCH,
			FSIID::MBI_FUEL_LEFT_FWD_PUMP_SWITCH,
			FSIID::MBI_FUEL_RIGHT_AFT_PUMP_SWITCH,
			FSIID::MBI_FUEL_RIGHT_FWD_PUMP_SWITCH,
			FSIID::SLI_FUEL_CENTRE_CAPACITY,
			FSIID::SLI_FUEL_LEFT_MAIN_CAPACITY,
			FSIID::SLI_FUEL_RIGHT_MAIN_CAPACITY,
			FSIID::SLI_AC_XFR_BUS_1_PHASE_1_VOLTAGE,
			FSIID::SLI_AC_XFR_BUS_2_PHASE_1_VOLTAGE,
			FSIID::MBI_FIRE_ENG1_FIRE_SWITCH_PULL_POS,
			FSIID::MBI_FIRE_ENG2_FIRE_SWITCH_PULL_POS,
			FSIID::MBI_FIRE_APU_FIRE_SWITCH_PULL_POS,
			FSIID::SLI_BAT_BUS_VOLTAGE
		};
		FSIcm::inst->DeclareAsWanted(wanted_vars, sizeof(wanted_vars));

		//standard values
		LightController::registerLight(FSIID::MBI_FUEL_CROSSFEED_VALVE_OPEN_LIGHT, FSIID::MBI_FUEL_CROSSFEED_VALVE_OPEN_LIGHT_DIMMED);
		LightController::registerLight(FSIID::MBI_FUEL_CTR_LEFT_PUMP_LOW_PRESSURE_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_CTR_RIGHT_PUMP_LOW_PRESSURE_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_LEFT_AFT_PUMP_LOW_PRESSURE_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_LEFT_ENG_VALVE_CLOSED_LIGHT, FSIID::MBI_FUEL_LEFT_ENG_VALVE_CLOSED_LIGHT_DIMMED);
		LightController::registerLight(FSIID::MBI_FUEL_LEFT_FILTER_BYPASS_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_LEFT_FWD_PUMP_LOW_PRESSURE_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_LEFT_SPAR_VALVE_CLOSED_LIGHT, FSIID::MBI_FUEL_LEFT_SPAR_VALVE_CLOSED_LIGHT_DIMMED);
		LightController::registerLight(FSIID::MBI_FUEL_RIGHT_AFT_PUMP_LOW_PRESSURE_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_RIGHT_ENG_VALVE_CLOSED_LIGHT, FSIID::MBI_FUEL_RIGHT_ENG_VALVE_CLOSED_LIGHT_DIMMED);
		LightController::registerLight(FSIID::MBI_FUEL_RIGHT_FILTER_BYPASS_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_RIGHT_FWD_PUMP_LOW_PRESSURE_LIGHT);
		LightController::registerLight(FSIID::MBI_FUEL_RIGHT_SPAR_VALVE_CLOSED_LIGHT, FSIID::MBI_FUEL_RIGHT_SPAR_VALVE_CLOSED_LIGHT_DIMMED);

		FSIcm::inst->set<bool>(FSIID::MBI_FUEL_LAMPTEST, false);
		FSIcm::inst->set<float>(FSIID::SLI_FUEL_CENTRE_CAPACITY, 1000);
		FSIcm::inst->set<float>(FSIID::SLI_FUEL_LEFT_MAIN_CAPACITY, 1000);
		FSIcm::inst->set<float>(FSIID::SLI_FUEL_RIGHT_MAIN_CAPACITY, 1000);

		FSIcm::inst->ProcessWrites();

		TimerManager::addTimedCallback(timedFunction);
	}

	void FUEL::fsiOnVarReceive(FSIID id) {
		instance->onVarReceive(id);
	}

	void FUEL::timedFunction(double time)
	{
		instance->run_machines(time);
	}

	void FUEL::run_machines(double time)
	{
		// RUN STATEMACHINES
		//Crosfeed Valve and Engine Valves
		valve_xfeed.run_machine(switch_xfeed, dcpower_bat);
		valve_eng1.run_machine(switch_eng1_fire, true);
		valve_eng2.run_machine(switch_eng2_fire, true);

		//Pumps
		pump_ctr_l.run_machine(switch_ctr_left, acpower_bus2, never_fail, centre_capacity);
		pump_ctr_r.run_machine(switch_ctr_right, acpower_bus1, never_fail, centre_capacity);
		pump_l_aft.run_machine(switch_left_aft, acpower_bus1, never_fail, left_capacity);
		pump_r_aft.run_machine(switch_right_aft, acpower_bus2, never_fail, right_capacity);
		pump_l_fwd.run_machine(switch_left_fwd, acpower_bus1, never_fail, left_capacity);
		pump_r_fwd.run_machine(switch_right_fwd, acpower_bus2, never_fail, right_capacity);

		debug(std::to_string(pump_ctr_l.actual_state));

		//Pipelines
		pumpstate leftstates[] = { pump_l_aft.actual_state, pump_l_fwd.actual_state, pump_ctr_l.actual_state };
		pumpstate rightstates[] = { pump_r_aft.actual_state, pump_r_fwd.actual_state, pump_ctr_r.actual_state };
		pipeline_left.run_machine(leftstates, rightstates, valve_xfeed.actual_state, valve_eng1.actual_state);
		pipeline_right.run_machine(rightstates, leftstates, valve_xfeed.actual_state, valve_eng1.actual_state);

		// SET FSI FUEL VARIABLES
		if (!pipeline_left.actual_state == sSHUTOFF) //ENG 1 SHUTOFF VALVES NOT CLOSED
		{
			if (pipeline_left.actual_state == sUNPRESSURIZED && centre_capacity && (FSIcm::inst->get<double>(FSIID::SLI_ENG1_N1) > 50)) // SUCTION FEED POSSIBLE
			{
				FSIcm::inst->set(FSIID::SLI_FUEL_ENG1_AVAIL, true);
			}
			if (pipeline_left.actual_state == sPRESSURIZED) // PUMPS ACITVE AND DELIVERING FUEL PRESSURE
			{
				FSIcm::inst->set(FSIID::SLI_FUEL_ENG1_AVAIL, true);
			}
			else FSIcm::inst->set(FSIID::SLI_FUEL_ENG1_AVAIL, false);
		}
		else FSIcm::inst->set(FSIID::SLI_FUEL_ENG1_AVAIL, false);

		// SET LIGHTS DEPENDING ON PRESSURE AND SWITCH STATUS
		LightController::set(FSIID::MBI_FUEL_RIGHT_FWD_PUMP_LOW_PRESSURE_LIGHT, !(pump_r_fwd.actual_state == sON) || !switch_right_fwd);
		LightController::set(FSIID::MBI_FUEL_RIGHT_AFT_PUMP_LOW_PRESSURE_LIGHT, !(pump_r_aft.actual_state == sON) || !switch_right_aft);
		LightController::set(FSIID::MBI_FUEL_LEFT_FWD_PUMP_LOW_PRESSURE_LIGHT, !(pump_l_fwd.actual_state == sON) || !switch_left_fwd);
		LightController::set(FSIID::MBI_FUEL_LEFT_AFT_PUMP_LOW_PRESSURE_LIGHT, !(pump_l_aft.actual_state == sON) || !switch_left_aft);
		LightController::set(FSIID::MBI_FUEL_CTR_RIGHT_PUMP_LOW_PRESSURE_LIGHT, !(pump_ctr_r.actual_state == sON) && switch_ctr_right);
		LightController::set(FSIID::MBI_FUEL_CTR_LEFT_PUMP_LOW_PRESSURE_LIGHT, !(pump_ctr_l.actual_state == sON) && switch_ctr_left);
		// SET CROSSFEED VALVE LIGHT
		LightController::set(FSIID::MBI_FUEL_CROSSFEED_VALVE_OPEN_LIGHT, (valve_xfeed.actual_state == sOPEN));	// To-Do: Add dimmed condition
		LightController::ProcessWrites();
	}

	void FUEL::onVarReceive(FSIID & id)
	{
		switch_xfeed = FSIcm::inst->get<bool>(FSIID::MBI_FUEL_CROSSFEED_SWITCH);
		switch_ctr_left = FSIcm::inst->get<bool>(FSIID::MBI_FUEL_CTR_LEFT_PUMP_SWITCH);
		switch_ctr_right = FSIcm::inst->get<bool>(FSIID::MBI_FUEL_CTR_RIGHT_PUMP_SWITCH);
		switch_left_aft = FSIcm::inst->get<bool>(FSIID::MBI_FUEL_LEFT_AFT_PUMP_SWITCH);
		switch_left_fwd = FSIcm::inst->get<bool>(FSIID::MBI_FUEL_LEFT_FWD_PUMP_SWITCH);
		switch_right_aft = FSIcm::inst->get<bool>(FSIID::MBI_FUEL_RIGHT_AFT_PUMP_SWITCH);
		switch_right_fwd = FSIcm::inst->get<bool>(FSIID::MBI_FUEL_RIGHT_FWD_PUMP_SWITCH);
		centre_capacity = (FSIcm::inst->get<float>(FSIID::SLI_FUEL_CENTRE_CAPACITY) > 0);
		left_capacity = (FSIcm::inst->get<float>(FSIID::SLI_FUEL_LEFT_MAIN_CAPACITY) > 0);
		right_capacity = (FSIcm::inst->get<float>(FSIID::SLI_FUEL_RIGHT_MAIN_CAPACITY) > 0);
		acpower_bus1 = (FSIcm::inst->get<float>(FSIID::SLI_AC_XFR_BUS_1_PHASE_1_VOLTAGE) > 100);
		acpower_bus2 = (FSIcm::inst->get<float>(FSIID::SLI_AC_XFR_BUS_2_PHASE_1_VOLTAGE) > 100);
		switch_eng1_fire = FSIcm::inst->get<bool>(FSIID::MBI_FIRE_ENG1_FIRE_SWITCH_PULL_POS);
		switch_eng2_fire = FSIcm::inst->get<bool>(FSIID::MBI_FIRE_ENG2_FIRE_SWITCH_PULL_POS);
		switch_apu_fire = FSIcm::inst->get<bool>(FSIID::MBI_FIRE_APU_FIRE_SWITCH_PULL_POS);
		dcpower_bat = (FSIcm::inst->get<float>(FSIID::SLI_BAT_BUS_VOLTAGE) > 20);
		
		//CROSSFEED
		if (id == FSIID::MBI_FUEL_CROSSFEED_SWITCH)
		{
			if (FSIcm::inst->get<bool>(FSIID::MBI_FUEL_CROSSFEED_SWITCH))
			{
				debug("FUEL Crossfeed On");
			}
			else
			{
				debug("FUEL Crossfeed Off");
			}
		}


		//FUEL CTR L
		if (id == FSIID::MBI_FUEL_CTR_LEFT_PUMP_SWITCH)
		{
			if (FSIcm::inst->get<bool>(FSIID::MBI_FUEL_CTR_LEFT_PUMP_SWITCH))
			{
				debug("FUEL CTR LEFT PUMP On");
			}
			else
			{
				debug("FUEL CTR LEFT PUMP Off");
			}
		}

		//FUEL CTR R
		if (id == FSIID::MBI_FUEL_CTR_RIGHT_PUMP_SWITCH)
		{
			if (FSIcm::inst->get<bool>(FSIID::MBI_FUEL_CTR_RIGHT_PUMP_SWITCH))
			{
				debug("FUEL CTR RIGHT PUMP On");
			}
			else
			{
				debug("FUEL CTR RIGHT PUMP Off");
			}
		}


		//FUEL AFT L
		if (id == FSIID::MBI_FUEL_LEFT_AFT_PUMP_SWITCH)
		{
			if (FSIcm::inst->get<bool>(FSIID::MBI_FUEL_LEFT_AFT_PUMP_SWITCH))
			{
				debug("FUEL AFT LEFT PUMP On");
			}
			else
			{
				debug("FUEL AFT LEFT PUMP Off");
			}
		}

		//FUEL AFT R
		if (id == FSIID::MBI_FUEL_RIGHT_AFT_PUMP_SWITCH)
		{
			if (FSIcm::inst->get<bool>(FSIID::MBI_FUEL_RIGHT_AFT_PUMP_SWITCH))
			{
				debug("FUEL AFT RIGHT PUMP On");
			}
			else
			{
				debug("FUEL AFT RIGHT PUMP Off");
			}
		}

		//FUEL FWD L
		if (id == FSIID::MBI_FUEL_LEFT_FWD_PUMP_SWITCH)
		{
			if (FSIcm::inst->get<bool>(FSIID::MBI_FUEL_LEFT_FWD_PUMP_SWITCH))
			{
				debug("FUEL FWD LEFT PUMP On");
			}
			else
			{
				debug("FUEL FWD LEFT PUMP Off");
			}
		}

		//FUEL FWD R
		if (id == FSIID::MBI_FUEL_RIGHT_FWD_PUMP_SWITCH)
		{
			if (FSIcm::inst->get<bool>(FSIID::MBI_FUEL_RIGHT_FWD_PUMP_SWITCH))
			{
				debug("FUEL FWD RIGHT PUMP On");
			}
			else
			{
				debug("FUEL FWD RIGHT PUMP Off");
			}
		}
	}
}