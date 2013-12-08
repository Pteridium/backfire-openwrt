obj-y		+= clk.o cpu.o cs.o gpio.o nvram.o irq.o prom.o setup.o \
		   timer.o dev-dsp.o dev-enet.o dev-flash.o dev-hsspi.o \
		   dev-pcmcia.o dev-spi.o dev-trng.o dev-uart.o dev-usb-ehci.o \
		   dev-usb-ohci.o dev-wdt.o pci-ath9k-fixup.o pci-rt2x00-fixup.o
obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o

obj-y		+= boards/

EXTRA_CFLAGS += -Werror
