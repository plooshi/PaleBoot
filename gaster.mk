all: gaster

gaster:
	@git stash > /dev/null
	@cat gaster.c | sed 's/main/gaster_ma1n/' | sed 's/static //' | sed 's/unsigned usb_timeout;/unsigned usb_timeout = 5;/' | tee gaster.c > /dev/null
	@make gen