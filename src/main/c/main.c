// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "flash.h"
#include "router_images.h"
#include "router_types.h"
#include "socket.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

const char* interfaceName = "eth0";

typedef const unsigned char* (*GetResourceFunc)(size_t* len);
int RDO_InitializeButton(void);
int RDO_GetButton(void);

#ifdef ISRAEL
const unsigned char* rdo_GetResource_firmwareOM5PACIsrael_bin(size_t* len);
const unsigned char* rdo_GetResource_firmwareOM5PANIsrael_bin(size_t* len);
#define GetACFirmwareResource rdo_GetResource_firmwareOM5PACIsrael_bin
#define GetANFirmwareResource rdo_GetResource_firmwareOM5PANIsrael_bin
#else
const unsigned char* rdo_GetResource_firmwareOM5PAC_bin(size_t* len);
const unsigned char* rdo_GetResource_firmwareOM5PAN_bin(size_t* len);
#define GetACFirmwareResource rdo_GetResource_firmwareOM5PAC_bin
#define GetANFirmwareResource rdo_GetResource_firmwareOM5PAN_bin
#endif

int main(int argc, const char** argv) {
  if (geteuid() != 0) {
    while (true) {
      fprintf(stderr, "You must run this program as admin (roborio) or root (pi)\n");
      sleep(1);
    }
  }

  bool skipButton = false;

  if (argc > 2 && strcmp(argv[1], "skipButton") == 0) {
    skipButton = true;
  }

  if (!skipButton && !RDO_InitializeButton()) {
    fprintf(stderr, "Failed to initialize button\n");
    return 1;
  }

  fprintf(stdout, "Started\n");

  fprintf(stdout, "Welcome to the roboRIO Radio Firmware Updater\n");

#ifdef ISRAEL
  fprintf(stdout, "You are running the Israel version of the tool\n");
#else
  fprintf(stdout, "You are not running the Israel version. If you are in Israel, please download the correct tool version\n");
#endif

  int counter = 49;
  while (!skipButton) {
    counter++;
    if (counter == 50) {
      fprintf(stdout, "Waiting for User Button Press\n");
      counter = 0;
    }
    if (RDO_GetButton()) {
      break;
    }
    usleep(20000);
  }

  fprintf(stdout, "Detecting Device\n");

  // Detect device type

  GetResourceFunc resourceFunc = GetACFirmwareResource;


  fprintf(stdout, "Extracting Firmware\n");

  size_t bufLen = 0;
  const unsigned char* buf = resourceFunc(&bufLen);

  const char* deviceImage = "/tmp/firmware.bin";
  int fd = creat(deviceImage, 0);
  if (fd < 0) {
    fprintf(stderr, "Failed to create %s\n", deviceImage);
  }

  ssize_t written = write(fd, buf, bufLen);

  if (written != (ssize_t)bufLen) {
    close(fd);
    printf("Failed to write firmware\n");
    return 1;
  }

  close(fd);

  fprintf(stdout, "Checking Firmware\n");

  router_images_init();

  int ret = router_images_verify_path(deviceImage);
  if (ret >= 0) {
    fprintf(stdout, "Found Correct Firmware\n");
  } else {
    fprintf(stdout, "Failed to load firmware\n");
    return 1;
  }

  fprintf(stdout, "Flashing!\n");

  ret = flash_start(interfaceName);

  if (ret != 0) {
    fprintf(stdout, "Flash Failed\n");
    return 1;
  } else {
    fprintf(stdout, "Flashed!\n");
  }

  return 0;

}
