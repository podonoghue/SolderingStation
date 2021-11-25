/*
 * UsbCommandMessage.h
 *
 *  Created on: 9 Dec 2019
 *      Author: podonoghue
 */

#ifndef SOURCES_BOOTLOADERINFO_H_
#define SOURCES_BOOTLOADERINFO_H_

#include <stdint.h>

// USB messages are packed data in LE (native) format
#pragma pack(push, 1)

/// Magic number used to reboot into ICP mode
static constexpr uint32_t MAGIC_NUMBER = 0xA55A1234;

/**
 * Structure of Boot information in Image Flash memory
 */
struct BootInformation {
   uint32_t *magicNumber;        ///< Pointer to magic number location to force ICP
   uint32_t softwareVersion;     ///< Version of this software image
   uint32_t hardwareVersion;     ///< Identifies the hardware this image is intended for
   uint32_t checksum;            ///< Filled in by bootloader

   constexpr BootInformation(
         uint32_t *magicNumber,
         uint32_t softwareVersion,
         uint32_t hardwareVersion,
         uint32_t checksum) :
            magicNumber(magicNumber),
            softwareVersion(softwareVersion),
            hardwareVersion(hardwareVersion),
            checksum(checksum) {
   }
};

// Each unique hardware should define a new number here
static constexpr uint32_t HW_LOGIC_BOARD_V2     = 1;
static constexpr uint32_t HW_LOGIC_BOARD_V3     = 2;
static constexpr uint32_t HW_LOGIC_BOARD_V4     = 3;
static constexpr uint32_t HW_SOLDER_STATION_V3  = 4;

static constexpr uint32_t BOOTLOADER_VERSION    = 4;

/**
 * Commands available
 */
enum UsbCommand : uint32_t {
   UsbCommand_Nop,            ///< No operation
   UsbCommand_Identify,       ///< Identify boot-loader and hardware versions etc
   UsbCommand_EraseFlash,     ///< Erase all of flash image
   UsbCommand_ReadBlock,      ///< Read block from flash
   UsbCommand_ProgramBlock,   ///< Program block to flash
   UsbCommand_Reset,          ///< Reset device
};

/**
 * Result of command
 */
enum UsbCommandStatus : uint32_t {
   UsbCommandStatus_OK,          ///< OK result
   UsbCommandStatus_Failed,      ///< Failed
};

/** Maximum size of data in message e.g. flash data block */
static constexpr unsigned MAX_MESSAGE_DATA = 1024;

/**
 * Get command name as string
 *
 * @param command
 *
 * @return Name as string
 *
 * @note return value is a pointer to a STATIC object - do not free
 */
static inline const char *getCommandName(UsbCommand command) {
   static const char * const names[] = {
         "UsbCommand_Nop",
         "UsbCommand_Identify",
         "UsbCommand_EraseFlash",
         "UsbCommand_ReadBlock",
         "UsbCommand_ProgramBlock",
         "UsbCommand_Reset",
   };
   const char *name = "Unknown";
   if (command < (sizeof(names)/sizeof(names[0]))) {
      name = names[command];
   }
   return name;
}

#if __cplusplus >= 201703L
/**
 * Get hardware type as string
 *
 * @param hardwareVersion Version being queried
 *
 * @return Name as string
 *
 * @note return value is a pointer to a STATIC object - do not free
 */
template<uint32_t version>
constexpr const char *getHardwareType() {
   if constexpr(version == HW_LOGIC_BOARD_V2) {
      return "Dig-Logic 2";
   }
   if constexpr(version == HW_LOGIC_BOARD_V3) {
      return "Dig-Logic 3";
   }
   if constexpr(version == HW_LOGIC_BOARD_V4) {
      return "Dig-Logic 4";
   }
   if constexpr(version == HW_SOLDER_STATION_V3) {
      return "Solder Station V3";
   }
   return "Unknown";
}
#else
/**
 * Get hardware type as string
 *
 * @param hardwareVersion Version being queried
 *
 * @return Name as string
 *
 * @note return value is a pointer to a STATIC object - do not free
 */
static inline const char *getHardwareType(uint32_t hardwareVersion) {
   static const char * const names[] = {
         "Unavailable",
         "Digital Lab Board V2",
         "Digital Lab Board V3",
         "Digital Lab Board V4",
         "Soldering Station V3",
   };
   const char *name = "Unknown";
   if (hardwareVersion < (sizeof(names)/sizeof(names[0]))) {
      name = names[hardwareVersion];
   }
   return name;
}
#endif

/**
 * General USB command message
 */
struct UsbCommandMessage {
   UsbCommand  command;                 ///< Command to execute
   uint32_t    startAddress;            ///< Target memory address
   uint32_t    byteLength;              ///< Size of data
   uint8_t     data[MAX_MESSAGE_DATA];  ///< Data (up to 1 flash block)
};

/**
 * Simple USB command message (no data)
 */
struct SimpleCommandMessage {
   UsbCommand  command;       ///< Command to execute
   uint32_t    startAddress;  ///< Target memory address
   uint32_t    byteLength;    ///< Size of data
};

/**
 * General USB response message
 */
struct ResponseMessage {
   UsbCommandStatus   status;        ///< Status
   uint32_t           byteLength;    ///< Size of data
   union {
      struct {
         uint32_t bootHardwareVersion;  ///< Hardware version
         uint32_t bootSoftwareVersion;  ///< Boot-loader software version
         uint32_t flashStart;           ///< Start of flash region
         uint32_t flashSize;            ///< Size of flash region
         uint32_t imageHardwareVersion; ///< Hardware version from loaded image
         uint32_t imageSoftwareVersion; ///< Software version from loaded image
      };
      uint8_t  data[MAX_MESSAGE_DATA];    ///< Data
   };
};

/**
 * USB status response message
 */
struct ResponseStatus {
   UsbCommandStatus   status;        ///< Status
   uint32_t           byteLength;    ///< Size of data
};

/**
 * USB identify response message
 */
struct ResponseIdentify {
   UsbCommandStatus   status;     ///< Status
   uint32_t byteLength;           ///< Size of data (not used)
   uint32_t bootHardwareVersion;  ///< Hardware version
   uint32_t bootSoftwareVersion;  ///< Boot-loader software version
   uint32_t flashStart;           ///< Start of flash region
   uint32_t flashSize;            ///< Size of flash region
   uint32_t imageHardwareVersion; ///< Hardware version from loaded image
   uint32_t imageSoftwareVersion; ///< Software version from loaded image
};

#pragma pack(pop)

#endif /* SOURCES_BOOTLOADERINFO_H_ */
