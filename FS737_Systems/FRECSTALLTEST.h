#pragma once
#include "Panel.h"
#include "FSToolbox/FSIcm.h"
#include "FSToolbox/LightController.h"

namespace fssystems
{
	using fstoolbox::FSIcm;
	using fsinterface::FSIID;
	using fstoolbox::LightController;

	class FRECSTALLTEST :
		public Panel
	{
	private:
		static FRECSTALLTEST * instance;
	public:
		FRECSTALLTEST();
		static void fsiOnVarReceive(FSIID id);
		void onVarReceive(FSIID & id);
	};
}

