/** \file
    \brief Header file for FlashImage.cpp

    \verbatim
    Copyright (C) 2015  Peter O'Donoghue

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Change History
   +====================================================================
   |    May 2015 | Created
   +====================================================================
    \endverbatim
 */
#ifndef  FLASHIMAGE_H_
#define  FLASHIMAGE_H_

#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <map>
#include "USBDM_API.h"
#include "UsbdmSystem.h"
#include "Elf.h"
#include "FlashImage.h"

class  MemoryPage;
class  EnumeratorImp;
typedef std::shared_ptr<MemoryPage> MemoryPagePtr;
/**
 * Represents a memory image containing loaded file(s)
 */
class FlashImage {

   /**
    *  Class to enumerate the occupied locations within the memory image
    *
    *  @note may be invalidated by changes to the referenced image
    */
   class Enumerator {
   protected:
      /**
       *  Construct an enumerator positioned at a given starting address
       *  Note: if address is unallocated then the Enumerator is advanced to next allocated address
       */
      Enumerator() {};

   public:
      virtual ~Enumerator() {};

      /**
       * Get the current location as a flat address
       */
      virtual uint32_t   getAddress() const = 0;
      /**
       * Indicates if the current memory location is valid (occupied)
       *
       * @return \n
       *         true  => current location is occupied
       *        false => current location is unoccupied/unallocated
       */
      virtual bool       isValid() const = 0;
      /**
       * Sets the iterator to the given address
       *
       *  @return \n
       *         true  => current location is occupied
       *         false => current location is unoccupied/unallocated
       */
      virtual bool       setAddress(uint32_t addr) = 0;
      /**
       * Advance to next occupied flash location
       *
       * @return \n
       *        true  => advanced to next occupied location
       *        false => no occupied locations remain, enumerator is left at last \e unoccupied location
       */
      virtual bool       nextValid() = 0;
      /**
       *  Advance location to just before the next unoccupied flash location or page boundary
       *  Assumes current location is occupied.
       */
      virtual void       lastValid() = 0;
   };

   template <typename T> class MallocWrapper {
   private:
      T *&ptr;
   public:
      static const uint32_t DataOffset    =  (0x02000000UL);  // Offset used for DSC Data region

      typedef std::shared_ptr<Enumerator> EnumeratorPtr;

      MallocWrapper(T *&ptr) : ptr(ptr) {
      }

      T *alloc(size_t size) {
         free();
         ptr = (T*)malloc(size);
         UsbdmSystem::Log::print("Allocated %ld @0x%p\n", (long)size, ptr);
         return ptr;
      }
      void free() {
         if (ptr != 0) {
            UsbdmSystem::Log::print("Freeing @0x%p\n", ptr);
            ::free(ptr);
            ptr = 0;
         }
      }
      ~MallocWrapper() {
         free();
      }
   };

   class Openfile {
   private:
      FILE *fp;
   public:
      Openfile(const char* filePath, const char* modes) {
         fp = fopen(filePath, modes);
         UsbdmSystem::Log::print("Opened %s (fp=%p)\n", filePath, fp);
      }
      FILE *getfp() {
         return  fp;
      }
      ~Openfile() {
         //         UsbdmSystem::Log::print("Closing file (fp=%p)\n", fp);
         if (fp != 0) {
            fclose(fp);
         }
      }
   };

   friend MemoryPage;
   friend EnumeratorImp;

protected:
   static const int                  PAGE_BIT_OFFSET =  (15-sizeof(uint8_t));  // 2**14 = 16K pages
   static const unsigned             PAGE_SIZE       =  (1U<<PAGE_BIT_OFFSET);
   static const int                  PAGE_MASK       =  (PAGE_SIZE-1U);
   static const int                  MAX_SREC_SIZE   =  (1<<4);//! Maximum size of a S-record (2^N)

protected:
   TargetType_t                      targetType;
   bool                              wordAddresses;
   std::map<uint32_t,MemoryPagePtr>  memoryPages;            //!< Pointers to occupied memory pages
   unsigned                          firstAllocatedAddress;  //!< First used memory locations
   unsigned                          lastAllocatedAddress;   //!< Last used memory locations
   uint16_t                          lastPageNumAccessed;    //!< Page # of last page accessed
   MemoryPagePtr                     lastMemoryPageAccessed; //!< Last page accessed
   unsigned                          elementCount;           //!< Count of occupied bytes
   bool                              littleEndian;           //!< Target is little-endian
   std::string                       sourceFilename;         //!< Name of last file loaded
   std::string                       sourcePath;             //!< Path of last file loaded
   bool                              allowOverwrite;
   FILE                             *fp;
   bool                              discardFF;
   bool                              printHeader;
   Elf32_Ehdr                        elfHeader;
   Elf32_Shdr                        stringSectionHeader;
   Elf32_Phdr                       *programHeaders;
   char                             *symTable;

public:
   /**
    *   Constructor - creates an empty Flash image
    */
   FlashImage();
   /*
    * Destructor
    */
   virtual ~FlashImage();

   /**
    *   Set target type of image
    *
    *   @param targetType - Target type to set
    */
   virtual void                  setTargetType(TargetType_t targetType);
   /**
    * Get string describing the error code
    *
    * @param rc - Error code
    *
    * @return String describing the error
    */
   virtual char const           *getErrorString(USBDM_ErrorCode rc);
   /**
    *  Load a S19 or ELF file into the buffer. \n
    *
    *  @param filePath      : Path of file to load
    *  @param clearBuffer   : Clear buffer before loading
    *
    *  @return error code see \ref USBDM_ErrorCode
    */
   virtual USBDM_ErrorCode       loadFile(const std::string &filePath, bool clearBuffer=true);
   /**
    *  Save image buffer as a S19 file. \n
    *
    *  @param filePath      : Path of file to load
    *  @param discardFF     : Discard sizable blocks of consecutive 0xFF values (assumed blank)
    *
    *  @return error code see \ref USBDM_ErrorCode
    */
   virtual USBDM_ErrorCode       saveFile(const std::string &filePath, bool discardFF=true);
   /**
    *  Initialises the memory to empty
    */
   virtual void                  clear();
   /**
    * Check if image is entirely empty (never written to)
    *
    * @return true=>image is entirely empty,\n
    *               image is not empty
    */
   virtual bool                  isEmpty() const;
   /**
    *  Checks if the memory location is valid (has been written to)
    *
    *  @param address - 32-bit memory address
    *
    *  @return\n
    *          true   => location has been previously written to \n
    *          false  => location is invalid
    */
   virtual bool                  isValid(uint32_t address);
   /**
    *  Returns an approximate count of occupied bytes
    */
   virtual unsigned              getByteCount() const;
   /**
    *  Obtain the value of a Flash memory location
    *
    *  @param address - 32-bit memory address
    *
    *  @return - uint8_t value (a dummy value of 0xFF is quietly returned for unallocated locations)
    */
   virtual uint8_t               getValue(uint32_t address);
   /**
    *   Set a Flash memory location
    *
    *   @param address - 32-bit memory address
    *   @param value   - uint8_t value to write to image
    *
    *   @note Allocates a memory location if necessary
    */
   virtual void                  setValue(uint32_t address, uint8_t value);
   /**
    * Remove a Flash memory location (set to unprogrammed)
    *
    * @param address - 32-bit memory address
    */
   virtual void                  remove(uint32_t address);

   typedef std::shared_ptr<FlashImage::Enumerator> EnumeratorPtr;

   /**
    *   Gets an enumerator for the memory
    *   The enumerator is positioned at the 1st valid location after the given address
    */
   virtual EnumeratorPtr         getEnumerator(uint32_t address = 0);
   /**
    *  Prints a summary of the Flash memory allocated/used.
    */
   virtual void                  printMemoryMap();
   /**
    *  Get pathname of last file loaded
    */
   virtual const std::string    &getSourcePathname() const;
   /**
    *  Dump the contents of a range of memory to log file
    *
    * @param startAddress - start of range
    * @param endAddress   - end of range
    *
    */
   virtual void                  dumpRange(uint32_t startAddress, uint32_t endAddress);
   /**
    * Load data into Flash image
    *
    * @param bufferSize    - size of data to load (in uint8_t)
    * @param address       - address to load at
    * @param data          - data to load
    * @param dontOverwrite - produce error if overwriting existing data
    */
   virtual USBDM_ErrorCode       loadData(uint32_t bufferSize, uint32_t address, const uint8_t  data[], bool dontOverwrite = false);
   /**
    * Load data into Flash image from byte array
    *
    * @param bufferSize    - size of data to load (in bytes)
    * @param address       - address to load at (byte/word address)
    * @param data          - data to load
    * @param dontOverwrite - true to prevent overwriting data
    *
    * @note This is only of use if uint8_t is not a byte
    */
   virtual USBDM_ErrorCode       loadDataBytes(uint32_t bufferSize, uint32_t address, const uint8_t data[], bool dontOverwrite = false);
   /**
    * Get first allocated address
    *
    * @return - first allocated address
    */
   virtual unsigned              getFirstAllocatedAddress() { return firstAllocatedAddress; }

   /**
    * Get last allocated address
    *
    * @return - last allocated address
    */
   virtual unsigned              getLastAllocatedAddress()  { return lastAllocatedAddress; }

   /**
    *  Fills a range of memory with a value
    *
    *  @param fillValue - value to use for fill
    *  @param size      - size of range to fill
    *  @param address   -  start address of range
    */
   virtual void                  fill(uint32_t size, uint32_t address, uint8_t fillValue = 0xFF);
   /**
    *  Fills unused bytes within a range of memory with a value
    *
    *  @param fillValue - value to use for fill
    *  @param size      - size of range to fill
    *  @param address   -  start address of range
    */
   virtual void                  fillUnused(uint32_t size, uint32_t address, uint8_t fillValue = 0xFF);

protected:
   virtual MemoryPagePtr   getmemoryPage(uint32_t pageNum);
   virtual MemoryPagePtr   allocatePage(uint32_t pageNum);
   uint32_t                targetToNative(uint32_t &);
   uint16_t                targetToNative(uint16_t &);
   int32_t                 targetToNative(int32_t &);
   int16_t                 targetToNative(int16_t &);

   void                    fixElfHeaderSex(Elf32_Ehdr *elfHeader);
   void                    printElfHeader(Elf32_Ehdr *elfHeader);
   void                    printElfProgramHeader(Elf32_Phdr *programHeader, uint32_t loadAddress=-1);
   void                    printElfSectionHeader(Elf32_Shdr *programHeader);
   const char *            getElfString(unsigned index);

   void                    fixElfProgramHeaderSex(Elf32_Phdr *programHeader);
   void                    fixElfSectionHeaderSex(Elf32_Shdr *elfsHeader);
   USBDM_ErrorCode         loadElfBlock(FILE *fp, long fOffset, Elf32_Word size, Elf32_Addr addr);
   USBDM_ErrorCode         loadElfBlockByProgramHeader(Elf32_Phdr *programHeader);
   Elf32_Phdr              *findRelatedProgramHeader(Elf32_Shdr *sectionHeader);

   USBDM_ErrorCode         loadElfBlockBySectionHeader(Elf32_Shdr *sectionHeader);
   USBDM_ErrorCode         recordElfProgramBlock(Elf32_Phdr *programHeader);
   USBDM_ErrorCode         loadElfFile(const std::string &fileName);
   USBDM_ErrorCode         checkTargetType(Elf32_Half e_machine, TargetType_t targetType);
   USBDM_ErrorCode         loadS1S9File(const std::string &fileName);
   USBDM_ErrorCode         loadAbsoluteFile(const std::string &fileName);

   static void             addressToPageOffset(uint32_t address, uint16_t &pageNum, uint16_t &offset);
   static uint32_t         pageOffsetToAddress(uint16_t pageNum, uint16_t offset);
   void                    writeSrec(uint8_t *buffer, uint32_t address, unsigned size);
   void                    writeData(uint8_t *buffer, uint32_t address, unsigned size);
   Elf32_Addr              getLoadAddress(Elf32_Shdr *sectionHeader);
};

typedef std::shared_ptr<FlashImage> FlashImagePtr;

#endif // FLASHIMAGE_H_
