#pragma once
#include "Panel.h"
#include "FSToolbox/FSIcm.h"
#include "FSToolbox/LightController.h"
#include "FSToolbox/Hardware.h"
#include "FSToolbox/TimerManager.h"


namespace fssystems
{
	using fstoolbox::FSIcm;
	using fsinterface::FSIID;
	using fstoolbox::LightController;
	using fstoolbox::TimerManager;

	class FUEL :
		public Panel
	{
	private:
		static FUEL * instance;
		VALVE valve_xfeed, valve_eng1, valve_eng2;
		PUMP pump_l_fwd, pump_l_aft, pump_ctr_l, pump_ctr_r, pump_r_fwd, pump_r_aft;
		PIPELINE pipeline_left, pipeline_right;
		bool never_fail = false;  // Currently no FSI fail variable available

		bool switch_xfeed = false;
		bool switch_ctr_left = false;
		bool switch_ctr_right = false;
		bool switch_left_aft = false;
		bool switch_left_fwd = false;
		bool switch_right_aft = false;
		bool switch_right_fwd = false;
		bool centre_capacity = false;
		bool left_capacity = false;
		bool right_capacity = false;
		bool acpower_bus1 = false;
		bool acpower_bus2 = false;
		bool switch_eng1_fire = false;
		bool switch_eng2_fire = false;
		bool switch_apu_fire = false;
		bool dcpower_bat = false;
	public:
		FUEL();

		static void fsiOnVarReceive(FSIID id);
		void onVarReceive(FSIID & id);
		static void timedFunction(double time);
		void run_machines(double time);
	};	
}

