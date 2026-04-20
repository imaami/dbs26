override DOC_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
override DOC_TGT := $(patsubst Doxyfile%,doc%,$(notdir \
                      $(wildcard $(DOC_DIR)Doxyfile-*)))

ifneq (,$(DOC_TGT))
.PHONY: default all clean $(DOC_TGT) $(DOC_TGT:%=clean-%)

ifneq (.DEFAULT,$(MAKECMDGOALS))
default:; @:
all: | $(DOC_TGT)
clean: | $(DOC_TGT:%=clean-%)

-include $(DOC_DIR)../mk/o.mk
$(call set-build-dir,$(DOC_DIR)../build)

$(foreach x,$(DOC_TGT:doc-%=%),\
  $(eval doc-$x: $$O$x/) \
  $(eval $$O$x/: $$(DOC_DIR)Doxyfile-$x))

$O%/: $(DOC_DIR)Doxyfile-%
	@DOC_DIR="$(DOC_DIR)" O="$O" doxygen -q "$<"

$(DOC_TGT:%=clean-%):
	$(RM) -r "$O$(@:clean-doc-%=%)/"
endif
endif
