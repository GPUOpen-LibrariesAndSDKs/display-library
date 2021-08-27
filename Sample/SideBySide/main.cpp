//For information about side-by-side refer to section #6 of "What is ADL and how to use it.doc" document shipped with ADL SDK 
//This example will demonstrates following concepts related to side-by-side 
//1.How to programmatically detect side-by-side installation.
//2.How to programmatically discover if currently loaded ADL is compatible with AMD graphics driver version installed to specific GPU.
//3.How to resolve incompatibility issue by loading ADL that is shipped with target driver version.
//4.How to programmatically disable compatibility check inside ADL
#include "SideBySide.h"

int main()
{
	bool isSideBySide = false;;
	{
		SideBySide sideBySide;
		//Print list of all system GPUs compatible to loaded ADL. We excpect all GPUs to be printed unless the system is side-by-side
		sideBySide.PrintCompatibleGPUs();

		if (isSideBySide = sideBySide.IsSydeBySideSystem())
		{
			//Confirming that the system is side-by-side. We expect that some of the GPUs are incompatible with currently loaded ADL. Printing them here.		
			sideBySide.PrintIncompatibleGPUs();

			std::vector<AdapterInfoX2> incompatibleGPUS = sideBySide.GetIncompatibleGPUs();
			if (incompatibleGPUS.size() > 0)
			{
				//We confirmed that there are GPUs incompatible with currenly loaded ADL. Let's demonstrate how you can load another instance of ADL that will be compatible with this GPU.
				sideBySide.ReloadAdl(incompatibleGPUS[0]);
				//Printing GPUs again to demonstrate that GPU became compatible
				sideBySide.PrintCompatibleGPUs();
				sideBySide.PrintIncompatibleGPUs();
			}
		}
	}

	if (isSideBySide)
	{
		//In this section we demonstrate how to disable compatibility check inside ADL when working on side-by-side system.
		//With this approach all GPUs on the system will be considered as compatible to currently loaded ADL. 
		//ADL will allow communication to all GPUs even with driver of different version. We don't recommend this approach as AMD can't guaranty results.
		//It is up to 3d party QA and development to test the implementation and confirm stability.
		SideBySide sideBySide(true);
		sideBySide.PrintCompatibleGPUs();
		sideBySide.PrintIncompatibleGPUs();
	}
}