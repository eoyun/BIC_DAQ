#include "NoticeBIC_DAQ.h"

enum EManipInterface {kInterfaceClaim, kInterfaceRelease};

struct dev_open {
   libusb_device_handle *devh;
   uint16_t vendor_id;
   uint16_t product_id;
   int serial_id;
   struct dev_open *next;
} *ldev_open = 0;

// internal functions *********************************************************************************
static int is_device_open(libusb_device_handle *devh);
static unsigned char get_serial_id(libusb_device_handle *devh);
static void add_device(struct dev_open **list, libusb_device_handle *tobeadded, int sid);
static int handle_interface_id(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type);
static void remove_device_id(struct dev_open **list, int sid);
libusb_device_handle* nkusb_get_device_handle(int sid);
int USB3Reset(int sid);
int USB3Read_i(int sid, int count, int addr, char *data);
int USB3Read(int sid, int addr);
int USB3Read_Block(int sid, int count, int addr, char *data);

static int is_device_open(libusb_device_handle *devh)
{
// See if the device handle "devh" is on the open device list

  struct dev_open *curr = ldev_open;
  libusb_device *dev, *dev_curr;
  int bus, bus_curr, addr, addr_curr;

  while (curr) {
    dev_curr = libusb_get_device(curr->devh);
    bus_curr = libusb_get_bus_number(dev_curr);
    addr_curr = libusb_get_device_address(dev_curr);

    dev = libusb_get_device(devh);
    bus = libusb_get_bus_number(dev);
    addr = libusb_get_device_address(dev);

    if (bus == bus_curr && addr == addr_curr) return 1;
    curr = curr->next;
  }

  return 0;
}

static unsigned char get_serial_id(libusb_device_handle *devh)
{
  int ret;
  if (!devh) {
    return 0;
  }
  unsigned char data[1];
  ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, 0xD2, 0, 0, data, 1, 1000);

  if (ret < 0) {
    fprintf(stdout, "Warning: get_serial_id: Could not get serial id.\n");
    return 0;
  }

  return data[0];
}

static void add_device(struct dev_open **list, libusb_device_handle *tobeadded, int sid)
{
  struct dev_open *curr;

  curr = (struct dev_open *)malloc(sizeof(struct dev_open));
  curr->devh = tobeadded;
  curr->vendor_id = BIC_DAQ_VID;
  curr->product_id = BIC_DAQ_PID;
  curr->serial_id = sid;
  curr->next  = *list;
  *list = curr;
}

static int handle_interface_id(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type)
{
  int ret = 0;
  if (!*list) {
    ret = -1;
    return ret;
  }

  struct dev_open *curr = *list;
  struct libusb_device_descriptor desc;
  libusb_device *dev;

  while (curr) {
    dev =libusb_get_device(curr->devh);
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
      fprintf(stdout, "Warning: remove_device: could not get device device descriptior."
                          " Ignoring.\n");
      continue;
    }
    if (desc.idVendor == BIC_DAQ_VID && desc.idProduct == BIC_DAQ_PID
    && (sid == 0xFF || sid == get_serial_id(curr->devh))) { 
      if (manip_type == kInterfaceClaim) {
        if ((ret = libusb_claim_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not claim interface (%d) on device (%u, %u, %u)\n",
                  interface, BIC_DAQ_VID, BIC_DAQ_PID, sid);
        }
      }
      else if (manip_type == kInterfaceRelease) {
        if ((ret =libusb_release_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not release interface (%d) on device (%u, %u, %u)\n",
                  interface, BIC_DAQ_VID, BIC_DAQ_PID, sid);
        }
      }
      else {
        fprintf(stderr, "Error: handle_interface_id: Unknown interface handle request: %d\n.",
                manip_type);
              
        ret = -1;
        return ret;
      }
    }

    curr = curr->next;
  }

  return ret;
}

static void remove_device_id(struct dev_open **list, int sid)
{
  if (!*list) return;

  struct dev_open *curr = *list;
  struct dev_open *prev = 0;
  struct libusb_device_descriptor desc;
  libusb_device *dev;

  while (curr) {
    dev =libusb_get_device(curr->devh);
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
      fprintf(stdout, "Warning, remove_device: could not get device device descriptior." " Ignoring.\n");
      continue;
    }
    if (desc.idVendor == BIC_DAQ_VID && desc.idProduct == BIC_DAQ_PID
    && (sid == 0xFF || sid == get_serial_id(curr->devh))) { 
      if (*list == curr) { 
        *list = curr->next;
        libusb_close(curr->devh);
        free(curr); 
        curr = *list;
      }
      else {
        prev->next = curr->next;
        libusb_close(curr->devh);
        free(curr); 
        curr = prev->next;
      }
    }
    else {
      prev = curr;
      curr = curr->next;
    }    
  }
}

libusb_device_handle* nkusb_get_device_handle(int sid) 
{
  struct dev_open *curr = ldev_open;
  //printf("VID : %x, PID : %x\n",curr->vendor_id, curr->product_id);
  while (curr) {
    if (curr->vendor_id == BIC_DAQ_VID && curr->product_id == BIC_DAQ_PID) {
      if (sid == 0xFF)
        return curr->devh;
      else if (curr->serial_id == sid)
        return curr->devh;
    }

    curr = curr->next;
  }

  return 0;
}

int USB3Reset(int sid)
{
  const unsigned int timeout = 1000;
  unsigned char data;
  int stat = 0;
  
  libusb_device_handle *devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "USB3WriteControl: Could not get device handle for the device.\n");
    return -1;
  }
  
  if ((stat = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, 
                                      0xD6, 0, 0, &data, 0, timeout)) < 0) {
    fprintf(stderr, "USB3WriteControl:  Could not make write request; error = %d\n", stat);
    return stat;
  }
  
  return stat;
}

int USB3Read_i(int sid, int count, int addr, char *data)
{
  const unsigned int timeout = 1000; 
  int length = 8;
  int transferred;
  unsigned char *buffer;
  int stat = 0;
  int nbulk;
  int remains;
  int loop;
  int size = 16384; // 16 kB

  nbulk = count / 4096;
  remains = count % 4096;

  if (!(buffer = (unsigned char *)malloc(size))) {
    fprintf(stderr, "USB3Read: Could not allocate memory (size = %d\n)", size);
    return -1;
  }
  
  buffer[0] = count & 0xFF;
  buffer[1] = (count >> 8)  & 0xFF;
  buffer[2] = (count >> 16)  & 0xFF;
  buffer[3] = (count >> 24)  & 0xFF;
  
  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8)  & 0xFF;
  buffer[6] = (addr >> 16)  & 0xFF;
  buffer[7] = (addr >> 24)  & 0x7F;
  buffer[7] = buffer[7] | 0x80;

  libusb_device_handle *devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "USB3Write: Could not get device handle for the device.\n");
    return -1;
  }

  if ((stat = libusb_bulk_transfer(devh, 0x06, buffer, length, &transferred, timeout)) < 0) {
    fprintf(stderr, "USB3Read: Could not make write request; error = %d\n", stat);
    USB3Reset(sid);
    free(buffer);
    return stat;
  }

  for (loop = 0; loop < nbulk; loop++) {
    if ((stat = libusb_bulk_transfer(devh, 0x82, buffer, size, &transferred, timeout)) < 0) {
      fprintf(stderr, "USB3Read: Could not make read request; error = %d\n", stat);
      USB3Reset(sid);
      return 1;
    }
    memcpy(data + loop * size, buffer, size);
  }

  if (remains) {
    if ((stat = libusb_bulk_transfer(devh, 0x82, buffer, remains * 4, &transferred, timeout)) < 0) {
      fprintf(stderr, "USB3Read: Could not make read request; error = %d\n", stat);
      USB3Reset(sid);
      return 1;
    }
    memcpy(data + nbulk * size, buffer, remains * 4);
  }

  free(buffer);
  
  return 0;
}

int USB3Read(int sid, int addr)
{
  unsigned char data[4];
  int value;
  int tmp;

  USB3Read_i(sid, 1, addr, data);

  value = data[0] & 0xFF;
  tmp = data[1] & 0xFF;
  tmp = tmp << 8;
  value = value + tmp;
  tmp = data[2] & 0xFF;
  tmp = tmp << 16;
  value = value + tmp;
  tmp = data[3] & 0xFF;
  tmp = tmp << 24;
  value = value + tmp;

  return value;
}

int USB3Read_Block(int sid, int count, int addr, char *data)
{
  return USB3Read_i(sid, count, addr, data);
}

// ******************************************************************************************************

// initialize libusb library
void USB3Init(void)
{
  if (libusb_init(0) < 0) {
    fprintf(stderr, "Failed to initialise LIBUSB\n");
    exit(1);
  }
}

// de-initialize libusb library
void USB3Exit(void)
{
  libusb_exit(0); 
}

// open BIC_DAQ
int DAQopen(int sid)
{
  struct libusb_device **devs;
  struct libusb_device *dev;
  struct libusb_device_handle *devh;
  size_t i = 0;
  int nopen_devices = 0; //number of open devices
  int r;
  int interface = 0;
  int sid_tmp;
  int speed;
  int status = 1;

  if (libusb_get_device_list(0, &devs) < 0) 
    fprintf(stderr, "Error: open_device: Could not get device list\n");

  fprintf(stdout, "Info: open_device: opening device Vendor ID = 0x%X, Product ID = 0x%X, Serial ID = %u\n", BIC_DAQ_VID, BIC_DAQ_PID, sid);

  while ((dev = devs[i++])) {
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      fprintf(stdout, "Warning, open_device: could not get device device descriptior." " Ignoring.\n");
      continue;
    }

    if (desc.idVendor == BIC_DAQ_VID && desc.idProduct == BIC_DAQ_PID)  {
      r = libusb_open(dev, &devh);
      if (r < 0) {
        fprintf(stdout, "Warning, open_device: could not open device." " Ignoring.\n");
        continue;
      }

      // do not open twice
      if (is_device_open(devh)) {
        fprintf(stdout, "Info, open_device: device already open." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      // See if sid matches
      // Assume interface 0
      if (libusb_claim_interface(devh, interface) < 0) {
        fprintf(stdout, "Warning, open_device: could not claim interface 0 on the device." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      sid_tmp = get_serial_id(devh);
	  printf("%d : sid | %d : sid tmp\n",sid,sid_tmp);
	  printf("0xFF is %d\n",0xFF);
      if (sid == 0xFF || sid == sid_tmp) {
        add_device(&ldev_open, devh, sid_tmp);
        nopen_devices++;
  
        // Print out the speed of just open device 
        speed = libusb_get_device_speed(dev);
		status = 1;
        switch (speed) {
          case 4:
            fprintf(stdout, "Info: open_device: super speed device opened");
            break;
          case 3:
            fprintf(stdout, "Info: open_device: high speed device opened");
            break;
          case 2:
            fprintf(stdout, "Info: open_device: full speed device opened");
            break;
          case 1:
            fprintf(stdout, "Info: open_device: low speed device opened");
            break;
          case 0:
            fprintf(stdout, "Info: open_device: unknown speed device opened");
            break;
        }
        
        fprintf(stdout, " (bus = %d, address = %d, serial id = %u).\n",
                    libusb_get_bus_number(dev), libusb_get_device_address(dev), sid_tmp);
        libusb_release_interface(devh, interface);
        break;
      }
      else {
        status = 0;
        fprintf(stdout, "No module!!\n");
        libusb_release_interface(devh, interface);
        libusb_close(devh);
      }
    }
  }

  libusb_free_device_list(devs, 1);

  // claim interface
  handle_interface_id(&ldev_open, sid, 0, kInterfaceClaim);

  if (!nopen_devices)
    return -1;

  devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "Could not get device handle for the device.\n");
    return -1;
  }

  return status;
}

// close BIC_DAQ
void DAQclose(int sid)
{
  handle_interface_id(&ldev_open, sid, 0, kInterfaceRelease);
  remove_device_id(&ldev_open, sid);
}

// read data size in kbyte
int DAQread_DATASIZE(int sid)
{
  return USB3Read(sid, 0x30000000);
}

// read DAQ run status, 1 = run, 0 = stopped
int DAQread_RUN(int sid)
{
  return USB3Read(sid, 0x30000001);
}

// read data
void DAQread_DATA(int sid, int data_size, char *data)
{
  int count;

  count = data_size * 256;
  USB3Read_Block(sid, count, 0x40000000, data);  
}

// read TDC calibration data size in kbyte
int DAQread_TDC_CAL_DATASIZE(int sid)
{
  return USB3Read(sid, 0x30000002);
}

// read TDC calibration data
void DAQread_TDC_CAL_DATA(int sid, int data_size, char *data)
{
  int count;

  count = data_size * 256;
  USB3Read_Block(sid, count, 0x40000002, data);  
}

// read APIX data size
int DAQread_APIX_DATASIZE(int sid, int ch)
{
  if (ch == 1)
    return USB3Read(sid, 0x30000004);
  else
    return USB3Read(sid, 0x30000005);
}

// read APIX data
void DAQread_APIX_DATA(int sid, int ch, int data_size, char *data)
{
  int count;

  count = data_size * 256;
  
  if (ch == 1)
    USB3Read_Block(sid, count, 0x40000004, data);  
  else
    USB3Read_Block(sid, count, 0x40000005, data);  
}



