# invoke SourceDir generated makefile for mutex.pem4f
mutex.pem4f: .libraries,mutex.pem4f
.libraries,mutex.pem4f: package/cfg/mutex_pem4f.xdl
	$(MAKE) -f C:\Users\Josh\workspace_v10\mutex_CC1352R1_LAUNCHXL_tirtos_ccs/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Josh\workspace_v10\mutex_CC1352R1_LAUNCHXL_tirtos_ccs/src/makefile.libs clean

