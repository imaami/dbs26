override REPO_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

include $(REPO_DIR)mk/o.mk
$(call set-build-dir,$(REPO_DIR)build)

override import-sub-dir =$(sort $(shell \
  $(MAKE) -Rpqrs MAKECMDGOALS:=.DEFAULT \
          -f "$(strip $1)" 2> /dev/null \
  | grep ^\\.PHONY:| xargs printf %s\\n \
  | grep '^[a-z][0-9A-Z_a-z.-]*$$'))

override SUB_DIRS = \
  $(eval override SUB_DIRS:=$$(patsubst \
    $(REPO_DIR)%/Makefile,%,$$(wildcard \
    $(REPO_DIR)*/Makefile)))$(SUB_DIRS)

$(foreach d,$(SUB_DIRS),$(eval override \
  TARGETS_$d := $$(call import-sub-dir, \
    $(REPO_DIR)$d/Makefile)))

$(eval override TARGETS := $$(sort \
  $$(filter-out default, \
    $(SUB_DIRS:%=$$(TARGETS_%)) \
  ) \
))

.PHONY: default $(TARGETS)
default $(TARGETS):; @:

$(foreach d,$(SUB_DIRS), \
  $(if $(TARGETS_$d), \
    $(if $(filter default,$(TARGETS_$d)),, \
      $(eval \
        default: | $(REPO_DIR)$d/default \
      ) \
      $(eval \
        .PHONY: $(REPO_DIR)$d/default \
      ) \
      $(eval \
        $(REPO_DIR)$d/default:; \
          @+$$(MAKE) -C "$$(@D)" O="$$O" \
      ) \
    ) \
    $(eval \
      .PHONY: $$(TARGETS_$d:%=$(REPO_DIR)$d/%) \
    ) \
    $(eval \
      $$(TARGETS_$d:%=$(REPO_DIR)$d/%):; \
        @+$$(MAKE) -C "$$(@D)" $$(@F) O="$$O" \
    ) \
    $(foreach x,$(TARGETS_$d), \
      $(eval \
        $x: | $(REPO_DIR)$d/$x \
      ) \
    ) \
  ) \
)

$(if $(filter purge,$(TARGETS)), \
  $(foreach d,$(SUB_DIRS), \
    $(if $(filter purge,$(TARGETS_$d)),, \
      $(if $(filter clean,$(TARGETS_$d)), \
        $(eval \
          purge: | $(REPO_DIR)$d/clean \
        ) \
      ) \
    ) \
  ) \
)
