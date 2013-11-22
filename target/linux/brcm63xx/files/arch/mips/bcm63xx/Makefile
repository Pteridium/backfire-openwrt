obj-y		+= clk.o cpu.o cs.o gpio.o irq.o prom.o setup.o timer.o \
		   dev-dsp.o dev-enet.o dev-pcmcia.o dev-uart.o dev-wdt.o \
		   dev-usb-ohci.o dev-usb-ehci.o dev-usb-udc.o
obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o

obj-y		+= boards/

EXTRA_CFLAGS += -Werror
