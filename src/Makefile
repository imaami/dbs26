override THIS_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

override BIN := dbs26

override SRC_dbs26 := \
  args.c              \
  dbs26.c

.PHONY: default
default:| $(BIN)

include $(THIS_DIR)../mk/common.mk
