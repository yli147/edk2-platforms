# Overview

This directory holds the reference UEFI implementation which intends to port the RiscVVirt bootflow to VisionFiveV2 machine
the code is based on original StarFive's implementation. The RiscVVirt bootflow chucks all M-Mode code from Tiano,
same model as adopted by Tiano on AArch64.PEI serves no role in systems where DDR init is traditionally done in/before SPL.
The flow is also same flow as U-Boot, has no impact to existing code, can share between U-Boot and Tiano payloads.
Like U-Boot, everything is self-configuring via FDT, with no hardcoded config in DSC / headers.
The reference solution also introduced PrePI lib to replace original SEC and PEI as DXE IPL. 
The PrePI use FDT to initilziae the memory and platform vironments in EDK2, the decompress the FV and build HOBs.

![image](https://github.com/yli147/edk2-platforms/assets/21300636/c84a78fb-9a67-49fa-81ca-877fda847a51)


# How to build (Linux Environment)

## Manual building

### 1. Compile EDK2
   ```
   git clone https://github.com/starfive-tech/edk2.git
   git clone https://github.com/yli147.edk2-platforms.git edk2-platforms -b vf2_jh7110_devel
   pushd edk2
   git submodule update --init
   popd
   pushd edk2-platforms
   git submodule update --init
   popd
   export WORKSPACE=$PWD
   export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/edk2-platforms
   export GCC5_RISCV64_PREFIX=/usr/bin/riscv64-linux-gnu-
   pushd edk2
   source ./edksetup.sh
   make -C ./BaseTools
   popd
   build -a RISCV64 -t GCC5 -p ./edk2-platforms/Platform/StarFive/JH7110SeriesPkg/JH7110Board/JH7110.dsc -D FW_BASE_ADDRESS=0x40200000
   ```
### 2. Compile U-Boot（to get the dtb file） and OpenSBI

   ```
   git clone https://github.com/starfive-tech/u-boot.git u-boot
   pushd u-boot
   make -C ./ O=./work starfive_visionfive2_defconfig
   make -C ./ O=./work CROSS_COMPILE=riscv64-linux-gnu-
   popd
   git clone https://github.com/riscv-software-src/opensbi.git opensbi
   pushd opensbi
   CFLAGS="-mabi=lp64d -march=rv64imafdc" make CROSS_COMPILE=riscv64-linux-gnu- PLATFORM=generic FW_PAYLOAD_PATH=../Build/JH7110/DEBUG_GCC5/FV/JH7110.fd FW_FDT_PATH=../u-boot/work/arch/riscv/dts/starfive_visionfive2.dtb FW_TEXT_START=0x40000000
   popd
   ```
  
### 3. Flash the OpenSBI and EDK2 Firmware to SDCard

   ```
   wget -c https://github.com/starfive-tech/edk2/releases/download/REL_VF2_APR2023/u-boot-spl.bin.normal.out
   sudo sgdisk -g --clear --set-alignment=1 \
		--new=1:4096:+2M: --change-name=1:'spl' --typecode=1:2e54b353-1271-4842-806f-e436d6af6985 \
		--new=2:8192:+12M: --change-name=2:'opensbi-uboot' --typecode=2:5b193300-fc78-40cd-8002-e86c45580b47 \
		--new=3:32768:221184 --change-name=3:'linux' --typecode=3:C12A7328-F81F-11D2-BA4B-00A0C93EC93B \
		/dev/sdb
   sudo dd if=u-boot-spl.bin.normal.out of=/dev/sdb1 bs=4096 oflag=direct
   sudo dd if=opensbi/build/platform/generic/firmware/fw_payload.bin of=/dev/sdb2 bs=4096 oflag=direct
   sync
   ```

### Power ON the VF2 and Boot from SDCard
```
U-Boot SPL 2021.10-dirty (Apr 06 2023 - 09:01:05 +0800)
DDR version: dc2e84f0.
Trying to boot from MMC2

OpenSBI v1.2-151-g355796c
   ____                    _____ ____ _____
  / __ \                  / ____|  _ \_   _|
 | |  | |_ __   ___ _ __ | (___ | |_) || |
 | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
 | |__| | |_) |  __/ | | |____) | |_) || |_
  \____/| .__/ \___|_| |_|_____/|___/_____|
        | |
        |_|

Platform Name             : StarFive VisionFive V2
Platform Features         : medeleg
Platform HART Count       : 5
Platform IPI Device       : aclint-mswi
Platform Timer Device     : aclint-mtimer @ 4000000Hz
Platform Console Device   : uart8250
Platform HSM Device       : ---
Platform PMU Device       : ---
Platform Reboot Device    : pm-reset
Platform Shutdown Device  : pm-reset
Platform Suspend Device   : ---
Platform CPPC Device      : ---
Firmware Base             : 0x40000000
Firmware Size             : 362 KB
Firmware RW Offset        : 0x40000
Firmware RW Size          : 106 KB
Firmware Heap Offset      : 0x50000
Firmware Heap Size        : 42 KB (total), 2 KB (reserved), 9 KB (used), 30 KB (free)
Firmware Scratch Size     : 4096 B (total), 760 B (used), 3336 B (free)
Runtime SBI Version       : 1.0

Domain0 Name              : root
Domain0 Boot HART         : 1
Domain0 HARTs             : 0*,1*,2*,3*,4*
Domain0 Region00          : 0x0000000002000000-0x000000000200ffff M: (I,R,W) S/U: ()
Domain0 Region01          : 0x0000000040040000-0x000000004005ffff M: (R,W) S/U: ()
Domain0 Region02          : 0x0000000040000000-0x000000004003ffff M: (R,X) S/U: ()
Domain0 Region03          : 0x0000000000000000-0xffffffffffffffff M: (R,W,X) S/U: (R,W,X)
Domain0 Next Address      : 0x0000000040200000
Domain0 Next Arg1         : 0x0000000042200000
Domain0 Next Mode         : S-mode
Domain0 SysReset          : yes
Domain0 SysSuspend        : yes

Boot HART ID              : 1
Boot HART Domain          : root
Boot HART Priv Version    : v1.11
Boot HART Base ISA        : rv64imafdcbx
Boot HART ISA Extensions  : none
Boot HART PMP Count       : 8
Boot HART PMP Granularity : 4096
Boot HART PMP Address Bits: 34
Boot HART MHPM Count      : 2
Boot HART MIDELEG         : 0x0000000000000222
Boot HART MEDELEG         : 0x000000000000b109
SecStartup() BootHartId: 0x1, DeviceTreeAddress=0x42200000
MemoryPeimInitialization: System RAM @ 0x40000000 - 0x13FFFFFFF
InitializeRamRegions: M-mode FW Memory Start:0x40040000 End:0x40060000
InitMmu: Set Supervisor address mode to bare-metal mode.
PlatformPeimInitialization: Build FDT HOB - FDT at address: 0x42200000
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading DxeCore at 0x0043533000 EntryPoint=0x0043533240
CoreInitializeMemoryServices:
  BaseAddress - 0x44000000 Length - 0xFC000000 MinimalMemorySizeNeeded - 0x10000
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 4356C3A0
ProtectUefiImageCommon - 0x4356C3A0
  - 0x0000000043533000 - 0x000000000003E000
DxeMain: MemoryBaseAddress=0x44000000 MemoryLength=0xFC000000
HOBLIST address in DXE = 0x13FFFC018
Memory Allocation 0x00000004 0x43FE4000 - 0x43FEFFFF
Memory Allocation 0x00000004 0x43FF0000 - 0x43FFFFFF
Memory Allocation 0x00000004 0x43FD4000 - 0x43FE3FFF
Memory Allocation 0x00000004 0x43AA2000 - 0x43FD3FFF
Memory Allocation 0x00000004 0x43571000 - 0x43AA1FFF
Memory Allocation 0x00000003 0x43533000 - 0x43570FFF
Memory Allocation 0x00000003 0x43533000 - 0x43570FFF
FV Hob            0x40200000 - 0x4031FFFF
FV Hob            0x43571000 - 0x43AA0FFF
FV2 Hob           0x43571000 - 0x43AA0FFF
                  00000000-0000-0000-0000-000000000000 - 9E21FD93-9C72-4C15-8C4B-E77F1DB2D792
InstallProtocolInterface: D8117CFE-94A6-11D4-9A3A-0090273FC14D 4356C350
InstallProtocolInterface: 8F644FA9-E850-4DB1-9CE2-0B44698E8DA4 13FFF87B0
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13FFF8A98
InstallProtocolInterface: 8F644FA9-E850-4DB1-9CE2-0B44698E8DA4 13FFF85B0
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13FFF5F98
InstallProtocolInterface: 220E73B6-6BDB-4413-8405-B974B108619A 13FFF8230
InstallProtocolInterface: 220E73B6-6BDB-4413-8405-B974B108619A 13FFF5730
InstallProtocolInterface: EE4E5898-3914-4259-9D6E-DC7BD79403CF 4356C370
CoreProcessFvImageFile() FV at 0x3F19D018, FvAlignment required is 0x1000
InstallProtocolInterface: 8F644FA9-E850-4DB1-9CE2-0B44698E8DA4 13FF05B30
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13FF05E98
InstallProtocolInterface: 220E73B6-6BDB-4413-8405-B974B108619A 13FC012B0
Loading driver 9B680FCE-AD6B-4F3A-B60B-F59899003443
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F271C40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F247000 EntryPoint=0x0013F24728A DevicePathDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F271A98
ProtectUefiImageCommon - 0x3F271C40
  - 0x000000013F247000 - 0x0000000000014B40
InstallProtocolInterface: 0379BE4E-D706-437D-B037-EDB82FB772A4 13F25A808
InstallProtocolInterface: 8B843E20-8132-4852-90CC-551A4E4A7F1C 13F25A848
InstallProtocolInterface: 05C99A21-C70F-4AD2-8A5F-35DF3343F51E 13F25A858
Loading driver 80CF7257-87AB-47F9-A3FE-D50B76D89541
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F2712C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F23A000 EntryPoint=0x0013F23A28A PcdDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F263F98
ProtectUefiImageCommon - 0x3F2712C0
  - 0x000000013F23A000 - 0x000000000000C1C0
InstallProtocolInterface: 11B34006-D85B-4D0A-A290-D5A571310EF7 13F2458A0
InstallProtocolInterface: 13A3F0F6-264A-3EF0-F2E0-DEC512342F34 13F245998
InstallProtocolInterface: 5BE40F57-FA68-4610-BBBF-E9C5FCDAD365 13F245A28
InstallProtocolInterface: FD0F4478-0EFD-461D-BA2D-E58C45FD5F5E 13F245A40
Loading driver B04036D3-4C60-43D6-9850-0FCC090FF054
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F267040
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0609/Build/JH7110/DEBUG_GCC5/RISCV64/Platform/StarFive/JH7110SeriesPkg/Universal/Dxe/RamFvbServicesRuntimeDxe/FvbServicesRuntimeDxe/DEBUG/FvbServicesRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F231000 EntryPoint=0x0013F23128A FvbServicesRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F267E18
ProtectUefiImageCommon - 0x3F267040
  - 0x000000013F231000 - 0x0000000000008C40
FvbInitialize(): BaseAddress: 0x40580000 Length:0x20000
ValidateFvHeader() return failed
Variable FV header is not valid. It will be reinitialized.
Installing RAM FVB
InstallProtocolInterface: 8F644FA9-E850-4DB1-9CE2-0B44698E8DA4 13FFFE8B0
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13F267A98
Loading driver 9B680FCE-AD6B-4F3A-B60B-F59899003443
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F2673C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F207000 EntryPoint=0x0013F20728A DevicePathDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F267698
ProtectUefiImageCommon - 0x3F2673C0
  - 0x000000013F207000 - 0x0000000000014B40
InstallProtocolInterface: 0379BE4E-D706-437D-B037-EDB82FB772A4 13F21A808
InstallProtocolInterface: 8B843E20-8132-4852-90CC-551A4E4A7F1C 13F21A848
InstallProtocolInterface: 05C99A21-C70F-4AD2-8A5F-35DF3343F51E 13F21A858
Loading driver 80CF7257-87AB-47F9-A3FE-D50B76D89541
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F2700C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F1FA000 EntryPoint=0x0013F1FA28A PcdDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F270C18
ProtectUefiImageCommon - 0x3F2700C0
  - 0x000000013F1FA000 - 0x000000000000C1C0
ASSERT [PcdDxe] /ssd/workspace/visionfivev2/0609/edk2/MdeModulePkg/Universal/PCD/Dxe/Pcd.c(133): &gPcdProtocolGuid already installed in database
```
