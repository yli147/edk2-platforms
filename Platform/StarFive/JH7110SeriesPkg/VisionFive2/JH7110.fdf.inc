## @file
#  Definitions of Flash definition file on StarFive VisionFive 2 platform
#
#  Copyright (c) 2023, StarFive Technology Co., Ltd. All rights reserved.<BR>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
DEFINE BLOCK_SIZE        = 0x1000

DEFINE FW_BASE_ADDRESS   = 0x40200000
DEFINE FW_SIZE           = 0x00900000
DEFINE FW_BLOCKS         = 0x900

DEFINE FVMAIN_OFFSET                 = 0x00000000
DEFINE FVMAIN_SIZE                   = 0x00280000

#
# EFI Variable memory region.
# The total size of EFI Variable FD must include
# all of sub regions of EFI Variable
#
DEFINE VARS_OFFSET                   = 0x00380000
DEFINE VARS_OFFSET                   = 0x00800000
DEFINE VARS_SIZE                     = 0x00010000
DEFINE VARS_FTW_WORKING_OFFSET       = 0x00810000
DEFINE VARS_FTW_WORKING_SIZE         = 0x00010000
DEFINE VARS_FTW_SPARE_OFFSET         = 0x00820000
DEFINE VARS_FTW_SPARE_SIZE           = 0x00010000

#
# Device Tree memory region
#
DEFINE DTB_OFFSET                    = 0x00840000
DEFINE DTB_SIZE                      = 0x00010000

SET gUefiOvmfPkgTokenSpaceGuid.PcdOvmfSecPeiTempRamBase = 0x43FF0000
SET gUefiOvmfPkgTokenSpaceGuid.PcdOvmfSecPeiTempRamSize = 0x00010000

SET gUefiCpuPkgTokenSpaceGuid.PcdCpuCoreCrystalClockFrequency = 24000000 # 24Ghz
SET gUefiRiscVPlatformPkgTokenSpaceGuid.PcdBootHartId         = 1        # Boot hart ID
