#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>


/*
Description:	print Endpoints
*/
static void print_endpoint(const struct libusb_endpoint_descriptor *endpoint) {
	printf("		bEndpointAddress:    %02xh\n", endpoint->bEndpointAddress);
}


/*
Description:	if ADB Interface found -> loop over endpoints
*/
static void get_adb_altsetting(libusb_device_handle *handle, const struct libusb_interface_descriptor *interface) {
	uint8_t i;
	unsigned char data[256];

	libusb_get_string_descriptor_ascii(handle, interface->iInterface, data, sizeof(data));
	if (!strcmp((char *)data, "ADB Interface")) {
		for (i = 0; i < interface->bNumEndpoints; i++) {
			print_endpoint(&interface->endpoint[i]);
		}
	}	
}

/*
Description:	loop over settings per interface
*/
static void get_adb_interface(libusb_device_handle *handle, const struct libusb_interface *interface) {
	uint8_t i;

	for (i = 0; i < interface->num_altsetting; i++) {
		get_adb_altsetting(handle, &interface->altsetting[i]);
	}
}


/*
Description:	loop over interfaces per configuration
*/
static void get_adb_config(libusb_device_handle *handle, struct libusb_config_descriptor *config) {
	uint8_t i;

	for (i = 0; i < config->bNumInterfaces; i++) {
		get_adb_interface(handle, &config->interface[i]);
	}
}


/*
Description: 	read device descriptor
				print busId, deviceId, VendorId, ProductId
				loop over configurations
*/
static void get_adb_device(libusb_device *dev, libusb_device_handle *handle) {
	struct libusb_device_descriptor desc;
	uint8_t i;
	int ret;

	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor");
		return;
	}

	printf("Dev (bus %u, device %u): %04x - %04x\n",
			libusb_get_bus_number(dev), libusb_get_device_address(dev),
			desc.idVendor, desc.idProduct);

	libusb_open(dev, &handle);
	if (handle) {
		for (i = 0; i < desc.bNumConfigurations; i++) {
			struct libusb_config_descriptor *config;

			ret = libusb_get_config_descriptor(dev, i, &config);
			if (LIBUSB_SUCCESS != ret) {
				printf("failed to get config descriptor %u\n", i);
				continue;
			}

			get_adb_config(handle, config);

			libusb_free_config_descriptor(config);
		} 
	}
	libusb_close(handle);
}


int main() {

	libusb_device **devs;
	libusb_context *ctx;
	int ret, i;
	ssize_t cnt;

	ret = libusb_init(&ctx);
	if (ret < 0)
		return ret;

	cnt = libusb_get_device_list(ctx, &devs);
	if (cnt < 0) {
		libusb_exit(ctx);
		return -1;
	}

	for (i = 0; devs[i]; i++) {
		get_adb_device(devs[i], NULL);
	}
		
	libusb_exit(ctx);
}
