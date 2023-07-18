CONTIKI_PROJECT = randomized-client randomized-server
CONTIKI_TARGET_SOURCEFILES += dht22.c leds.c
all: $(CONTIKI_PROJECT)
PLATFORMS_ONLY = zoul
CONTIKI = ../contiki-ng
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_DRIVERS_SENSOR_DIR)/bme280
include $(CONTIKI)/Makefile.include
