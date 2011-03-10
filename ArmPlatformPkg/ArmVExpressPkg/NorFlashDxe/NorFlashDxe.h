/** @file  NorFlashDxe.h

  Copyright (c) 2010, ARM Ltd. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __NOR_FLASH_DXE_H__
#define __NOR_FLASH_DXE_H__


#include <Base.h>
#include <PiDxe.h>

#include <Protocol/BlockIo.h>
#include <Protocol/FirmwareVolumeBlock.h>

#include <ArmPlatform.h>

#define HIGH_16_BITS                              0xFFFF0000
#define LOW_16_BITS                               0x0000FFFF
#define LOW_8_BITS                                0x000000FF

// Hardware addresses

#define VE_SYSTEM_REGISTERS_OFFSET                0x00000000
#define SYSTEM_REGISTER_SYS_FLASH                 0x0000004C

#define VE_REGISTER_SYS_FLASH_ADDR                ( ARM_VE_BOARD_PERIPH_BASE + VE_SYSTEM_REGISTERS_OFFSET + SYSTEM_REGISTER_SYS_FLASH )

// Device access macros
// These are necessary because we use 2 x 16bit parts to make up 32bit data

#define FOLD_32BIT_INTO_16BIT(value)              ( ( value >> 16 ) | ( value & LOW_16_BITS ) )

#define GET_LOW_BYTE(value)                       ( value & LOW_8_BITS )
#define GET_HIGH_BYTE(value)                      ( GET_LOW_BYTE( value >> 16 ) )

// Each command must be sent simultaneously to both chips,
// i.e. at the lower 16 bits AND at the higher 16 bits
#define CREATE_NOR_ADDRESS(BaseAddr,OffsetAddr)   ( (volatile UINTN *)((BaseAddr) + ((OffsetAddr) << 2)) )
#define CREATE_DUAL_CMD(Cmd)                      ( ( Cmd << 16) | ( Cmd & LOW_16_BITS) )
#define SEND_NOR_COMMAND(BaseAddr,OffsetAddr,Cmd) ( *CREATE_NOR_ADDRESS(BaseAddr,OffsetAddr) = CREATE_DUAL_CMD(Cmd) )
#define GET_NOR_BLOCK_ADDRESS(BaseAddr,Lba,LbaSize)( BaseAddr + (UINTN)(Lba * LbaSize) )

// Status Register Bits
#define P30_SR_BIT_WRITE                          0x00800080     /* Bit 7 */
#define P30_SR_BIT_ERASE_SUSPEND                  0x00400040     /* Bit 6 */
#define P30_SR_BIT_ERASE                          0x00200020     /* Bit 5 */
#define P30_SR_BIT_PROGRAM                        0x00100010     /* Bit 4 */
#define P30_SR_BIT_VPP                            0x00080008     /* Bit 3 */
#define P30_SR_BIT_PROGRAM_SUSPEND                0x00040004     /* Bit 2 */
#define P30_SR_BIT_BLOCK_LOCKED                   0x00020002     /* Bit 1 */
#define P30_SR_BIT_BEFP                           0x00010001     /* Bit 0 */

// Device Commands for Intel StrataFlash(R) Embedded Memory (P30) Family

// On chip buffer size for buffered programming operations
// There are 2 chips, each chip can buffer up to 32 (16-bit)words, and each word is 2 bytes.
// Therefore the total size of the buffer is 2 x 32 x 2 = 128 bytes
#define P30_MAX_BUFFER_SIZE_IN_BYTES              ((UINTN)128)
#define P30_MAX_BUFFER_SIZE_IN_WORDS              (P30_MAX_BUFFER_SIZE_IN_BYTES/((UINTN)4))
#define MAX_BUFFERED_PROG_ITERATIONS              10000000
#define BOUNDARY_OF_32_WORDS                      0x7F

// CFI Addresses
#define P30_CFI_ADDR_QUERY_UNIQUE_QRY             0x10
#define P30_CFI_ADDR_VENDOR_ID                    0x13

// CFI Data
#define CFI_QRY                                   0x00595251

// READ Commands
#define P30_CMD_READ_DEVICE_ID                    0x0090
#define P30_CMD_READ_STATUS_REGISTER              0x0070
#define P30_CMD_CLEAR_STATUS_REGISTER             0x0050
#define P30_CMD_READ_ARRAY                        0x00FF
#define P30_CMD_READ_CFI_QUERY                    0x0098

// WRITE Commands
#define P30_CMD_WORD_PROGRAM_SETUP                0x0040
#define P30_CMD_ALTERNATE_WORD_PROGRAM_SETUP      0x0010
#define P30_CMD_BUFFERED_PROGRAM_SETUP            0x00E8
#define P30_CMD_BUFFERED_PROGRAM_CONFIRM          0x00D0
#define P30_CMD_BEFP_SETUP                        0x0080
#define P30_CMD_BEFP_CONFIRM                      0x00D0

// ERASE Commands
#define P30_CMD_BLOCK_ERASE_SETUP                 0x0020
#define P30_CMD_BLOCK_ERASE_CONFIRM               0x00D0

// SUSPEND Commands
#define P30_CMD_PROGRAM_OR_ERASE_SUSPEND          0x00B0
#define P30_CMD_SUSPEND_RESUME                    0x00D0

// BLOCK LOCKING / UNLOCKING Commands
#define P30_CMD_LOCK_BLOCK_SETUP                  0x0060
#define P30_CMD_LOCK_BLOCK                        0x0001
#define P30_CMD_UNLOCK_BLOCK                      0x00D0
#define P30_CMD_LOCK_DOWN_BLOCK                   0x002F

// PROTECTION Commands
#define P30_CMD_PROGRAM_PROTECTION_REGISTER_SETUP 0x00C0

// CONFIGURATION Commands
#define P30_CMD_READ_CONFIGURATION_REGISTER_SETUP 0x0060
#define P30_CMD_READ_CONFIGURATION_REGISTER       0x0003

#define NOR_FLASH_SIGNATURE                       SIGNATURE_32('n', 'o', 'r', '0')
#define INSTANCE_FROM_FVB_THIS(a)                 CR(a, NOR_FLASH_INSTANCE, FvbProtocol, NOR_FLASH_SIGNATURE)
#define INSTANCE_FROM_BLKIO_THIS(a)               CR(a, NOR_FLASH_INSTANCE, BlockIoProtocol, NOR_FLASH_SIGNATURE)

typedef struct _NOR_FLASH_INSTANCE                NOR_FLASH_INSTANCE;

typedef EFI_STATUS (*NOR_FLASH_INITIALIZE)        (NOR_FLASH_INSTANCE* Instance);

typedef struct {
    UINTN                             BaseAddress;
    UINTN                             Size;
    UINTN                             BlockSize;
    BOOLEAN                           SupportFvb;
    EFI_GUID                          Guid;
} NOR_FLASH_DESCRIPTION;

typedef struct {
  VENDOR_DEVICE_PATH                  Vendor;
  EFI_DEVICE_PATH_PROTOCOL            End;
} NOR_FLASH_DEVICE_PATH;

struct _NOR_FLASH_INSTANCE {
  UINT32                              Signature;
  EFI_HANDLE                          Handle;

  BOOLEAN                             Initialized;
  NOR_FLASH_INITIALIZE                Initialize;

  UINTN                               BaseAddress;
  UINTN                               Size;

  EFI_BLOCK_IO_PROTOCOL               BlockIoProtocol;
  EFI_BLOCK_IO_MEDIA                  Media;

  BOOLEAN                             SupportFvb;
  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL FvbProtocol;

  NOR_FLASH_DEVICE_PATH	              DevicePath;
};

EFI_STATUS
EFIAPI
NorFlashBlkIoInitialize (
  IN NOR_FLASH_INSTANCE*      Instance
  );

EFI_STATUS
NorFlashReadCfiData (
  IN UINTN                    BaseAddress,
  IN UINTN                    CFI_Offset,
  IN UINT32                   NumberOfBytes,
  OUT UINT32                  *Data
);

EFI_STATUS
NorFlashWriteBuffer (
    IN  UINTN                 TargetAddress,
    IN  UINTN                 BufferSizeInBytes,
    IN  UINT32                *Buffer
);


//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.Reset
//
EFI_STATUS
EFIAPI
NorFlashBlockIoReset (
  IN EFI_BLOCK_IO_PROTOCOL    *This,
  IN BOOLEAN                  ExtendedVerification
  );

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.ReadBlocks
//
EFI_STATUS
EFIAPI
NorFlashBlockIoReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSizeInBytes,
  OUT VOID                    *Buffer
);

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.WriteBlocks
//
EFI_STATUS
EFIAPI
NorFlashBlockIoWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSizeInBytes,
  IN  VOID                    *Buffer
);

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.FlushBlocks
//
EFI_STATUS
EFIAPI
NorFlashBlockIoFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL    *This
);


//
// NorFlashFvbDxe.c
//

EFI_STATUS
EFIAPI
NorFlashFvbInitialize (
  IN NOR_FLASH_INSTANCE*                            Instance
);

EFI_STATUS
EFIAPI
FvbGetAttributes(
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL     *This,
  OUT       EFI_FVB_ATTRIBUTES_2                    *Attributes
);

EFI_STATUS
EFIAPI
FvbSetAttributes(
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL     *This,
  IN OUT    EFI_FVB_ATTRIBUTES_2                    *Attributes
);

EFI_STATUS
EFIAPI
FvbGetPhysicalAddress(
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL     *This,
  OUT       EFI_PHYSICAL_ADDRESS                    *Address
);

EFI_STATUS
EFIAPI
FvbGetBlockSize(
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL     *This,
  IN        EFI_LBA                                 Lba,
  OUT       UINTN                                   *BlockSize,
  OUT       UINTN                                   *NumberOfBlocks
);

EFI_STATUS
EFIAPI
FvbRead(
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL     *This,
  IN        EFI_LBA                                 Lba,
  IN        UINTN                                   Offset,
  IN OUT    UINTN                                   *NumBytes,
  IN OUT    UINT8                                   *Buffer
);

EFI_STATUS
EFIAPI
FvbWrite(
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL     *This,
  IN        EFI_LBA                                 Lba,
  IN        UINTN                                   Offset,
  IN OUT    UINTN                                   *NumBytes,
  IN        UINT8                                   *Buffer
);

EFI_STATUS
EFIAPI
FvbEraseBlocks(
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL     *This,
  ...
);

//
// NorFlashDxe.c
//

EFI_STATUS
NorFlashUnlockAndEraseSingleBlock(
  IN  UINTN             BlockAddress
);

EFI_STATUS
NorFlashWriteSingleBlock (
  IN  UINTN             DeviceBaseAddress,
  IN  EFI_LBA           Lba,
  IN  UINT32            *pDataBuffer,
  IN  UINT32            BlockSizeInWords
);

EFI_STATUS
NorFlashWriteBlocks (
  IN  NOR_FLASH_INSTANCE *Instance,
  IN  EFI_LBA           Lba,
  IN  UINTN             BufferSizeInBytes,
  IN  VOID              *Buffer
);

EFI_STATUS
NorFlashReadBlocks (
  IN  NOR_FLASH_INSTANCE *Instance,
  IN  EFI_LBA           Lba,
  IN  UINTN             BufferSizeInBytes,
  OUT VOID              *Buffer
);

EFI_STATUS
NorFlashReset (
  IN  NOR_FLASH_INSTANCE *Instance
);

#endif /* __NOR_FLASH_DXE_H__ */