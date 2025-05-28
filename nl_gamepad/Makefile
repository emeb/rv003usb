all : flash

TARGET:=nl_gamepad
TARGET_MCU:=CH32V003
CH32V003FUN:=../ch32v003fun/ch32fun
MINICHLINK?=../ch32v003fun/minichlink

ADDITIONAL_C_FILES+=../rv003usb/rv003usb.S ../rv003usb/rv003usb.c
EXTRA_CFLAGS:=-I../lib -I../rv003usb

include ../ch32v003fun/ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean


