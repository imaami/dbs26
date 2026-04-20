ifndef __O_MK__
define __O_MK__ 1

# Undefine variable $1 if its origin is the environment.
override define undef-env
$(eval
ifneq (,$$(filter environment,$$(origin $1)))
override undefine $1
endif
)
endef

# If variable $1 is defined, and
# - its origin is environment, or
# - it's only whitespace, or
# - it's an empty string,
# then undefine it.
override define undef-env-ws
$(call undef-env,$1)
$(eval
ifneq (undefined,$$(flavor $1))
ifeq (,$$(strip $$(value $1)))
override undefine $1
endif
endif
)
endef

override define set-build-dir
$(call undef-env-ws,O)
$(eval
override O := $$(strip $$(subst //,/,$$(abspath $$(or $$O,$1))/))
)
endef

endif
