#######################################################################################
#
#   This file will build whole OMX project.
#
######################################################################################


all:
	@mkdir -p release
	$(MAKE) -C osal
	$(MAKE) -C core
	$(MAKE) -C component

clean:
	$(MAKE) clean -C osal
	$(MAKE) clean -C core
	$(MAKE) clean -C component
	@rm -rf ./release
