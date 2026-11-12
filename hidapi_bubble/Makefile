all : flash

TARGET:=hidapi_bubble
TARGET_MCU:=CH32V003
CH32V003FUN:=../ch32fun/ch32fun
MINICHLINK?=../ch32fun/minichlink

ADDITIONAL_C_FILES+=../rv003usb/rv003usb.S ../rv003usb/rv003usb.c
EXTRA_CFLAGS:=-I../lib -I../rv003usb

include ../ch32fun/ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean


