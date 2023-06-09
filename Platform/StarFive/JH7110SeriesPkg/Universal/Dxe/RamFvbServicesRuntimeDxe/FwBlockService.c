/**@file

  Copyright (c) 2019, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>
  Copyright (c) 2006 - 2014, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  Module Name:

    FWBlockService.c

  Abstract:

  Revision History

**/

//
// The protocols, PPI and GUID defintions for this module
//
#include <Protocol/DevicePath.h>
#include <Protocol/FirmwareVolumeBlock.h>

//
// The Library classes this module consumes
//
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "FwBlockService.h"
#include "RamFlash.h"

#define EFI_FVB2_STATUS \
          (EFI_FVB2_READ_STATUS | EFI_FVB2_WRITE_STATUS | EFI_FVB2_LOCK_STATUS)

ESAL_FWB_GLOBAL         *mFvbModuleGlobal;

FV_MEMMAP_DEVICE_PATH mFvMemmapDevicePathTemplate = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_MEMMAP_DP,
      {
        (UINT8)(sizeof (MEMMAP_DEVICE_PATH)),
        (UINT8)(sizeof (MEMMAP_DEVICE_PATH) >> 8)
      }
    },
    EfiMemoryMappedIO,
    (EFI_PHYSICAL_ADDRESS) 0,
    (EFI_PHYSICAL_ADDRESS) 0,
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};

FV_PIWG_DEVICE_PATH mFvPIWGDevicePathTemplate = {
  {
    {
      MEDIA_DEVICE_PATH,
      MEDIA_PIWG_FW_VOL_DP,
      {
        (UINT8)(sizeof (MEDIA_FW_VOL_DEVICE_PATH)),
        (UINT8)(sizeof (MEDIA_FW_VOL_DEVICE_PATH) >> 8)
      }
    },
    { 0 }
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};

EFI_FW_VOL_BLOCK_DEVICE mFvbDeviceTemplate = {
  FVB_DEVICE_SIGNATURE,
  NULL,
  0,
  {
    FvbProtocolGetAttributes,
    FvbProtocolSetAttributes,
    FvbProtocolGetPhysicalAddress,
    FvbProtocolGetBlockSize,
    FvbProtocolRead,
    FvbProtocolWrite,
    FvbProtocolEraseBlocks,
    NULL
  }
};

/*++

  Routine Description:
    Retrieves the physical address of a memory mapped FV

  Arguments:
    Instance              - The FV instance whose base address is going to be
                            returned
    Global                - Pointer to ESAL_FWB_GLOBAL that contains all
                            instance data
    FwhInstance           - The EFI_FW_VOL_INSTANCE fimrware instance structure

  Returns:
    EFI_SUCCESS           - Successfully returns
    EFI_INVALID_PARAMETER - Instance not found

--*/
EFI_STATUS
GetFvbInstance (
  IN  UINTN                               Instance,
  IN  ESAL_FWB_GLOBAL                     *Global,
  OUT EFI_FW_VOL_INSTANCE                 **FwhInstance
)
{
  EFI_FW_VOL_INSTANCE *FwhRecord;

  *FwhInstance = NULL;
  if (Instance >= Global->NumFv) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Find the right instance of the FVB private data
  //
  FwhRecord = Global->FvInstance;
  while (Instance > 0) {
    FwhRecord = (EFI_FW_VOL_INSTANCE *)
      (
        (UINTN)FwhRecord + FwhRecord->VolumeHeader.HeaderLength +
          (sizeof (EFI_FW_VOL_INSTANCE) - sizeof (EFI_FIRMWARE_VOLUME_HEADER))
      );
    Instance--;
  }

  *FwhInstance = FwhRecord;

  return EFI_SUCCESS;
}

/*++

  Routine Description:
    Retrieves the physical address of a memory mapped FV

  Arguments:
    Instance              - The FV instance whose base address is going to be
                            returned
    Address               - Pointer to a caller allocated EFI_PHYSICAL_ADDRESS
                            that on successful return, contains the base
                            address of the firmware volume.
    Global                - Pointer to ESAL_FWB_GLOBAL that contains all
                            instance data

  Returns:
    EFI_SUCCESS           - Successfully returns
    EFI_INVALID_PARAMETER - Instance not found

--*/
EFI_STATUS
FvbGetPhysicalAddress (
  IN UINTN                                Instance,
  OUT EFI_PHYSICAL_ADDRESS                *Address,
  IN ESAL_FWB_GLOBAL                      *Global
)
{
  EFI_FW_VOL_INSTANCE *FwhInstance;
  EFI_STATUS          Status;

  //
  // Find the right instance of the FVB private data
  //
  Status = GetFvbInstance (Instance, Global, &FwhInstance);
  ASSERT_EFI_ERROR (Status);
  *Address = FwhInstance->FvBase;

  return EFI_SUCCESS;
}
/*++

  Routine Description:
    Retrieves attributes, insures positive polarity of attribute bits, returns
    resulting attributes in output parameter

  Arguments:
    Instance              - The FV instance whose attributes is going to be
                            returned
    Attributes            - Output buffer which contains attributes
    Global                - Pointer to ESAL_FWB_GLOBAL that contains all
                            instance data

  Returns:
    EFI_SUCCESS           - Successfully returns
    EFI_INVALID_PARAMETER - Instance not found

--*/
EFI_STATUS
FvbGetVolumeAttributes (
  IN UINTN                                Instance,
  OUT EFI_FVB_ATTRIBUTES_2                *Attributes,
  IN ESAL_FWB_GLOBAL                      *Global
  )
{
  EFI_FW_VOL_INSTANCE *FwhInstance;
  EFI_STATUS          Status;

  //
  // Find the right instance of the FVB private data
  //
  Status = GetFvbInstance (Instance, Global, &FwhInstance);
  ASSERT_EFI_ERROR (Status);
  *Attributes = FwhInstance->VolumeHeader.Attributes;

  return EFI_SUCCESS;
}

/*++

  Routine Description:
    Retrieves the starting address of an LBA in an FV

  Arguments:
    Instance              - The FV instance which the Lba belongs to
    Lba                   - The logical block address
    LbaAddress            - On output, contains the physical starting address
                            of the Lba
    LbaLength             - On output, contains the length of the block
    NumOfBlocks           - A pointer to a caller allocated UINTN in which the
                            number of consecutive blocks starting with Lba is
                            returned. All blocks in this range have a size of
                            BlockSize
    Global                - Pointer to ESAL_FWB_GLOBAL that contains all
                            instance data

  Returns:
    EFI_SUCCESS           - Successfully returns
    EFI_INVALID_PARAMETER - Instance not found

--*/
EFI_STATUS
FvbGetLbaAddress (
  IN  UINTN                               Instance,
  IN  EFI_LBA                             Lba,
  OUT UINTN                               *LbaAddress,
  OUT UINTN                               *LbaLength,
  OUT UINTN                               *NumOfBlocks,
  IN  ESAL_FWB_GLOBAL                     *Global
  )
{
  UINT32                  NumBlocks;
  UINT32                  BlockLength;
  UINTN                   Offset;
  EFI_LBA                 StartLba;
  EFI_LBA                 NextLba;
  EFI_FW_VOL_INSTANCE     *FwhInstance;
  EFI_FV_BLOCK_MAP_ENTRY  *BlockMap;
  EFI_STATUS              Status;

  //
  // Find the right instance of the FVB private data
  //
  Status = GetFvbInstance (Instance, Global, &FwhInstance);
  ASSERT_EFI_ERROR (Status);

  StartLba  = 0;
  Offset    = 0;
  BlockMap  = &(FwhInstance->VolumeHeader.BlockMap[0]);

  //
  // Parse the blockmap of the FV to find which map entry the Lba belongs to
  //
  while (TRUE) {
    NumBlocks   = BlockMap->NumBlocks;
    BlockLength = BlockMap->Length;

    if (NumBlocks == 0 || BlockLength == 0) {
      return EFI_INVALID_PARAMETER;
    }

    NextLba = StartLba + NumBlocks;

    //
    // The map entry found
    //
    if (Lba >= StartLba && Lba < NextLba) {
      Offset = Offset + (UINTN) MultU64x32 ((Lba - StartLba), BlockLength);
      if (LbaAddress != NULL) {
        *LbaAddress = FwhInstance->FvBase + Offset;
      }

      if (LbaLength != NULL) {
        *LbaLength = BlockLength;
      }

      if (NumOfBlocks != NULL) {
        *NumOfBlocks = (UINTN) (NextLba - Lba);
      }

      return EFI_SUCCESS;
    }

    StartLba  = NextLba;
    Offset    = Offset + NumBlocks * BlockLength;
    BlockMap++;
  }
}
/*++

  Routine Description:
    Modifies the current settings of the firmware volume according to the
    input parameter, and returns the new setting of the volume

  Arguments:
    Instance              - The FV instance whose attributes is going to be
                            modified
    Attributes            - On input, it is a pointer to EFI_FVB_ATTRIBUTES_2
                            containing the desired firmware volume settings.
                            On successful return, it contains the new settings
                            of the firmware volume
    Global                - Pointer to ESAL_FWB_GLOBAL that contains all
                            instance data

  Returns:
    EFI_SUCCESS           - Successfully returns
    EFI_ACCESS_DENIED     - The volume setting is locked and cannot be modified
    EFI_INVALID_PARAMETER - Instance not found, or The attributes requested are
                            in conflict with the capabilities as declared in
                            the firmware volume header

--*/
EFI_STATUS
FvbSetVolumeAttributes (
  IN UINTN                                  Instance,
  IN OUT EFI_FVB_ATTRIBUTES_2               *Attributes,
  IN ESAL_FWB_GLOBAL                        *Global
  )
{
  EFI_FW_VOL_INSTANCE   *FwhInstance;
  EFI_FVB_ATTRIBUTES_2  OldAttributes;
  EFI_FVB_ATTRIBUTES_2  *AttribPtr;
  UINT32                Capabilities;
  UINT32                OldStatus;
  UINT32                NewStatus;
  EFI_STATUS            Status;
  EFI_FVB_ATTRIBUTES_2  UnchangedAttributes;

  //
  // Find the right instance of the FVB private data
  //
  Status = GetFvbInstance (Instance, Global, &FwhInstance);
  ASSERT_EFI_ERROR (Status);

  AttribPtr     =
    (EFI_FVB_ATTRIBUTES_2 *) &(FwhInstance->VolumeHeader.Attributes);
  OldAttributes = *AttribPtr;
  Capabilities  = OldAttributes & (EFI_FVB2_READ_DISABLED_CAP | \
                                   EFI_FVB2_READ_ENABLED_CAP | \
                                   EFI_FVB2_WRITE_DISABLED_CAP | \
                                   EFI_FVB2_WRITE_ENABLED_CAP | \
                                   EFI_FVB2_LOCK_CAP \
                                   );
  OldStatus     = OldAttributes & EFI_FVB2_STATUS;
  NewStatus     = *Attributes & EFI_FVB2_STATUS;

  UnchangedAttributes = EFI_FVB2_READ_DISABLED_CAP  | \
                        EFI_FVB2_READ_ENABLED_CAP   | \
                        EFI_FVB2_WRITE_DISABLED_CAP | \
                        EFI_FVB2_WRITE_ENABLED_CAP  | \
                        EFI_FVB2_LOCK_CAP           | \
                        EFI_FVB2_STICKY_WRITE       | \
                        EFI_FVB2_MEMORY_MAPPED      | \
                        EFI_FVB2_ERASE_POLARITY     | \
                        EFI_FVB2_READ_LOCK_CAP      | \
                        EFI_FVB2_WRITE_LOCK_CAP     | \
                        EFI_FVB2_ALIGNMENT;

  //
  // Some attributes of FV is read only can *not* be set
  //
  if ((OldAttributes & UnchangedAttributes) ^
      (*Attributes & UnchangedAttributes)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // If firmware volume is locked, no status bit can be updated
  //
  if (OldAttributes & EFI_FVB2_LOCK_STATUS) {
    if (OldStatus ^ NewStatus) {
      return EFI_ACCESS_DENIED;
    }
  }
  //
  // Test read disable
  //
  if ((Capabilities & EFI_FVB2_READ_DISABLED_CAP) == 0) {
    if ((NewStatus & EFI_FVB2_READ_STATUS) == 0) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // Test read enable
  //
  if ((Capabilities & EFI_FVB2_READ_ENABLED_CAP) == 0) {
    if (NewStatus & EFI_FVB2_READ_STATUS) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // Test write disable
  //
  if ((Capabilities & EFI_FVB2_WRITE_DISABLED_CAP) == 0) {
    if ((NewStatus & EFI_FVB2_WRITE_STATUS) == 0) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // Test write enable
  //
  if ((Capabilities & EFI_FVB2_WRITE_ENABLED_CAP) == 0) {
    if (NewStatus & EFI_FVB2_WRITE_STATUS) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // Test lock
  //
  if ((Capabilities & EFI_FVB2_LOCK_CAP) == 0) {
    if (NewStatus & EFI_FVB2_LOCK_STATUS) {
      return EFI_INVALID_PARAMETER;
    }
  }

  *AttribPtr  = (*AttribPtr) & (0xFFFFFFFF & (~EFI_FVB2_STATUS));
  *AttribPtr  = (*AttribPtr) | NewStatus;
  *Attributes = *AttribPtr;

  return EFI_SUCCESS;
}
/*++

  Routine Description:

    Retrieves the physical address of the device.

  Arguments:

    This                  - Calling context
    Address               - Output buffer containing the address.

  Returns:
    EFI_SUCCESS           - Successfully returns

--*/
EFI_STATUS
EFIAPI
FvbProtocolGetPhysicalAddress (
  IN CONST EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  OUT EFI_PHYSICAL_ADDRESS                        *Address
  )
{
  EFI_FW_VOL_BLOCK_DEVICE *FvbDevice;

  FvbDevice = FVB_DEVICE_FROM_THIS (This);

  return FvbGetPhysicalAddress (FvbDevice->Instance, Address,
           mFvbModuleGlobal);
}

/*++

  Routine Description:
    Retrieve the size of a logical block

  Arguments:
    This                  - Calling context
    Lba                   - Indicates which block to return the size for.
    BlockSize             - A pointer to a caller allocated UINTN in which
                            the size of the block is returned
    NumOfBlocks           - a pointer to a caller allocated UINTN in which the
                            number of consecutive blocks starting with Lba is
                            returned. All blocks in this range have a size of
                            BlockSize

  Returns:
    EFI_SUCCESS           - The firmware volume was read successfully and
                            contents are in Buffer

--*/
EFI_STATUS
EFIAPI
FvbProtocolGetBlockSize (
  IN CONST EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN CONST EFI_LBA                                     Lba,
  OUT UINTN                                       *BlockSize,
  OUT UINTN                                       *NumOfBlocks
  )
{
  EFI_FW_VOL_BLOCK_DEVICE *FvbDevice;

  FvbDevice = FVB_DEVICE_FROM_THIS (This);

  return FvbGetLbaAddress (
          FvbDevice->Instance,
          Lba,
          NULL,
          BlockSize,
          NumOfBlocks,
          mFvbModuleGlobal
          );
}

/*++

  Routine Description:
      Retrieves Volume attributes.  No polarity translations are done.

  Arguments:
      This                - Calling context
      Attributes          - output buffer which contains attributes

  Returns:
    EFI_SUCCESS           - Successfully returns

--*/
EFI_STATUS
EFIAPI
FvbProtocolGetAttributes (
  IN CONST EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  OUT EFI_FVB_ATTRIBUTES_2                              *Attributes
  )
{
  EFI_FW_VOL_BLOCK_DEVICE *FvbDevice;

  FvbDevice = FVB_DEVICE_FROM_THIS (This);

  return FvbGetVolumeAttributes (FvbDevice->Instance, Attributes,
           mFvbModuleGlobal);
}

/*++

  Routine Description:
    Sets Volume attributes. No polarity translations are done.

  Arguments:
    This                  - Calling context
    Attributes            - output buffer which contains attributes

  Returns:
    EFI_SUCCESS           - Successfully returns

--*/
EFI_STATUS
EFIAPI
FvbProtocolSetAttributes (
  IN CONST EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN OUT EFI_FVB_ATTRIBUTES_2                           *Attributes
  )
{
  EFI_FW_VOL_BLOCK_DEVICE *FvbDevice;

  FvbDevice = FVB_DEVICE_FROM_THIS (This);

  return FvbSetVolumeAttributes (FvbDevice->Instance, Attributes,
           mFvbModuleGlobal);
}

/*++

  Routine Description:

    The EraseBlock() function erases one or more blocks as denoted by the
    variable argument list. The entire parameter list of blocks must be
    verified prior to erasing any blocks.  If a block is requested that does
    not exist within the associated firmware volume (it has a larger index than
    the last block of the firmware volume), the EraseBlock() function must
    return EFI_INVALID_PARAMETER without modifying the contents of the firmware
    volume.

  Arguments:
    This                  - Calling context
    ...                   - Starting LBA followed by Number of Lba to erase.
                            a -1 to terminate the list.

  Returns:
    EFI_SUCCESS           - The erase request was successfully completed
    EFI_ACCESS_DENIED     - The firmware volume is in the WriteDisabled state
    EFI_DEVICE_ERROR      - The block device is not functioning correctly and
                            could not be written. Firmware device may have been
                            partially erased

--*/
EFI_STATUS
EFIAPI
FvbProtocolEraseBlocks (
  IN CONST EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    *This,
  ...
  )
{
  EFI_FW_VOL_BLOCK_DEVICE *FvbDevice;
  EFI_FW_VOL_INSTANCE     *FwhInstance;
  UINTN                   NumOfBlocks;
  VA_LIST                 args;
  EFI_LBA                 StartingLba;
  UINTN                   NumOfLba;
  EFI_STATUS              Status;

  FvbDevice = FVB_DEVICE_FROM_THIS (This);

  Status    = GetFvbInstance (FvbDevice->Instance, mFvbModuleGlobal,
                &FwhInstance);
  ASSERT_EFI_ERROR (Status);

  NumOfBlocks = FwhInstance->NumOfBlocks;

  VA_START (args, This);

  do {
    StartingLba = VA_ARG (args, EFI_LBA);
    if (StartingLba == EFI_LBA_LIST_TERMINATOR) {
      break;
    }

    NumOfLba = VA_ARG (args, UINT32);

    //
    // Check input parameters
    //
    if ((NumOfLba == 0) || ((StartingLba + NumOfLba) > NumOfBlocks)) {
      VA_END (args);
      return EFI_INVALID_PARAMETER;
    }
  } while (1);

  VA_END (args);

  VA_START (args, This);
  do {
    StartingLba = VA_ARG (args, EFI_LBA);
    if (StartingLba == EFI_LBA_LIST_TERMINATOR) {
      break;
    }

    NumOfLba = VA_ARG (args, UINT32);

    while (NumOfLba > 0) {
      Status = RamFlashEraseBlock (StartingLba);
      if (EFI_ERROR (Status)) {
        VA_END (args);
        return Status;
      }

      StartingLba++;
      NumOfLba--;
    }

  } while (1);

  VA_END (args);

  return EFI_SUCCESS;
}

/*++

  Routine Description:

    Writes data beginning at Lba:Offset from FV. The write terminates either
    when *NumBytes of data have been written, or when a block boundary is
    reached.  *NumBytes is updated to reflect the actual number of bytes
    written. The write opertion does not include erase. This routine will
    attempt to write only the specified bytes. If the writes do not stick,
    it will return an error.

  Arguments:
    This                  - Calling context
    Lba                   - Block in which to begin write
    Offset                - Offset in the block at which to begin write
    NumBytes              - On input, indicates the requested write size. On
                            output, indicates the actual number of bytes
                            written
    Buffer                - Buffer containing source data for the write.

  Returns:
    EFI_SUCCESS           - The firmware volume was written successfully
    EFI_BAD_BUFFER_SIZE   - Write attempted across a LBA boundary. On output,
                            NumBytes contains the total number of bytes
                            actually written
    EFI_ACCESS_DENIED     - The firmware volume is in the WriteDisabled state
    EFI_DEVICE_ERROR      - The block device is not functioning correctly and
                            could not be written
    EFI_INVALID_PARAMETER - NumBytes or Buffer are NULL

--*/
EFI_STATUS
EFIAPI
FvbProtocolWrite (
  IN CONST EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN       EFI_LBA                                      Lba,
  IN       UINTN                                        Offset,
  IN OUT   UINTN                                    *NumBytes,
  IN       UINT8                                        *Buffer
  )
{
  return RamFlashWrite (Lba, Offset, NumBytes, Buffer);
}

/*++

  Routine Description:

    Reads data beginning at Lba:Offset from FV. The Read terminates either
    when *NumBytes of data have been read, or when a block boundary is
    reached.  *NumBytes is updated to reflect the actual number of bytes
    written. The write opertion does not include erase. This routine will
    attempt to write only the specified bytes. If the writes do not stick,
    it will return an error.

  Arguments:
    This                  - Calling context
    Lba                   - Block in which to begin Read
    Offset                - Offset in the block at which to begin Read
    NumBytes              - On input, indicates the requested write size. On
                            output, indicates the actual number of bytes Read
    Buffer                - Buffer containing source data for the Read.

  Returns:
    EFI_SUCCESS           - The firmware volume was read successfully and
                            contents are in Buffer
    EFI_BAD_BUFFER_SIZE   - Read attempted across a LBA boundary. On output,
                            NumBytes contains the total number of bytes
                            returned in Buffer
    EFI_ACCESS_DENIED     - The firmware volume is in the ReadDisabled state
    EFI_DEVICE_ERROR      - The block device is not functioning correctly and
                            could not be read
    EFI_INVALID_PARAMETER - NumBytes or Buffer are NULL

--*/
EFI_STATUS
EFIAPI
FvbProtocolRead (
  IN CONST EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL           *This,
  IN CONST EFI_LBA                                      Lba,
  IN CONST UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  )
{
  return RamFlashRead (Lba, Offset, NumBytes, Buffer);
}

/*++

  Routine Description:
    Check the integrity of firmware volume header

  Arguments:
    FwVolHeader           - A pointer to a firmware volume header

  Returns:
    EFI_SUCCESS           - The firmware volume is consistent
    EFI_NOT_FOUND         - The firmware volume has corrupted. So it is not an
                            FV

--*/
EFI_STATUS
ValidateFvHeader (
  EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader
  )
{
  UINT16 Checksum;

  //
  // Verify the header revision, header signature, length
  // Length of FvBlock cannot be 2**64-1
  // HeaderLength cannot be an odd number
  //
  if ((FwVolHeader->Revision != EFI_FVH_REVISION) ||
      (FwVolHeader->Signature != EFI_FVH_SIGNATURE) ||
      (FwVolHeader->FvLength == ((UINTN) -1)) ||
      ((FwVolHeader->HeaderLength & 0x01) != 0)
      ) {
    return EFI_NOT_FOUND;
  }

  //
  // Verify the header checksum
  //

  Checksum = CalculateSum16 ((UINT16 *) FwVolHeader,
               FwVolHeader->HeaderLength);
  if (Checksum != 0) {
    UINT16 Expected;

    Expected =
      (UINT16) (((UINTN) FwVolHeader->Checksum + 0x10000 - Checksum) & 0xffff);

    DEBUG ((DEBUG_INFO, "FV@%p Checksum is 0x%x, expected 0x%x\n",
            FwVolHeader, FwVolHeader->Checksum, Expected));
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MarkMemoryRangeForRuntimeAccess (
  EFI_PHYSICAL_ADDRESS                BaseAddress,
  UINTN                               Length
  )
{
  EFI_STATUS                          Status;

  //
  // Mark flash region as runtime memory
  //
  Status = gDS->RemoveMemorySpace (
                  BaseAddress,
                  Length
                  );
#if 0 // Will get access denied error, is it related memory initilize ?
  Status = gDS->AddMemorySpace (
                  EfiGcdMemoryTypeSystemMemory,
                  BaseAddress,
                  Length,
                  EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                  );
  ASSERT_EFI_ERROR (Status);
#endif
  Status = gBS->AllocatePages (
                  AllocateAddress,
                  EfiRuntimeServicesData,
                  EFI_SIZE_TO_PAGES (Length),
                  &BaseAddress
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

STATIC
EFI_STATUS
InitializeVariableFvHeader (
  VOID
  )
{
  EFI_STATUS                          Status;
  EFI_FIRMWARE_VOLUME_HEADER          *GoodFwVolHeader;
  EFI_FIRMWARE_VOLUME_HEADER          *FwVolHeader;
  UINTN                               Length;
  UINTN                               WriteLength;
  UINTN                               BlockSize;

  FwVolHeader =
    (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN)
      PcdGet32 (PcdPlatformFlashNvStorageVariableBase);

  Length =
    (FixedPcdGet32 (PcdFlashNvStorageVariableSize) +
     FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize) +
     FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize));

  BlockSize = PcdGet32 (PcdVariableFdBlockSize);

  Status      = ValidateFvHeader (FwVolHeader);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ValidateFvHeader() return ok\n"));
    if (FwVolHeader->FvLength != Length ||
        FwVolHeader->BlockMap[0].Length != BlockSize) {
      Status = EFI_VOLUME_CORRUPTED;
     DEBUG ((DEBUG_INFO, "FwVolHeader->FvLength(%x) != Length(%x) || FwVolHeader->BlockMap[0].Length(%x) != BlockSize(%x)\n", FwVolHeader->FvLength, Length, FwVolHeader->BlockMap[0].Length, BlockSize));
    }
  }
  else {
    DEBUG ((DEBUG_INFO, "ValidateFvHeader() return failed\n"));
  }
  if (EFI_ERROR (Status)) {
    UINTN   Offset;
    UINTN   Start;

    DEBUG ((DEBUG_INFO,
      "Variable FV header is not valid. It will be reinitialized.\n"));

    //
    // Get FvbInfo to provide in FwhInstance.
    //
    Status = GetFvbInfo (Length, &GoodFwVolHeader);
    ASSERT (!EFI_ERROR (Status));

    Start = (UINTN)(UINT8*) FwVolHeader - PcdGet32 (PcdVariableFdBaseAddress);
    ASSERT (Start % BlockSize == 0 && Length % BlockSize == 0);
    ASSERT (GoodFwVolHeader->HeaderLength <= BlockSize);

    //
    // Erase all the blocks
    //
    for (Offset = Start; Offset < Start + Length; Offset += BlockSize) {
      Status = RamFlashEraseBlock (Offset / BlockSize);
      ASSERT_EFI_ERROR (Status);
    }

    //
    // Write good FV header
    //
    WriteLength = GoodFwVolHeader->HeaderLength;
    Status = RamFlashWrite (
               Start / BlockSize,
               0,
               &WriteLength,
               (UINT8 *) GoodFwVolHeader);
    ASSERT_EFI_ERROR (Status);
    ASSERT (WriteLength == GoodFwVolHeader->HeaderLength);
  }

  return Status;
}

/*++

  Routine Description:
    This function does common initialization for FVB services

  Arguments:

  Returns:

--*/
EFI_STATUS
EFIAPI
FvbInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                          Status;
  EFI_FW_VOL_INSTANCE                 *FwhInstance;
  EFI_FIRMWARE_VOLUME_HEADER          *FwVolHeader;
  UINT32                              BufferSize;
  EFI_FV_BLOCK_MAP_ENTRY              *PtrBlockMapEntry;
  EFI_FW_VOL_BLOCK_DEVICE             *FvbDevice;
  UINT32                              MaxLbaSize;
  EFI_PHYSICAL_ADDRESS                BaseAddress;
  UINTN                               Length;
  UINTN                               NumOfBlocks;

  if (EFI_ERROR (RamFlashInitialize ())) {
    //
    // Return an error so image will be unloaded
    //
    DEBUG ((DEBUG_INFO,
      "RAM flash was not detected. Writable FVB is not being installed.\n"));
    return EFI_WRITE_PROTECTED;
  }

  //
  // Allocate runtime services data for global variable, which contains
  // the private data of all firmware volume block instances
  //
  mFvbModuleGlobal = AllocateRuntimePool (sizeof (ESAL_FWB_GLOBAL));
  ASSERT (mFvbModuleGlobal != NULL);

  BaseAddress = (UINTN) PcdGet32 (PcdVariableFdBaseAddress);
  Length = PcdGet32 (PcdVariableFdSize);
  DEBUG ((DEBUG_INFO, "FvbInitialize(): BaseAddress: 0x%lx Length:0x%x\n", BaseAddress, Length));
  Status = InitializeVariableFvHeader ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO,
      "RAM Flash: Unable to initialize variable FV header\n"));
    return EFI_WRITE_PROTECTED;
  }

  FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) BaseAddress;
  Status      = ValidateFvHeader (FwVolHeader);
  if (EFI_ERROR (Status)) {
    //
    // Get FvbInfo
    //
    DEBUG ((DEBUG_INFO, "FvbInitialize(): ValidateFvHeader() return error(%r)\n", Status));

    Status = GetFvbInfo (Length, &FwVolHeader);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "FvbInitialize(): GetFvbInfo (Length, &FwVolHeader) return error(%r)\n", Status));
      return EFI_WRITE_PROTECTED;
    }
  }

  BufferSize = (sizeof (EFI_FW_VOL_INSTANCE) +
                FwVolHeader->HeaderLength -
                sizeof (EFI_FIRMWARE_VOLUME_HEADER)
                );
  mFvbModuleGlobal->FvInstance = AllocateRuntimePool (BufferSize);
  ASSERT (mFvbModuleGlobal->FvInstance != NULL);

  FwhInstance = mFvbModuleGlobal->FvInstance;

  mFvbModuleGlobal->NumFv                   = 0;
  MaxLbaSize = 0;

  FwVolHeader =
    (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN)
      PcdGet32 (PcdPlatformFlashNvStorageVariableBase);

  FwhInstance->FvBase = (UINTN) BaseAddress;

  CopyMem ((UINTN *) &(FwhInstance->VolumeHeader), (UINTN *) FwVolHeader,
    FwVolHeader->HeaderLength);
  FwVolHeader = &(FwhInstance->VolumeHeader);

  NumOfBlocks = 0;

  for (PtrBlockMapEntry = FwVolHeader->BlockMap;
       PtrBlockMapEntry->NumBlocks != 0;
       PtrBlockMapEntry++) {
    //
    // Get the maximum size of a block.
    //
    if (MaxLbaSize < PtrBlockMapEntry->Length) {
      MaxLbaSize = PtrBlockMapEntry->Length;
    }

    NumOfBlocks = NumOfBlocks + PtrBlockMapEntry->NumBlocks;
  }

  //
  // The total number of blocks in the FV.
  //
  FwhInstance->NumOfBlocks = NumOfBlocks;

  //
  // Add a FVB Protocol Instance
  //
  FvbDevice = AllocateRuntimePool (sizeof (EFI_FW_VOL_BLOCK_DEVICE));
  ASSERT (FvbDevice != NULL);

  CopyMem (FvbDevice, &mFvbDeviceTemplate, sizeof (EFI_FW_VOL_BLOCK_DEVICE));

  FvbDevice->Instance = mFvbModuleGlobal->NumFv;
  mFvbModuleGlobal->NumFv++;

  //
  // Set up the devicepath
  //
  if (FwVolHeader->ExtHeaderOffset == 0) {
    FV_MEMMAP_DEVICE_PATH *FvMemmapDevicePath;

    //
    // FV does not contains extension header, then produce MEMMAP_DEVICE_PATH
    //
    FvMemmapDevicePath = AllocateCopyPool (sizeof (FV_MEMMAP_DEVICE_PATH),
                           &mFvMemmapDevicePathTemplate);
    FvMemmapDevicePath->MemMapDevPath.StartingAddress = BaseAddress;
    FvMemmapDevicePath->MemMapDevPath.EndingAddress   =
      BaseAddress + FwVolHeader->FvLength - 1;
    FvbDevice->DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)FvMemmapDevicePath;
  } else {
    FV_PIWG_DEVICE_PATH *FvPiwgDevicePath;

    FvPiwgDevicePath = AllocateCopyPool (sizeof (FV_PIWG_DEVICE_PATH),
                         &mFvPIWGDevicePathTemplate);
    CopyGuid (
      &FvPiwgDevicePath->FvDevPath.FvName,
      (GUID *)(UINTN)(BaseAddress + FwVolHeader->ExtHeaderOffset)
      );
    FvbDevice->DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)FvPiwgDevicePath;
  }

  //
  // Module type specific hook.
  //
  InstallProtocolInterfaces (FvbDevice);

  MarkMemoryRangeForRuntimeAccess (BaseAddress, Length);

  //
  // Set several PCD values to point to flash
  //
  Status = PcdSet64S (
    PcdFlashNvStorageVariableBase64,
    (UINTN) PcdGet32 (PcdPlatformFlashNvStorageVariableBase)
    );
  ASSERT_EFI_ERROR (Status);
  Status = PcdSet32S (
    PcdFlashNvStorageFtwWorkingBase,
    PcdGet32 (PcdPlatformFlashNvStorageFtwWorkingBase)
    );
  ASSERT_EFI_ERROR (Status);
  Status = PcdSet32S (
    PcdFlashNvStorageFtwSpareBase,
    PcdGet32 (PcdPlatformFlashNvStorageFtwSpareBase)
    );
  ASSERT_EFI_ERROR (Status);

  FwhInstance = (EFI_FW_VOL_INSTANCE *)
    (
      (UINTN) FwhInstance + FwVolHeader->HeaderLength +
      (sizeof (EFI_FW_VOL_INSTANCE) - sizeof (EFI_FIRMWARE_VOLUME_HEADER))
    );

  //
  // Module type specific hook.
  //
  InstallVirtualAddressChangeHandler ();
  return EFI_SUCCESS;
}