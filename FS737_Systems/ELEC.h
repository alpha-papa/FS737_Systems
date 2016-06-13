#pragma once
#include "Panel.h"
#include "FSToolbox/FSIcm.h"
#include "FSToolbox/LightController.h"


/*  ### FS737 ELEC 0.1 ###
SIMPLIFIED SIMULATION OF B737-800 ELECTRICAL SYSTEM
by Marcel Haupt and Arvid Preuss, 2016

FOLLOWING CONDITIONS ARE SUPPOSED:
* ALL GENERATORS RTL
* BATTERY INFINITELY LOADED
* NO FAILURES

TO-DO (not in prioritized order):
* Simulate failures upon receive of IOS fails
* Create FSI variables for varios new busses and uncomment respective code
* Create reset() method: all values are to be set to their initial state
* Create failsafe() method: all values are to be set to "online" values regardless of environmental states and switches
* Detailed comments and more / more precise debug messages
*/


namespace fssystems
{
	using fstoolbox::FSIcm;
	using fsinterface::FSIID;
	using fstoolbox::LightController;

	class AC_Powersource
	{
	public:
		bool isAvailable = false, isOnline = false;

		bool SwitchOn();
		void SwitchOff();
		void Available();
		void Unavailable();
	};

	class IDG
	{
	private:
		AC_Powersource * assigned_powersource;

	public:
		//          /Ready to Load /CSD connected 
		bool isConnected = true;

		IDG(AC_Powersource & powersource) :
			assigned_powersource(&powersource) { }
		void Disconnect();
	};


	class AC_BUS
	{
	public:
		bool isPowered = false;
		bool sourceOff = true;

		AC_Powersource * powersource;
		AC_Powersource * selected_source;

		AC_BUS(AC_Powersource & disconnected) :
			powersource(&disconnected), selected_source(&disconnected)
		{}

		void connect(AC_Powersource & new_powersource);
		void disconnect(AC_Powersource & disconnect);
		void select(AC_Powersource & new_source);
	};


	class ELEC :
		public Panel
	{
	private:
		bool bustransfer_auto = true, sby_pwr_auto = true, sby_pwr_bat = false, battery_online = false;
		AC_Powersource eng1_gen;
		AC_Powersource eng2_gen;
		AC_Powersource ext_pwr_l;
		AC_Powersource ext_pwr_r;
		AC_Powersource apu_gen1;
		AC_Powersource apu_gen2;
		IDG idg1;
		IDG idg2;
		AC_BUS ac_bus1;
		AC_BUS ac_bus2;

		static ELEC * instance;
	public:
		static AC_Powersource disconnected; // unpowered dummy to have some value available for the AC_BUS class

		ELEC();
		static void fsiOnVarReceive(FSIID id);
		void onVarReceive(FSIID & id);

		void simElectrics();

		void switchAC1Systems(bool power);
		void switchAC2Systems(bool power);
		void switchDCSystems(bool power);
	};

}