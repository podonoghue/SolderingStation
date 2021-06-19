/****************************************************************************************************//**
 * @file     formatted_io.h (180.ARM_Peripherals/Project_Headers/formatted_io.h)
 * @brief    Formatted I/O
 *
 * @version  V0.0
 * @date     2015/06
 *
 *******************************************************************************************************/

#ifndef HEADER_FORMATTED_IO_H
#define HEADER_FORMATTED_IO_H
/*
 * *****************************
 * *** DO NOT EDIT THIS FILE ***
 * *****************************
 *
 * This file is generated automatically.
 * Any manual changes will be lost.
 */
#include <stdint.h>
#include <math.h>
//#include <cstdio>       // snprintf()
#include <ctype.h>      // isspace() etc
#include "hardware.h"

#if defined(__FREE_RTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#elif defined(__CMSIS_RTOS)
#include "cmsis.h"
#endif

namespace USBDM {

/**
 * @addtogroup FORMATTED_IO_Group Formatted Input/Output
 * @brief C++ Class allowing input and output of basic types as character streams
 * @{
 */

/**
 * Enumeration selecting radix for integer types with << or >> operators
 */
enum Radix : uint8_t {
   Radix_2       = 2,         //!< Convert as binary number
   Radix_8       = 8,         //!< Convert as octal number
   Radix_10      = 10,        //!< Convert as decimal number
   Radix_16      = 16,        //!< Convert as hexadecimal number
   Radix_Default = Radix_10,  //!< Default radix (10)
};

enum WhiteSpaceType {
   /**
    * With operator<< Discard input white-space characters
    */
   WhiteSpace
};

enum EndOfLineType {
   /**
    * With operator<< Discard input until end-of-line \n
    * With operator>> Write end-of-line
    */
   EndOfLine
};

/**
 * Padding for integers
 */
enum Padding : uint8_t {
   Padding_None ,         //!< No padding
   Padding_LeadingSpaces, //!< Pad with leading spaces
   Padding_LeadingZeroes, //!< Pad with leading zeroes
   Padding_TrailingSpaces,//!< Pad with trailing spaces
};

/**
 * Width for integers
 */
enum Width : uint8_t {
   Width_auto = 0,//!< Width_auto
};

enum EchoMode : uint8_t {
   /*
    * For use with operator<< and operator>>
    */
   EchoMode_Off = false, //!< Turn echo off
   EchoMode_On  = true,  //!< Turn echo on
};

enum FlushType {
   /**
    * With operator<< Discard queued input \n
    * With operator>> Wait until queued data is transmitted.
    */
   Flush
};

struct FormattingSettings {
   /**
    * Precision multiplier used for floating point numbers (10^fFloatPrecision)
    */
   unsigned fFloatPrecisionMultiplier;

   /**
    * Current radix for << and >> operators
    */
   Radix fRadix;

   /**
    * Control echo of input characters
    */
   EchoMode fEcho;

   /**
    * Padding for integers
    */
   Padding fPadding;

   /**
    * Width used for integers numbers
    */
   uint8_t fWidth;

   /**
    * How to pad the digits on left of floating point number
    */
   Padding fFloatPadding;

   /**
    * Precision used for floating point numbers
    */
   uint8_t fFloatWidth;

   /**
    * Precision used for floating point numbers
    */
   uint8_t fFloatPrecision;

   constexpr FormattingSettings() :
      fFloatPrecisionMultiplier(1000), fRadix(Radix_10), fEcho(EchoMode_On), fPadding(Padding_None),
      fWidth(0), fFloatPadding(Padding_None), fFloatWidth(0), fFloatPrecision(3) {
   }
};

/**
 * Virtual Base class for formatted IO
 */
class FormattedIO {

protected:

   /**
    * Current settings
    */
   FormattingSettings fFormat;

   /**
    * Indicate in error state
    */
   bool inErrorState = false;

   /**
    * One character look-ahead
    */
   int16_t lookAhead = -1;

#if defined (__FREE_RTOS) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configUSE_RECURSIVE_MUTEXES == 1 )
   SemaphoreHandle_t mutex;
#elif defined(__CMSIS_RTOS)
   CMSIS::Mutex* mutex;
#endif

   /**
    * Construct formatter interface
    */
   FormattedIO() {
#if defined (__FREE_RTOS) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configUSE_RECURSIVE_MUTEXES == 1 )
      mutex = xSemaphoreCreateRecursiveMutex();
#elif defined(__CMSIS_RTOS)
      mutex = new CMSIS::Mutex();
#endif
   }

   /**
    * Destructor
    */
   virtual ~FormattedIO() {
#if defined (__FREE_RTOS) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configUSE_RECURSIVE_MUTEXES == 1 )
      vSemaphoreDelete(mutex);
#elif defined(__CMSIS_RTOS)
      delete mutex;
#endif
   }

   /**
    * Convert character to digit in given radix
    *
    * @param[in] ch    The character to convert
    * @param[in] radix The radix to use
    *
    * @return >=0 Digit in range 0 - (radix-1)
    * @return <0  Invalid character for radix
    */
   static int convertDigit(int ch, Radix radix) {
      unsigned digit = ch - '0';
      if (digit<10) {
         return (digit<radix)?digit:-1;
      }
      digit = ch-'a'+10;
      if (digit<radix) {
         return digit;
      }
      digit = ch-'A'+10;
      if (digit<radix) {
         return digit;
      }
      return -1;
   }

protected:
   /**
    * Check if character is available
    *
    * @return true  Character available i.e. _readChar() will not block
    * @return false No character available
    */
   virtual bool _isCharAvailable() = 0;

   /**
    * Receives a character (blocking)
    *
    * @return Character received
    */
   virtual int _readChar() = 0;

   /**
    * Writes a character (blocking)
    *
    * @param[in]  ch - character to send
    */
   virtual void _writeChar(char ch) = 0;

public:
   /**
    * Get current settings e.g width, precision etc
    *
    * @param[out] settings Setting object
    */
   FormattedIO &getFormat(FormattingSettings &settings) {
      settings = fFormat;
      return *this;
   }

   /**
    * Set current settings e.g width, precision etc
    *
    * @param[in] settings Setting object
    */
   FormattedIO &setFormat(FormattingSettings &settings) {
      fFormat = settings;
      return *this;
   }

   /**
    * Reset to default formatting.
    * Radix = radix_10, width=0, Padding_None
    *
    * @return Reference to self
    */
   FormattedIO &resetFormat() {
      // Default settings
      static const FormattingSettings defaultSettings;

      fFormat = defaultSettings;
      return *this;
   }

   /**
    *  Flush output data
    */
   virtual void flushOutput() = 0;

   /**
    *  Flush input data
    */
   virtual void flushInput() = 0;

   /**
    * Lock the object
    *
    * @note Requires use of RTOS + Mutexes
    */
   FormattedIO &lock() {
#if defined (__FREE_RTOS) && (configSUPPORT_DYNAMIC_ALLOCATION == 1) && (configUSE_RECURSIVE_MUTEXES == 1)
      xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
#elif defined(__CMSIS_RTOS)
      mutex->wait(osWaitForever);
#endif
      return *this;
   }

   /**
    * Unlock the object
    *
    * @note Requires use of RTOS + Mutexes
    */
   void unlock() {
#if defined (__FREE_RTOS) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configUSE_RECURSIVE_MUTEXES == 1 )
      xSemaphoreGiveRecursive(mutex);
#elif defined(__CMSIS_RTOS)
      mutex->release();
#endif
   }

   /**
    * Peek at lookahead (non-blocking).
    *
    * @return <0   No character available
    * @return >=0  Character available
    */
   int __attribute__((noinline)) peek() {
      if (lookAhead>0) {
         return lookAhead;
      }
      if (!_isCharAvailable()) {
         return -1;
      }
      lookAhead = _readChar();
      if (lookAhead == static_cast<uint8_t>('\r')) {
         lookAhead = '\n';
      }
      if (fFormat.fEcho) {
         _writeChar(lookAhead);
      }
      return lookAhead;
   }

   /**
    * Push a value to the look-ahead buffer
    *
    * @param[in] ch Character to push
    */
   void NOINLINE_DEBUG pushBack(char ch) {
      lookAhead = static_cast<uint8_t>(ch);
   }

   /**
    * Writes a character
    *
    * @param[in]  ch - character to send
    */
   void NOINLINE_DEBUG writeChar(char ch) {
      _writeChar(ch);
   }

   /**
    * Receives a single character
    *
    * @return >0 Character received
    * @return <0 No character available
    */
   int NOINLINE_DEBUG readChar() {
      int ch;
      do {
         ch = peek();
      } while (ch < 0);
      lookAhead = -1;
      return ch;
   }

   /**
    * Set padding for integers
    *
    * @param padding Padding mode
    *
    * @return Reference to self
    */
   FormattedIO &setPadding(Padding padding) {
      fFormat.fPadding = padding;
      return *this;
   }

   /**
    * Set width for integers
    *
    * @param width Width to use
    *
    * @return Reference to self
    */
   FormattedIO &setWidth(unsigned width) {
      fFormat.fWidth = width;
      return *this;
   }

   /**
    * Set precision for floating point numbers
    *
    * @param precision Precision to use
    *
    * @return Reference to self
    */
   /**
    *
    * @param precision Number of digits to the right of decimal point
    * @param padding   How to pad on the left of the number (Padding_LeadingSpaces, Padding_None, Padding_LeadingZeroes)
    * @param width     Number of characters to the left of decimal point
    * @return
    */
   FormattedIO &setFloatFormat( unsigned  precision,
                                Padding   padding  = Padding_None,
                                unsigned  width    = 0) {
      if (padding == Padding_TrailingSpaces) {
         padding = Padding_LeadingSpaces;
      }
      fFormat.fFloatPrecision           = precision;
      fFormat.fFloatPrecisionMultiplier = 1;
      while (precision-->0) {
         fFormat.fFloatPrecisionMultiplier *= 10;
      }
      fFormat.fFloatPadding = padding;
      fFormat.fFloatWidth   = width;
      return *this;
   }

   /**
    * Converts an unsigned long to a string
    *
    * @param[in] ptr        Buffer to write result (at least 32 characters for binary)
    * @param[in] value      Unsigned long to convert
    * @param[in] radix      Radix for conversion [2..16]
    * @param[in] padding    How to pad the number if smaller than field width
    * @param[in] width      Field width of printed number
    * @param[in] isNegative Write leading '-'
    *
    * @return Pointer to '\0' null character at end of converted number\n
    *         May be used for incrementally writing to a buffer.
    */
   static __attribute__((noinline)) char *ultoa(
         char          *ptr,
         unsigned long  value,
         Radix          radix,
         Padding        padding,
         int            width,
         bool           isNegative
         ) {

#ifdef DEBUG_BUILD
      if (ptr == nullptr) {
         __BKPT();
      }
      if ((radix<2)||(radix>16)) {
         __BKPT();
      }
#endif
      // Save beginning for reversal
      char *beginPtr = ptr;
      // Convert backwards
      do {
         *ptr++ = "0123456789ABCDEF"[value % radix];
         value /= radix;
      } while (value != 0);

      // Add leading padding
      switch (padding) {
         case Padding_None:
            if (isNegative) {
                *ptr++ = '-';
             }
             break;
         case Padding_LeadingSpaces:
            if (isNegative) {
               *ptr++ = '-';
            }
            while ((ptr-beginPtr) < width) {
               *ptr++ = ' ';
            }
            break;
         case Padding_LeadingZeroes:
            while ((ptr-beginPtr) < (width-1)) {
               *ptr++ = '0';
            }
            if (isNegative) {
               *ptr++ = '-';
            }
            if ((ptr-beginPtr) < width) {
               *ptr++ = '0';
            }
            break;
         case Padding_TrailingSpaces:
            break;
      }
      // Reverse digits
      char *endPtr = ptr-1;
      char *tPtr   = beginPtr;
      while (tPtr < endPtr) {
         char t = *tPtr;
         *tPtr++ = *endPtr;
         *endPtr-- = t;
      }
      // Add trailing padding
      if (padding==Padding_TrailingSpaces) {
         while ((ptr-beginPtr) < width) {
            *ptr++ = ' ';
         }
      }
      // Terminate and leave ptr at last digit
      *ptr = '\0';
      return ptr;
   }

   /**
    * Converts an unsigned long to a string
    *
    * @param[in] ptr      Buffer to write result (at least 32 characters for binary)
    * @param[in] value    Unsigned long to convert
    * @param[in] radix    Radix for conversion [2..16] (default 10)
    * @param[in] padding  How to pad the number if smaller than field width
    * @param[in] width    Field width of printed number
    *
    * @return Pointer to '\0' null character at end of converted number\n
    *         May be used for incrementally writing to a buffer.
    */
   static NOINLINE_DEBUG char *ultoa(
         char *ptr,
         unsigned long value,
         Radix radix=Radix_10,
         Padding padding=Padding_None,
         int width=0
         ) {
      return ultoa(ptr, value, radix, padding, width, false);
}

   /**
    * Converts a long to a string
    *
    * @param[in] ptr      Buffer to write result (at least 32 characters for binary)
    * @param[in] value    Long to convert
    * @param[in] radix    Radix for conversion [2..16] (default 10)
    * @param[in] padding  How to pad the number if smaller than field width
    * @param[in] width    Field width of printed number
    *
    * @return Pointer to '\0' null character at end of converted number\n
    *         May be used for incrementally writing to a buffer.
    */
   static NOINLINE_DEBUG char *ltoa(
         char *ptr,
         long value,
         Radix radix=Radix_10,
         Padding padding=Padding_None,
         int width=0
         ) {
      bool isNegative = value<0;
      if (isNegative) {
         value = -value;
      }
      return ultoa(ptr, value, radix, padding, width, isNegative);
   }

   /**
    * Copies a C string including terminating '\0' character
    *
    * @param[out] dst  Where to copy string
    * @param[in]  src  Source to copy from
    *
    * @return Pointer to '\0' null character at end of concatenated string.\n
    *         May be used for incrementally writing to a buffer.
    */
   static NOINLINE_DEBUG char *strcpy(char *dst, const char *src) {
#ifdef DEBUG_BUILD
      if (dst == nullptr) {
         __BKPT();
      }
#endif
      do {
         *dst++ = *src;
      } while (*src++ != '\0');
      return dst-1;
   }

   /**
    * Write data
    *
    * @param[in]  data     Data to transmit
    * @param[in]  size     Size of transmission data
    */
   void NOINLINE_DEBUG transmit(const uint8_t data[], uint16_t size) {
      while (size-->0) {
         writeChar(*data++);
      }
   }

   /**
    * Receive data
    *
    * @param[out] data     Data buffer for reception
    * @param[in]  size     Size of data to receive
    */
   void NOINLINE_DEBUG receive(uint8_t data[], uint16_t size) {
      while (size-->0) {
         *data++ = readChar();
      }
   }

   /**
    * Receive string until terminator character or buffer full.\n
    * The terminating character is discarded and the string always '\0' terminated
    *
    * @param[out] data       Data buffer for reception
    * @param[in]  size       Size of data buffer (including space for '\0')
    * @param[in]  terminator Terminating character
    *
    * @return number of characters read (excluding terminator)
    *
    * @note Excess characters are discarded once the buffer is full.
    *
    * Usage
    * @code
    *    char buff[100];
    *    int numChars = gets(buff, sizeof(buff));
    * @endcode
    */
   int __attribute__((noinline)) gets(char data[], uint16_t size, char terminator='\n') {
      char *ptr = data;

      char ch;
      do {
         ch = readChar();
         if (ptr<(data+size)) {
            *ptr++ = ch;
         }
      } while(ch != terminator);
      *--ptr = '\0';
      return ptr-data;
   }

   /**
    * Write a character
    *
    * @param[in]  ch - character to send
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &write(char ch) {
      writeChar(ch);
      return *this;
   }

   /**
    * Null function (for debug)
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &null() {
      return *this;
   }

   /**
    * Write an end-of-line
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &writeln() {
      return write('\n');
   }

   /**
    * Write a character with newline
    *
    * @param[in]  ch - character to send
    *
    * @return Reference to self
    */
   FormattedIO  NOINLINE_DEBUG &writeln(char ch) {
      write(ch);
      return writeln();
   }

   /**
    * Write a C string
    *
    * @param[in]  str   String to print
    * @param[in]  width Width of string (either truncated or padded to this width)
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &write(const char *str, unsigned width) {
      while ((*str != '\0') && (width-->0)) {
         write(*str++);
      }
      while (width-->0) {
         write(' ');
      }
      return *this;
   }

   /**
    * Write a C string
    *
    * @param[in]  str   String to print
    * @param[in]  width Width of string (either truncated or padded to this width)
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &writeln(const char *str, unsigned width) {
      write(str, width);
      return writeln();
   }

   /**
    * Write a C string
    *
    * @param[in]  str String to print
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &write(const char *str) {
      while (*str != '\0') {
         write(*str++);
      }
      return *this;
   }

   /**
    * Write a C string with new line
    *
    * @param[in]  str String to print
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &writeln(const char *str) {
      write(str);
      return writeln();
   }

   /**
    * Write a boolean value
    *
    * @param[in]  b Boolean to print
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &write(bool b) {
      return write(b?"true":"false");
   }

   /**
    * Write a boolean value with new line
    *
    * @param[in]  b Boolean to print
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &writeln(bool b) {
      write(b);
      return writeln();
   }

   /**
    * Write an unsigned long integer
    *
    * @param[in]  value Unsigned long to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &write(unsigned long value, Radix radix=Radix_10) {
      char buff[35];
      ultoa(buff, value, radix, fFormat.fPadding, fFormat.fWidth, false);
      return write(buff);
   }

   /**
    * Write a long integer
    *
    * @param[in]  value Long to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &write(long value, Radix radix=Radix_10) {
      char buff[35];
      bool isNegative = value < 0;
      if (isNegative) {
         value = -value;
      }
      ultoa(buff, static_cast<unsigned long>(value), radix, fFormat.fPadding, fFormat.fWidth, isNegative);
      return write(buff);
   }

   /**
    * Write an unsigned long integer with newline
    *
    * @param[in]  value Unsigned long to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &writeln(unsigned long value, Radix radix=Radix_10) {
      write(value, radix);
      return writeln();
   }

   /**
    * Write an pointer value
    *
    * @param[in]  value Pointer value to print
    * @param[in]  radix Radix for conversion [2..16] (default 16)
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &write(const void *value, Radix radix=Radix_16) {
      return write(reinterpret_cast<unsigned long>(value), radix);
   }

   /**
    * Write an pointer value with newline
    *
    * @param[in]  value Pointer value to print
    * @param[in]  radix Radix for conversion [2..16] (default 16)
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &writeln(const void *value, Radix radix=Radix_16) {
      return writeln(reinterpret_cast<unsigned long>(value), radix);
   }

   /**
    * Write a long integer with newline
    *
    * @param[in]  value Long to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &writeln(long value, Radix radix=Radix_10) {
      write(value, radix);
      return writeln();
   }

   /**
    * Write an unsigned integer
    *
    * @param[in]  value Unsigned to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &write(unsigned value, Radix radix=Radix_10) {
      return write(static_cast<unsigned long>(value), radix);
   }

   /**
    * Write an unsigned integer with newline
    *
    * @param[in]  value Unsigned to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &writeln(unsigned value, Radix radix=Radix_10) {
      return writeln(static_cast<unsigned long>(value), radix);
   }

   /**
    * Write an integer
    *
    * @param[in]  value Integer to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &write(int value, Radix radix=Radix_10) {
      return write(static_cast<long>(value), radix);
   }

   /**
    * Write an integer with newline
    *
    * @param[in]  value Integer to print
    * @param[in]  radix Radix for conversion [2..16] (default 10)
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG  &writeln(int value, Radix radix=Radix_10) {
      return writeln(static_cast<long>(value), radix);
   }

#if 0
   /**
    * Write a double
    *
    * @param[in]  value Double to print
    *
    * @return Reference to self
    *
    * @note Uses snprintf() which is large.
    * @note To use this function it is necessary to enable floating point printing\n
    *       in the linker options (Support %f format in printf -u _print_float)).
    */
   FormattedIO NOINLINE_DEBUG &write(double value) {
      char buff[20];
      snprintf(buff, sizeof(buff), "%f", value);
      return write(buff);
   }
#else
   /**
    * Write a double - Limited to 3 decimal places
    *
    * @param[in]  value Double to print
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &write(double value) {
      char buff[20];
      bool isNegative = value<0;
      if (isNegative) {
         value = -value;
      }
      long scaledValue = static_cast<long>(round(value*fFormat.fFloatPrecisionMultiplier));
      ultoa(buff, scaledValue/fFormat.fFloatPrecisionMultiplier, Radix_10, fFormat.fFloatPadding, fFormat.fFloatWidth, isNegative);
      write(buff).write('.');
      ultoa(buff, 
           (scaledValue)%fFormat.fFloatPrecisionMultiplier,
           Radix_10, Padding_LeadingZeroes, fFormat.fFloatPrecision);
      write(buff);
      return *this;
   }
#endif
   /**
    * Write a double with newline
    *
    * @param[in]  value Double to print
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &writeln(double value) {
      write(value);
      return writeln();
   }

   /**
    * Write a float
    *
    * @param[in]  value Float to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &write(float value) {
      return write(static_cast<double>(value));
   }

   /**
    * Write a float with newline
    *
    * @param[in]  value Float to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &writeln(float value) {
      return writeln(static_cast<double>(value));
   }

   /**
    * Write a character
    *
    * @param[in]  ch Character to print
    *
    * @return Reference to self
     */
   FormattedIO NOINLINE_DEBUG &operator <<(char ch) {
      return write(ch);
   }

   /**
    * Write a boolean value
    *
    * @param[in]  b Boolean to print
    *
    * @return Reference to self
     */
   FormattedIO NOINLINE_DEBUG &operator <<(bool b) {
      return write(b);
   }

   /**
    * Write a C string
    *
    * @param[in]  str String to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(const char *str) {
      return write(str);
   }

   /**
    * Write an unsigned long integer
    *
    * @param[in]  value Unsigned long to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(unsigned long value) {
      return write(value, fFormat.fRadix);
   }

   /**
    * Write a long integer
    *
    * @param[in]  value Long to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(long value) {
      return write(value, fFormat.fRadix);
   }

   /**
    * Write an unsigned integer
    *
    * @param[in]  value Unsigned to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(unsigned int value) {
      return write(value, fFormat.fRadix);
   }

   /**
    * Write an integer
    *
    * @param[in]  value Integer to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(int value) {
      return write(value, fFormat.fRadix);
   }

   /**
    * Write a pointer value
    *
    * @param[in]  value Pointer value to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(const void *value) {
      return write(reinterpret_cast<unsigned long>(value), fFormat.fRadix);
   }

   /**
    * Write a float
    *
    * @param[in]  value Float to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(float value) {
      return write(static_cast<double>(value));
   }

   /**
    * Write a double
    *
    * @param[in]  value Double to print
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(double value) {
      return write(value);
   }

   /**
    * Sets the conversion radix for integer types
    *
    * @param[in] radix Radix to set
    *
    * @return Reference to self
    *
    * @note Only applies for operator<< methods
    */
   FormattedIO NOINLINE_DEBUG &operator <<(Radix radix) {
      fFormat.fRadix = radix;
      return *this;
   }

   /**
    * Write end-of-line
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(EndOfLineType) {
      return writeln();
   }

   /**
    * Enable/Disable echoing of input characters
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(EchoMode echoMode) {
      return setEcho(echoMode);
   }

   /**
    * Enable/Disable echoing of input characters
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator >>(EchoMode echoMode) {
      return setEcho(echoMode);
   }

   /**
    * Flush output data
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator <<(FlushType) {
      flushOutput();
      return *this;
   }

   /**
    * Discard white-space from the input
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &skipWhiteSpace() {
      int ch;
      do {
         ch = readChar();
      } while (isspace(ch));
      pushBack(ch);
      return *this;
   }

   /**
    * Discard input until end-of-line (inclusive)
    *
    * @return Reference to self
    */
   FormattedIO __attribute__((noinline)) &readln() {
      while (readChar() != '\n') {
         __asm__("nop");
      }
      return *this;
   }

   /**
    * Read a character from the input
    *
    * @param[out] ch Where to place character read
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &read(char &ch) {
      ch = readChar();
      return *this;
   }

   /**
    * Get and clear error state
    *
    * @return false No error
    * @return true  Operation failed since last checked e.g. illegal digit at start of number
    */
   bool __attribute__((noinline)) isError() {
      bool t = inErrorState;
      inErrorState = false;
      return t;
   }

   /**
    * Receives an unsigned long
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO __attribute__((noinline)) &read(unsigned long &value, Radix radix=Radix_10) {
      // Skip white space
      int ch;
      do {
         ch = readChar();
      } while (isspace(ch));

      // Check if sign character
      bool negative = (ch == '-');
      if (negative) {
         // Discard  '-'
         ch = readChar();
      }

      // Parse number
      value = 0;
      int digitCount = 0;
      do {
         int digit = convertDigit(ch, radix);
         if (digit<0) {
            break;
         }
         digitCount++;
         value *= radix;
         value += digit;
         ch = readChar();
      } while (true);

      // Must have at least 1 digit
      inErrorState = (digitCount<=0);

      // Push back 1st non-digit
      pushBack(ch);
      if (negative) {
         value = -value;
      }
      return *this;
   }

   /**
    * Controls echoing of input characters
    *
    * @param echoMode
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &setEcho(EchoMode echoMode=EchoMode_On) {
      fFormat.fEcho = echoMode;
      return *this;
   }
   /**
    * Receives an unsigned long and then discards characters until end of line.
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &readln(unsigned long &value, Radix radix=Radix_10) {
      read(value, radix);
      return readln();
   }

   /**
    * Receives a long
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &read(long &value, Radix radix=Radix_10) {
      unsigned long temp;
      read(temp, radix);
      value = temp;
      return *this;
   }

   /**
    * Receives a long and then discards characters until end of line.
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &readln(long &value, Radix radix=Radix_10) {
      read(value, radix);
      return readln();
   }

   /**
    * Receives an unsigned integer
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &read(unsigned int &value, Radix radix=Radix_10) {
      unsigned long temp;
      read(temp, radix);
      value = temp;
      return *this;
   }

   /**
    * Receives an unsigned integer and then discards characters until end of line.
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &readln(unsigned &value, Radix radix=Radix_10) {
      read(value, radix);
      return readln();
   }

   /**
    * Receives an integer
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &read(int &value, Radix radix=Radix_10) {
      long temp;
      read(temp, radix);
      value = temp;
      return *this;
   }

   /**
    * Receives an integer and then discards characters until end of line.
    *
    * @param[out] value Where to place value read
    * @param[in]  radix The radix to use
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &readln(int &value, Radix radix=Radix_10) {
      read(value, radix);
      return readln();
   }

   /**
    * Discard white-space from the input
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator >>(WhiteSpaceType) {
      return skipWhiteSpace();
   }

   /**
    * Discard input until end-of-line
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator >>(EndOfLineType) {
      while (readChar() != '\n') {
         __asm__("nop");
      }
      return *this;
   }

   /**
    * Flush input data
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator >>(FlushType) {
      flushInput();
      return *this;
   }

   /**
    * Sets the conversion radix for integer types
    *
    * @param[in]  radix Radix to set
    *
    * @return Reference to self
    *
    * @note Only applies for operator<< methods
    */
   FormattedIO NOINLINE_DEBUG &operator >>(Radix radix) {
      fFormat.fRadix = radix;
      return *this;
   }

   /**
    * Receives a single character
    *
    * @param[out] ch Where to place character read
    *
    * @return Reference to self
    */
   FormattedIO NOINLINE_DEBUG &operator >>(char &ch) {
      ch = readChar();
      return *this;
   }

   /**
    * Receives an unsigned long
    *
    * @param[out] value Where to place value read
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &operator >>(unsigned long &value) {
      return read(value, fFormat.fRadix);
   }

   /**
    * Receives a long
    *
    * @param[out] value Where to place value read
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &operator >>(long &value) {
      return read(value, fFormat.fRadix);
   }

   /**
    * Receives an unsigned long
    *
    * @param[out] value Where to place value read
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &operator >>(unsigned int &value) {
      return read(value, fFormat.fRadix);
   }

   /**
    * Receives an integer
    *
    * @param[out] value Where to place value read
    *
    * @return Reference to self
    *
    * @note Skips leading whitespace
    */
   FormattedIO NOINLINE_DEBUG &operator >>(int &value) {
      return read(value, fFormat.fRadix);
   }

   /**
    * Get conversion radix for given base
    *
    * @param[in]  radix Base to convert to radix [2..16]
    *
    * @return Radix corresponding to base
    */
   static constexpr Radix NOINLINE_DEBUG radix(unsigned radix) {
      return static_cast<Radix>(radix);
   }

   /**
    * Get field width
    *
    * @param[in]  width Integer to convert to width
    *
    * @return Width corresponding to width
    */
   static constexpr Width NOINLINE_DEBUG width(int width) {
      return static_cast<Width>(width);
   }

   /**
    *
    * @param[in] width
    *
    * @return
    */
   FormattedIO &operator<<(Width width) {
      setWidth(width);
      return *this;
   }

   /**
    *
    * @param[in] padding
    *
    * @return
    */
   FormattedIO &operator<<(Padding padding) {
      setPadding(padding);
      return *this;
   }

   /**
    * Print an array as a hex table.
    * The indexes shown are for byte offsets suitable for a memory dump.
    *
    * @param data          Array to print
    * @param size          Size of array in elements
    * @param visibleIndex The starting index to print for the array. Should be multiple of sizeof(data[]).
    */
   template <typename T>
   void writeArray(T *data, uint32_t size, uint32_t visibleIndex=0) {
      usbdm_assert((visibleIndex%sizeof(T))==0, "visibleIndex should be multiple of sizeof(data[])");
      unsigned rowMask;
      unsigned offset;

      switch(sizeof(T)) {
         case 1  :
            offset = (visibleIndex/sizeof(T))&0xF;
            visibleIndex &= ~0xF;
            rowMask = 0xF;  break;
         case 2  :
            offset = (visibleIndex/sizeof(T))&0x7;
            visibleIndex &= ~0xF;
            rowMask = 0x7; break;
         default :
            offset = (visibleIndex/sizeof(T))&0x7;
            visibleIndex &= ~0x1F;
            rowMask = 0x7; break;
      }
      setPadding(Padding_TrailingSpaces).setWidth(2*sizeof(T));
      write("          ");
      for (unsigned index=0; index<=(rowMask*sizeof(T)); index+=sizeof(T)) {
         write(index, Radix_16).write(" ");
      }
      writeln();
      setPadding(Padding_LeadingZeroes);
      bool needNewline = true;
      size += offset;
      for (unsigned index=0; index<size; index++) {
         if (needNewline) {
            setWidth(8);
            write(visibleIndex+index*sizeof(T), Radix_16).write(": ");
         }
         if (index<offset) {
            switch(sizeof(T)) {
               case 1  : write("   ");       break;
               case 2  : write("     ");     break;
               default : write("         "); break;
            }
         }
         else {
            setWidth(2*sizeof(T));
            write(data[index-offset], Radix_16).write(" ");
         }
         needNewline = (((index+1)&rowMask)==0);
         if (needNewline) {
            writeln();
         }
      }
      writeln().resetFormat();
   }

};

/**
 * End FORMATTED_IO_Group
 * @}
 */

} // End namespace USBDM

#endif /* HEADER_FORMATTED_IO_H */
