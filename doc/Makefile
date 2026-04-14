override DOC_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
override DOC_TGT := $(patsubst Doxyfile%,doc%,$(notdir \
                      $(wildcard $(DOC_DIR)Doxyfile-*)))

ifneq (,$(DOC_TGT))
.PHONY: default all clean $(DOC_TGT) $(DOC_TGT:%=clean-%)

ifneq (.DEFAULT,$(MAKECMDGOALS))
default:; @:
all: | $(DOC_TGT)
clean: | $(DOC_TGT:%=clean-%)

$(foreach x,$(DOC_TGT:doc-%=%),\
  $(eval doc-$x: $$(DOC_DIR)$x/) \
  $(eval $$(DOC_DIR)$x/: $$(DOC_DIR)Doxyfile-$x))

$(DOC_DIR)%/: $(DOC_DIR)Doxyfile-%
	@cd "$(DOC_DIR)" && doxygen -q "$(<F)"

$(DOC_TGT:%=clean-%):
	@$(RM) -r "$(DOC_DIR)$(@:clean-doc-%=%)/"
endif
endif
