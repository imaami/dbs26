override PROJECT_ROOT := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

override query-targets = $(sort $(shell \
  $(MAKE) -Rpqrs MAKECMDGOALS:=.DEFAULT \
          -f "$(strip $1)" 2> /dev/null \
  | grep ^\\.PHONY:| xargs printf %s\\n \
  | grep '^[a-z][0-9A-Z_a-z.-]*$$'))

override PROJECT_DIRS = \
  $(eval override PROJECT_DIRS:=$$(patsubst \
    $(PROJECT_ROOT)%/Makefile,%,$$(wildcard \
    $(PROJECT_ROOT)*/Makefile)))$(PROJECT_DIRS)

$(foreach d,$(PROJECT_DIRS), \
  $(eval override TARGETS_$d \
    :=$$(call query-targets, \
      $(PROJECT_ROOT)$d/Makefile \
    ) \
  ) \
)

$(eval override TARGETS := $$(sort \
  $$(filter-out default, \
    $(PROJECT_DIRS:%=$$(TARGETS_%)) \
  ) \
))

.PHONY: default $(TARGETS)
default $(TARGETS):; @:

$(foreach d,$(PROJECT_DIRS), \
  $(if $(TARGETS_$d), \
    $(if $(filter default,$(TARGETS_$d)),, \
      $(eval \
        default: | $(PROJECT_ROOT)$d/default \
      ) \
      $(eval \
        .PHONY: $(PROJECT_ROOT)$d/default \
      ) \
      $(eval \
        $(PROJECT_ROOT)$d/default:; \
          @+$$(MAKE) -C "$$(@D)" \
      ) \
    ) \
    $(eval \
      .PHONY: $$(TARGETS_$d:%=$(PROJECT_ROOT)$d/%) \
    ) \
    $(eval \
      $$(TARGETS_$d:%=$(PROJECT_ROOT)$d/%):; \
        @+$$(MAKE) -C "$$(@D)" $$(@F) \
    ) \
    $(foreach x,$(TARGETS_$d), \
      $(eval \
        $x: | $(PROJECT_ROOT)$d/$x \
      ) \
    ) \
  ) \
)
