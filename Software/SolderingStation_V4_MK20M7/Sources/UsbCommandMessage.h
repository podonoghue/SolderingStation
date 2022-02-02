/*
 * UsbCommandMessage.h
 *
 *  Created on: 9 Dec 2019
 *      Author: podonoghue
 */

#ifndef SOURCES_USBCOMMANDMESSAGE_H_
#define SOURCES_USBCOMMANDMESSAGE_H_

#include <stdint.h>

// USB messages are packed data in LE (native) format
#pragma pack(push, 1)

/**
 * Commands available
 */
enum UsbCommand : uint32_t {
   UsbCommand_Nop,               ///< No operation (also sets OK status on LEDs)
   UsbCommand_Identify,          ///< Read IDCODE
   UsbCommand_CheckVref,         ///< Check for target Vref
   UsbCommand_XSVF,              ///< Start XSVF download
   UsbCommand_XSVF_data,         ///< Transfer XSVF block
   UsbCommand_XSVF_execute,      ///< Execute short XSVF block
   UsbCommand_Status_Leds,       ///< Sets green and red LEDs
   UsbCommand_SetFailed = 200,   ///< Dummy command to force failed status on LEDs
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
   static const char *names[] = {
         "UsbCommand_Nop",
         "UsbCommand_Identify",
         "UsbCommand_CheckVref",
         "UsbCommand_XSVF",
         "UsbCommand_XSVF_data",
         "UsbCommand_XSVF_execute",
   };
   const char *name = "Unknown";
   if (command < (sizeof(names)/sizeof(names[0]))) {
      name = names[command];
   }
   return name;
}

/**
 * Simple USB command message
 */
struct SimpleCommandMessage {
   UsbCommandStatus   status;        ///< Status
   uint32_t           byteLength;    ///< Size of data
};

/**
 * XSVF block USB command message
 */
struct XsvfBlockCommandMessage {
   UsbCommand         command;       ///< Status
   uint32_t           byteLength;    ///< Size of data
   uint8_t            data[MAX_MESSAGE_DATA];    ///< Data
};

/**
 * General USB command message
 */
struct UsbCommandMessage {
   UsbCommand  command;       ///< Command to execute
   uint32_t    byteLength;    ///< Size of data
   union {
      struct {
         uint32_t xsvfSize;
      };
      struct {
         bool     passLed;       ///< Green/Pass LED
         bool     failLed;       ///< Red/Fail/Busy LED
      };
      uint8_t     data[MAX_MESSAGE_DATA];    ///< Data
   };
};

/**
 * General USB response message
 */
struct ResponseMessage {
   UsbCommandStatus   status;        ///< Status
   uint32_t           byteLength;    ///< Size of data
   union {
      struct {
         uint32_t idcode;
      };
      struct {
         uint32_t result;
      };
   };
};

/**
 * General USB response message
 */
struct SimpleResponseMessage {
   UsbCommandStatus   status;        ///< Status
   uint32_t           byteLength;    ///< Size of data
};

//===================================================

/**
 * Identify command message
 */
struct UsbCommandIdentifyMessage {
   UsbCommand  command;       ///< Command to execute
   uint32_t    byteLength;    ///< Size of data
};

/**
 * Identify response message
 */
struct ResponseIdentifyMessage {
   UsbCommandStatus   status;        ///< Status
   uint32_t           byteLength;    ///< Size of data
   uint32_t           idcode;        ///< IDCODE from target
};

//===================================================

/**
 * Start XSVF transfer message
 */
struct UsbStartXsvfMessage {
   UsbCommand  command;       ///< Command to execute
   uint32_t    byteLength;    ///< Size of data
   uint32_t    xsvfSize;
};

//===================================================

/**
 * Send XSVF block message
 */
struct UsbSendXsvfBlockMessage {
   UsbCommand  command;                ///< Command to execute
   uint32_t    byteLength;             ///< Size of data
   uint8_t     data[MAX_MESSAGE_DATA]; ///< Data
};

/**
 * Send XSVF block message
 */
struct UsbSetSatusMessage {
   UsbCommand  command;       ///< Command to execute
   uint32_t    byteLength;    ///< Size of data
   bool        okLed;         ///< Green/OK LED
   bool        failLed;       ///< Red/Fail/Busy LED
};

//===================================================

#pragma pack(pop)

#endif /* SOURCES_USBCOMMANDMESSAGE_H_ */
