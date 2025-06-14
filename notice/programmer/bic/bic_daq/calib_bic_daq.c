#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "NoticeBIC_TCB.h"

#define BIC_DAQ_VID  (0x0547)
#define BIC_DAQ_PID (0x2503)

enum EManipInterface {kInterfaceClaim, kInterfaceRelease};

struct dev_open {
   libusb_device_handle *devh;
   uint16_t vendor_id;
   uint16_t product_id;
   int serial_id;
   struct dev_open *next;
} *ldev_open = 0;

static int is_device_open_d(libusb_device_handle *devh);
static unsigned char get_serial_id_d(libusb_device_handle *devh);
static void add_device_d(struct dev_open **list, libusb_device_handle *tobeadded, int sid);
static int handle_interface_id_d(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type);
static void remove_device_id_d(struct dev_open **list, int sid);
libusb_device_handle* nkusb_get_device_handle_d(int sid);
int USB3Reset_d(int sid);
int USB3Read_i_d(int sid, int count, int addr, char *data);
int USB3Read_d(int sid, int addr);
int USB3Read_Block_d(int sid, int count, int addr, char *data);
int DAQopen(int sid);
void DAQclose(int sid);
int DAQread_TDC_CAL_DATASIZE(int sid);
void DAQread_TDC_CAL_DATA(int sid, int data_size, char *data);
int calib_TDC(int sid, int mid, int ch);

static int is_device_open_d(libusb_device_handle *devh)
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

static unsigned char get_serial_id_d(libusb_device_handle *devh)
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

static void add_device_d(struct dev_open **list, libusb_device_handle *tobeadded, int sid)
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

static int handle_interface_id_d(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type)
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
    && (sid == 0xFF || sid == get_serial_id_d(curr->devh))) { 
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

static void remove_device_id_d(struct dev_open **list, int sid)
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
    && (sid == 0xFF || sid == get_serial_id_d(curr->devh))) { 
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

libusb_device_handle* nkusb_get_device_handle_d(int sid) 
{
  struct dev_open *curr = ldev_open;
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
  
  libusb_device_handle *devh = nkusb_get_device_handle_d(sid);
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

int USB3Read_i_d(int sid, int count, int addr, char *data)
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

  libusb_device_handle *devh = nkusb_get_device_handle_d(sid);
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

int USB3Read_d(int sid, int addr)
{
  unsigned char data[4];
  int value;
  int tmp;

  USB3Read_i_d(sid, 1, addr, data);

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

int USB3Read_Block_d(int sid, int count, int addr, char *data)
{
  return USB3Read_i_d(sid, count, addr, data);
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
      if (is_device_open_d(devh)) {
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

      sid_tmp = get_serial_id_d(devh);

      if (sid == 0xFF || sid == sid_tmp) {
        add_device_d(&ldev_open, devh, sid_tmp);
        nopen_devices++;
  
        // Print out the speed of just open device 
        speed = libusb_get_device_speed(dev);
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
  handle_interface_id_d(&ldev_open, sid, 0, kInterfaceClaim);

  if (!nopen_devices)
    return -1;

  devh = nkusb_get_device_handle_d(sid);
  if (!devh) {
    fprintf(stderr, "Could not get device handle for the device.\n");
    return -1;
  }

  return status;
}

// close BIC_DAQ
void DAQclose(int sid)
{
  handle_interface_id_d(&ldev_open, sid, 0, kInterfaceRelease);
  remove_device_id_d(&ldev_open, sid);
}

// read TDC calibration data size in kbyte
int DAQread_TDC_CAL_DATASIZE(int sid)
{
  return USB3Read_d(sid, 0x30000002);
}

// read TDC calibration data
void DAQread_TDC_CAL_DATA(int sid, int data_size, char *data)
{
  int count;

  count = data_size * 256;
  USB3Read_Block_d(sid, count, 0x40000002, data);  
}

int main(void)
{
  int sid = 0;
  int com;
  int link_status[2];
  int daq_mid[40];
  int daq;
  int daq_exist;
  int mid;
  int ch;
  char wfilename[256];
  char rfilename[256];
  FILE *wfp;
  FILE *rfp;
  char *data;

  // init USB
  USB3Init();
  
  // open TCB
  TCBopen(sid);

  // reset TCB
  TCBreset(sid);

  // get link status and ADC mid
  TCBread_LINK_STATUS(sid, link_status);
  TCBread_MID(sid, daq_mid);
  printf("link status = %X %X\n", link_status[1], link_status[0]);
  for (daq = 0; daq < 32; daq++) {
    daq_exist = link_status[0] & (1 << daq);
    if (daq_exist) {
      mid = daq_mid[daq]; 
      printf("DAQ[%d] is found @%d\n", mid, daq + 1);
      daq = 32;
    }
  }
  if (!daq_exist) {
    for (daq = 0; daq < 8; daq++) {
      daq_exist = link_status[1] & (1 << daq);
      if (daq_exist) {
        mid = daq_mid[daq + 32]; 
        printf("DAQ[%d] is found @%d\n", mid, daq + 33);
        daq = 8;
      }
    }
  }

  if (!daq_exist) {
    printf("No DAQ is found!\n");
    return -1;
  }
  
  // align ADC & DRAM
  TCBinit_BIC_ADC(sid, mid);
  TCBinit_DRAM(sid, mid);
  
  // open DAQ
  DAQopen(mid);

  com = 99;
  while (com) {
    printf("\n\n");
    printf("1. Calibrate TDC\n");
    printf("2. Show TDC distribution\n");
    printf("3. Merge lookup table\n");
    printf("0. Quit\n");
    printf("Enter command : ");
    scanf("%d", &com);
    
    if (com == 1) {
      printf("Connect input\n");
      printf("Enter channel to calibrate(1~32) : ");
      scanf("%d", &ch);
      calib_TDC(sid, mid, ch);
    }
    else if (com == 2)
      system("root -l show_distribution.C");
    else if (com == 3) {
      data = (char *)malloc(8192); 
      sprintf(wfilename, "tdc_cal_%d.lut", mid);
      wfp = fopen(wfilename, "wb");
      for (ch = 0; ch < 32; ch++) {
        sprintf(rfilename, "tdc_cal_%d_%d.lut", mid, ch + 1);
        rfp = fopen(rfilename, "rb");
        fread(data, 1, 8192, rfp);
        fclose(rfp);
        fwrite(data, 1, 8192, wfp);
      }
      fclose(wfp);
      free(data);
    }
  }
        
  // close DAQ
  DAQclose(mid);
  TCBclose(sid);
  
  // close usb
  USB3Exit();

  return 0;
}

int calib_TDC(int sid, int mid, int ch)
{
  int ich;
  char filename[256];
  FILE *lfp;
  FILE *dfp;
  FILE *tfp;
  int disc_enable[32];
  int hist[4096];
  int i;
  int count;
  int data_size;
  char data[16384];
  short adc[8192];
  double cnt_all;
  double bin_begin;
  double bin_end;
  double cnt_begin;
  double cnt_end;
  short cal_val[4096];     
  char lut_val[8192];

  // set threshold
  for (ich = 1; ich <= 32; ich++)
    TCBwrite_DISC_THR(sid, mid, ich, 4095);
  TCBwrite_DISC_THR(sid, mid, ch, 1000);
  
  // set disc enable
  for (ich = 1; ich <= 32; ich++)
    disc_enable[ich - 1] = 0;
  disc_enable[ch - 1] = 1;
  TCBwrite_DISC_ENABLE(sid, mid, disc_enable);
  
  // select calibration ch
  TCBwrite_TDC_CAL_CH(sid, mid, ch);

 
  // reset DAQ
  TCBreset(sid);  
  
  // set data file name
  sprintf(filename, "tdc_cal_%d_%d.lut", mid, ch);

  // open file
  lfp = fopen(filename, "wb");

  // open debug file
  dfp = fopen("check_calib.txt", "wt");
  tfp = fopen("cal_lut.txt", "wt");

  // reset histogram
  for (i = 0; i < 4096; i++)
    hist[i] = 0;
  
  // strat calibration
  TCBstart_TDC_CAL(sid, mid);
  
  count = 0;
  while (count < 5000) {
    // read data size
    data_size = DAQread_TDC_CAL_DATASIZE(mid);
    if (data_size) {
      // read data
      DAQread_TDC_CAL_DATA(mid, 16, data);
      memcpy(adc, data, 16384);
      
      // fill histogram
      for (i = 0; i < 8192; i++)      
        hist[adc[i]] = hist[adc[i]] + 1;

      count = count + 1;
      printf("%d / 5000 taken\n", count);
    }
  }

  // stop calibration
  TCBstop_TDC_CAL(sid, mid);
  
  // reset
  TCBreset(sid);

  // get TDC calibration table
  cnt_all = 40960000;
  bin_begin = 0.0;
  bin_end = 0.0;
  cnt_begin = 0.0;
  cnt_end = 0.0;

  for (i = 0; i < 4096; i++) {
    cnt_end = cnt_end + hist[4095 - i];
    bin_end = bin_end + ((cnt_end - cnt_begin) / cnt_all * 1000.0);
    cal_val[4095 - i] = (int)((bin_end + bin_begin) / 2.0 + 0.5);
    cnt_begin = cnt_end;
    bin_begin = bin_end;
  }

  cal_val[0] = 0;

  for (i = 1; i < 4096; i++) 
    cal_val[i] = cal_val[i] + 1;

  memcpy(lut_val, cal_val, 8192);
  fwrite(lut_val, 1, 8192, lfp);

  for (i = 0; i < 4096; i++) {
    fprintf(dfp, "%d\n", hist[i]);
    fprintf(tfp, "%d\n", cal_val[i]);
  }

  fclose(lfp);
  fclose(dfp);
  fclose(tfp);
 
  return 0;
}
