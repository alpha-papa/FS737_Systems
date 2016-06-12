#pragma once
#include "Panel.h"
#include "FSToolbox/FSIcm.h"
#include "FSToolbox/LightController.h"

namespace fssystems
{
	using fstoolbox::FSIcm;
	using fsinterface::FSIID;
	using fstoolbox::LightController;

	class PNEUMATICS :
		public Panel
	{
	public:
		PNEUMATICS();
		static void fsiOnVarReceive(FSIID id);
	};
}

