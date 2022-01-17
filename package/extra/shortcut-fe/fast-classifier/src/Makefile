obj-m += fast-classifier.o

ifeq ($(SFE_SUPPORT_IPV6),)
SFE_SUPPORT_IPV6=y
endif
ccflags-$(SFE_SUPPORT_IPV6) += -DSFE_SUPPORT_IPV6

ccflags-y += -I$(obj)/../shortcut-fe

obj ?= .

ccflags-y += -Werror -Wall
