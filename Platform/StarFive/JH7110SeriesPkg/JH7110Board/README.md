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
   git clone https://github.com/yli147.edk2.git edk2 -b vf2_jh7110_devel
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

### 3. Use Jtag run the OpenSBI and EDK2 Firmware in DRAM
   ```
   https://github.com/starfive-tech/edk2/wiki/How-to-flash-and-debug-with-JTAG
   ```

### 4. Flash or Use Jtag to the OpenSBI and EDK2 Firmware to SDCard
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

### Power ON the VF2 and Boot from DRAM or SDCard 
(*Currently boot from DRAM works, SDCard still have issue)
```

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
Loading DxeCore at 0x0043594000 EntryPoint=0x0043594240
CoreInitializeMemoryServices:
  BaseAddress - 0x44000000 Length - 0xFC000000 MinimalMemorySizeNeeded - 0x10000
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 435CCCE0
ProtectUefiImageCommon - 0x435CCCE0
  - 0x0000000043594000 - 0x000000000003D000
DxeMain: MemoryBaseAddress=0x44000000 MemoryLength=0xFC000000
HOBLIST address in DXE = 0x13FFFC018
Memory Allocation 0x00000004 0x43FE4000 - 0x43FEFFFF
Memory Allocation 0x00000004 0x43FF0000 - 0x43FFFFFF
Memory Allocation 0x00000004 0x43FD4000 - 0x43FE3FFF
Memory Allocation 0x00000004 0x43AD2000 - 0x43FD3FFF
Memory Allocation 0x00000004 0x435D1000 - 0x43AD1FFF
Memory Allocation 0x00000003 0x43594000 - 0x435D0FFF
Memory Allocation 0x00000003 0x43594000 - 0x435D0FFF
FV Hob            0x40200000 - 0x4031FFFF
FV Hob            0x435D1000 - 0x43AD0FFF
FV2 Hob           0x435D1000 - 0x43AD0FFF
                  00000000-0000-0000-0000-000000000000 - 9E21FD93-9C72-4C15-8C4B-E77F1DB2D792
InstallProtocolInterface: D8117CFE-94A6-11D4-9A3A-0090273FC14D 435CCC90
InstallProtocolInterface: 8F644FA9-E850-4DB1-9CE2-0B44698E8DA4 13FFF87B0
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13FFF8A98
InstallProtocolInterface: 8F644FA9-E850-4DB1-9CE2-0B44698E8DA4 13FFF85B0
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13FFF5F98
#### EfiCreateProtocolNotifyEvent  DEBUG 0
InstallProtocolInterface: 220E73B6-6BDB-4413-8405-B974B108619A 13FFF8230
InstallProtocolInterface: 220E73B6-6BDB-4413-8405-B974B108619A 13FFF5730
InstallProtocolInterface: EE4E5898-3914-4259-9D6E-DC7BD79403CF 435CCCB0
#### EfiCreateProtocolNotifyEvent  DEBUG 0
Loading driver 9B680FCE-AD6B-4F3A-B60B-F59899003443
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13FC021C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F64E000 EntryPoint=0x0013F64E28A DevicePathDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13FC02598
ProtectUefiImageCommon - 0x3FC021C0
  - 0x000000013F64E000 - 0x0000000000014B40
InstallProtocolInterface: 0379BE4E-D706-437D-B037-EDB82FB772A4 13F661808
InstallProtocolInterface: 8B843E20-8132-4852-90CC-551A4E4A7F1C 13F661848
InstallProtocolInterface: 05C99A21-C70F-4AD2-8A5F-35DF3343F51E 13F661858
Loading driver 80CF7257-87AB-47F9-A3FE-D50B76D89541
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13FC010C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F641000 EntryPoint=0x0013F64128A PcdDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13FC01318
ProtectUefiImageCommon - 0x3FC010C0
  - 0x000000013F641000 - 0x000000000000C200
InstallProtocolInterface: 11B34006-D85B-4D0A-A290-D5A571310EF7 13F64C8E0
InstallProtocolInterface: 13A3F0F6-264A-3EF0-F2E0-DEC512342F34 13F64C9D8
InstallProtocolInterface: 5BE40F57-FA68-4610-BBBF-E9C5FCDAD365 13F64CA68
InstallProtocolInterface: FD0F4478-0EFD-461D-BA2D-E58C45FD5F5E 13F64CA80
#### EfiCreateProtocolNotifyEvent  DEBUG 0
Loading driver B04036D3-4C60-43D6-9850-0FCC090FF054
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F677240
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/Platform/StarFive/JH7110SeriesPkg/Universal/Dxe/RamFvbServicesRuntimeDxe/FvbServicesRuntimeDxe/DEBUG/FvbServicesRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F665000 EntryPoint=0x0013F66528A FvbServicesRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F677818
ProtectUefiImageCommon - 0x3F677240
  - 0x000000013F665000 - 0x0000000000008C80
FvbInitialize(): BaseAddress: 0x40580000 Length:0x20000
ValidateFvHeader() return ok
Installing RAM FVB
InstallProtocolInterface: 8F644FA9-E850-4DB1-9CE2-0B44698E8DA4 13FFFE8B0
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13F676F98
Loading driver D93CE3D8-A7EB-4730-8C8E-CC466A9ECC3C
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F676140
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe/DEBUG/ReportStatusCodeRouterRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F631000 EntryPoint=0x0013F63128A ReportStatusCodeRouterRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F676C18
ProtectUefiImageCommon - 0x3F676140
  - 0x000000013F631000 - 0x00000000000078C0
InstallProtocolInterface: 86212936-0E76-41C8-A03A-2AF2FC1C39E2 13F6382A0
InstallProtocolInterface: D2B2B828-0826-48A7-B3DF-983C006024F0 13F638298
Loading driver B601F8C4-43B7-4784-95B1-F4226CB40CEE
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F671040
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/MdeModulePkg/Core/RuntimeDxe/RuntimeDxe/DEBUG/RuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F62A000 EntryPoint=0x0013F62A28A RuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F671D18
ProtectUefiImageCommon - 0x3F671040
  - 0x000000013F62A000 - 0x0000000000006D00
InstallProtocolInterface: B7DFB4E1-052F-449F-87BE-9818FC91B733 13F630688
Loading driver F80697E9-7FD6-4665-8646-88E33EF71DFC
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F6713C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F622000 EntryPoint=0x0013F62228A SecurityStubDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F671998
ProtectUefiImageCommon - 0x3F6713C0
  - 0x000000013F622000 - 0x0000000000007B00
InstallProtocolInterface: 94AB2F58-1438-4EF1-9152-18941A3A0E68 13F6294D0
InstallProtocolInterface: A46423E3-4617-49F1-B9FF-D1BFA9115839 13F6294C8
InstallProtocolInterface: 15853D7C-3DDF-43E0-A1CB-EBF85B8F872C 13F6294D8
#### EfiCreateProtocolNotifyEvent  DEBUG 0
Loading driver A19B1FE7-C1BC-49F8-875F-54A5D542443F
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F66E7C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F61C000 EntryPoint=0x0013F61C28A CpuIo2Dxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F66E718
ProtectUefiImageCommon - 0x3F66E7C0
  - 0x000000013F61C000 - 0x0000000000005680
InstallProtocolInterface: AD61F191-AE5F-4C0E-B9FA-E869D288C64F 13F621108
Loading driver C8339973-A563-4561-B858-D8476F9DEFC4
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F63B040
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F617000 EntryPoint=0x0013F61728A Metronome.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F66E418
ProtectUefiImageCommon - 0x3F63B040
  - 0x000000013F617000 - 0x00000000000046C0
InstallProtocolInterface: 26BACCB2-6F42-11D4-BCE7-0080C73C8881 13F61B1C8
Loading driver 2AEB1F3E-5B6B-441B-92C1-4A9E6FC85E92
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F63B9C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F612000 EntryPoint=0x0013F61228A CpuDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F63B798
ProtectUefiImageCommon - 0x3F63B9C0
  - 0x000000013F612000 - 0x0000000000004A00
InstallProtocolInterface: 26BACCB1-6F42-11D4-BCE7-0080C73C8881 13F616488
MemoryProtectionCpuArchProtocolNotify:
ProtectUefiImageCommon - 0x435CCCE0
  - 0x0000000043594000 - 0x000000000003D000
ProtectUefiImageCommon - 0x3FC021C0
  - 0x000000013F64E000 - 0x0000000000014B40
ProtectUefiImageCommon - 0x3FC010C0
  - 0x000000013F641000 - 0x000000000000C200
ProtectUefiImageCommon - 0x3F677240
  - 0x000000013F665000 - 0x0000000000008C80
ProtectUefiImageCommon - 0x3F676140
  - 0x000000013F631000 - 0x00000000000078C0
ProtectUefiImageCommon - 0x3F671040
  - 0x000000013F62A000 - 0x0000000000006D00
ProtectUefiImageCommon - 0x3F6713C0
  - 0x000000013F622000 - 0x0000000000007B00
ProtectUefiImageCommon - 0x3F66E7C0
  - 0x000000013F61C000 - 0x0000000000005680
ProtectUefiImageCommon - 0x3F63B040
  - 0x000000013F617000 - 0x00000000000046C0
ProtectUefiImageCommon - 0x3F63B9C0
  - 0x000000013F612000 - 0x0000000000004A00
Loading driver CBD2E4D5-7068-4FF5-B462-9822B4AD8D60
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F640140
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe/DEBUG/VariableRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F5DE000 EntryPoint=0x0013F5DE28A VariableRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F640898
ProtectUefiImageCommon - 0x3F640140
  - 0x000000013F5DE000 - 0x0000000000019480
VarCheckLibRegisterSetVariableCheckHandler - 0x3F5EF612 Success
Variable driver common space: 0x6F9C 0x6F9C 0x6F9C
Variable driver will work without auth variable support!
InstallProtocolInterface: CD3D0A05-9E24-437C-A891-1EE053DB7638 13F5F5F88
InstallProtocolInterface: AF23B340-97B4-4685-8D4F-A3F28169B21D 13F5F5FC0
InstallProtocolInterface: 1E5668E2-8481-11D4-BCF1-0080C73C8881 0
#### EfiCreateProtocolNotifyEvent  DEBUG 0
VarCheckLibRegisterSetVariableCheckHandler - 0x3F5EE3CE Success
InstallProtocolInterface: 81D1675C-86F6-48DF-BD95-9A6E4F0925C3 13F5F5F90
Loading driver 4B28E4C7-FF36-4E10-93CF-A82159E777C5
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F639840
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe/DEBUG/ResetSystemRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F5CE000 EntryPoint=0x0013F5CE28A ResetSystemRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F639718
ProtectUefiImageCommon - 0x3F639840
  - 0x000000013F5CE000 - 0x0000000000007040
InstallProtocolInterface: 27CFAC88-46CC-11D4-9A38-0090273FC14D 0
InstallProtocolInterface: 9DA34AE0-EAF9-4BBF-8EC3-FD60226C44BE 13F5D49F0
InstallProtocolInterface: 695D7835-8D47-4C11-AB22-FA8ACCE7AE7A 13F5D4A18
InstallProtocolInterface: 2DF6BA0B-7092-440D-BD04-FB091EC3F3C1 13F5D4A40
Loading driver 79E4A61C-ED73-4312-94FE-E3E7563362A9
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5F8140
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F5C6000 EntryPoint=0x0013F5C628A PrintDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5F8B18
ProtectUefiImageCommon - 0x3F5F8140
  - 0x000000013F5C6000 - 0x0000000000005280
InstallProtocolInterface: F05976EF-83F1-4F3D-8619-F7595D41E538 13F5CAC88
InstallProtocolInterface: 0CC252D2-C106-4661-B5BD-3147A4F81F92 13F5CACD8
Loading driver 348C4D62-BFBD-4882-9ECE-C80BB1C4783B
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5DD040
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F566000 EntryPoint=0x0013F56628A HiiDatabase.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5DDD18
ProtectUefiImageCommon - 0x3F5DD040
  - 0x000000013F566000 - 0x000000000002F880
InstallProtocolInterface: E9CA4775-8657-47FC-97E7-7ED65A084324 13F593C30
InstallProtocolInterface: 0FD96974-23AA-4CDC-B9CB-98D17750322A 13F593CA8
InstallProtocolInterface: EF9FC172-A1B2-4693-B327-6D32FC416042 13F593CD0
InstallProtocolInterface: 587E72D7-CC50-4F79-8209-CA291FC1A10F 13F593D28
InstallProtocolInterface: 0A8BADD5-03B8-4D19-B128-7B8F0EDAA596 13F593D58
InstallProtocolInterface: 31A6406A-6BDF-4E46-B2A2-EBAA89C40920 13F593C50
InstallProtocolInterface: 1A1241E6-8F19-41A9-BC0E-E8EF39E06546 13F593C78
Loading driver 96B5C032-DF4C-4B6E-8232-438DCF448D0E
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5D6040
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F5C0000 EntryPoint=0x0013F5C028A NullMemoryTestDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5D6D98
ProtectUefiImageCommon - 0x3F5D6040
  - 0x000000013F5C0000 - 0x0000000000005600
InstallProtocolInterface: 309DE7F1-7F5E-4ACE-B49C-531BE5AA95EF 13F5C5048
Loading driver A7D8F3F7-D8A7-47DF-B3EC-9E5A693C380C
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5D6340
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F5AE000 EntryPoint=0x0013F5AE28A FdtDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5D6918
ProtectUefiImageCommon - 0x3F5D6340
  - 0x000000013F5AE000 - 0x00000000000080C0
Fixing up device tree with boot hart id: 1
Loading driver 9A5163E7-5C29-453F-825C-837A46A81E15
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5DCB40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F5BB000 EntryPoint=0x0013F5BB28A SerialDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5DCE18
ProtectUefiImageCommon - 0x3F5DCB40
  - 0x000000013F5BB000 - 0x0000000000004BC0
InstallProtocolInterface: BB25CF6F-F1D4-11D2-9A0C-0090273FC1FD 13F5BF698
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13F5BF648
Loading driver F9D88642-0737-49BC-81B5-6889CD57D9EA
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5DC1C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F598000 EntryPoint=0x0013F59828A SmbiosDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5DC498
ProtectUefiImageCommon - 0x3F5DC1C0
  - 0x000000013F598000 - 0x000000000000AA80
InstallProtocolInterface: 03583FF6-CB36-4940-947E-B9B39F4AFAF7 13F5A2378
Loading driver A210F973-229D-4F4D-AA37-9895E6C9EABA
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5DB0C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F5A4000 EntryPoint=0x0013F5A428A DpcDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5DBC98
ProtectUefiImageCommon - 0x3F5DB0C0
  - 0x000000013F5A4000 - 0x0000000000004FC0
InstallProtocolInterface: 480F8AE9-0C46-4AA9-BC89-DB9FBA619806 13F5A88C8
Loading driver 6C2004EF-4E0E-4BE4-B14C-340EB4AA5891
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5DB440
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe/DEBUG/StatusCodeHandlerRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F558000 EntryPoint=0x0013F55828A StatusCodeHandlerRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5DAB18
ProtectUefiImageCommon - 0x3F5DB440
  - 0x000000013F558000 - 0x0000000000006C40
PROGRESS CODE: V03040003 I0
Loading driver B336F62D-4135-4A55-AE4E-4971BBF0885D
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5DA1C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe/DEBUG/RealTimeClock.dll  !!!!!!!!
Loading driver at 0x0013F551000 EntryPoint=0x0013F55128A RealTimeClock.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5DAA18
ProtectUefiImageCommon - 0x3F5DA1C0
  - 0x000000013F551000 - 0x0000000000006180
PROGRESS CODE: V03040002 I0
InitializeRealTimeClock: using default timezone/daylight settings
InstallProtocolInterface: 27CFAC87-46CC-11D4-9A38-0090273FC14D 0
PROGRESS CODE: V03040003 I0
Loading driver 3F75D495-23FF-46B6-9D19-0DECC8A4EA91
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5D7B40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F561000 EntryPoint=0x0013F56128A Timer.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5D7E98
ProtectUefiImageCommon - 0x3F5D7B40
  - 0x000000013F561000 - 0x0000000000004C00
PROGRESS CODE: V03040002 I0
TimerDriverSetTimerPeriod(0x0)
RegisterCpuInterruptHandler: Type:1 Handler: 3F561514
TimerDriverSetTimerPeriod(0x186A0)
InstallProtocolInterface: 26BACCB3-6F42-11D4-BCE7-0080C73C8881 13F5656C8
TimerDriverRegisterHandler(0x435B1CF2) called
PROGRESS CODE: V03040003 I0
Loading driver 5FC01647-AADD-42E1-AD99-DF4CB89F5A92
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5D7140
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F53F000 EntryPoint=0x0013F53F28A RiscVSmbiosDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5D7418
ProtectUefiImageCommon - 0x3F5D7140
  - 0x000000013F53F000 - 0x00000000000080C0
PROGRESS CODE: V03040002 I0
RiscVSmbiosBuilderEntry: entry
No RISC-V SMBIOS information found.
Error: Image at 0013F53F000 start failed: Not Found
PROGRESS CODE: V03040003 I0
Loading driver FE5CEA76-4F72-49E8-986F-2CD899DFFE5D
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5D7140
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F539000 EntryPoint=0x0013F53928A FaultTolerantWriteDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5D7718
ProtectUefiImageCommon - 0x3F5D7140
  - 0x000000013F539000 - 0x000000000000BC80
PROGRESS CODE: V03040002 I0
#### EfiCreateProtocolNotifyEvent  DEBUG 0
Ftw: FtwWorkSpaceLba - 0x7, WorkBlockSize  - 0x1000, FtwWorkSpaceBase - 0x0
Ftw: FtwSpareLba     - 0x8, SpareBlockSize - 0x1000
Ftw: NumberOfWorkBlock - 0x1, FtwWorkBlockLba - 0x7
Ftw: WorkSpaceLbaInSpare - 0x0, WorkSpaceBaseInSpare - 0x0
Ftw: Remaining work space size - FE0
InstallProtocolInterface: 3EBD9E82-2C78-4DE6-9786-8D4BFCB7C881 13F5AC028
RecordSecureBootPolicyVarData GetVariable SecureBoot Status E
InstallProtocolInterface: 6441F818-6362-4E44-B570-7DBA31DD2453 0
PROGRESS CODE: V03040003 I0
Loading driver 6D33944A-EC75-4855-A54D-809C75241F6C
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5B80C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F4FB000 EntryPoint=0x0013F4FB28A BdsDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5B8E18
ProtectUefiImageCommon - 0x3F5B80C0
  - 0x000000013F4FB000 - 0x000000000001EEC0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 665E3FF6-46CC-11D4-9A38-0090273FC14D 13F518F08
PROGRESS CODE: V03040003 I0
Loading driver EBF342FE-B1D3-4EF8-957C-8048606FF671
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5B8540
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F4A9000 EntryPoint=0x0013F4A928A SetupBrowser.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5B7F98
ProtectUefiImageCommon - 0x3F5B8540
  - 0x000000013F4A9000 - 0x0000000000028780
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: B9D4C360-BCFB-4F9B-9298-53C136982258 13F4CFC98
InstallProtocolInterface: A770C357-B693-4E6D-A6CF-D21C728E550B 13F4CFCC8
InstallProtocolInterface: 1F73B18D-4630-43C1-A1DE-6F80855D7DA4 13F4CFCA8
#### EfiCreateProtocolNotifyEvent  DEBUG 0
PROGRESS CODE: V03040003 I0
Loading driver 2F30DA26-F51B-4B6F-85C4-31873C281BCA
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5B7240
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F51B000 EntryPoint=0x0013F51B28A LinuxInitrdDynamicShellCommand.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5B7598
InstallProtocolInterface: 6A1EE763-D47A-43B4-AABE-EF1DE2AB56FC 13F528870
ProtectUefiImageCommon - 0x3F5B7240
  - 0x000000013F51B000 - 0x000000000000E9C0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 3C7200E9-005F-4EA4-87DE-A3DFAC8A27C3 13F5285F0
PROGRESS CODE: V03040003 I0
Loading driver F099D67F-71AE-4C36-B2A3-DCEB0EB2B7D8
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5AB7C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F547000 EntryPoint=0x0013F54728A WatchdogTimer.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5AAB18
ProtectUefiImageCommon - 0x3F5AB7C0
  - 0x000000013F547000 - 0x0000000000004D40
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 665E3FF5-46CC-11D4-9A38-0090273FC14D 13F54B808
PROGRESS CODE: V03040003 I0
Loading driver AD608272-D07F-4964-801E-7BD3B7888652
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5AB2C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe/DEBUG/MonotonicCounterRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F52D000 EntryPoint=0x0013F52D28A MonotonicCounterRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5A9F98
ProtectUefiImageCommon - 0x3F5AB2C0
  - 0x000000013F52D000 - 0x0000000000005140
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 1DA97072-BDDC-4B30-99F1-72A0B56FFF2A 0
PROGRESS CODE: V03040003 I0
Loading driver 42857F0A-13F2-4B21-8A23-53D3F714B840
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5A91C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
!!!!!!!!  InsertImageRecord - Section Alignment(0x40) is not 4K  !!!!!!!!
!!!!!!!!  Image - /ssd/workspace/visionfivev2/0615/Build/JH7110/DEBUG_GCC5/RISCV64/MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe/DEBUG/CapsuleRuntimeDxe.dll  !!!!!!!!
Loading driver at 0x0013F4F5000 EntryPoint=0x0013F4F528A CapsuleRuntimeDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F5A9418
ProtectUefiImageCommon - 0x3F5A91C0
  - 0x000000013F4F5000 - 0x0000000000005E80
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 5053697E-2CBC-4819-90D9-0580DEEE5754 0
PROGRESS CODE: V03040003 I0
Loading driver E660EA85-058E-4B55-A54B-F02F83A24707
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F550C40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F487000 EntryPoint=0x0013F48728A DisplayEngine.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F550A98
ProtectUefiImageCommon - 0x3F550C40
  - 0x000000013F487000 - 0x0000000000021280
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 9BBE29E9-FDA1-41EC-AD52-452213742D2E 13F4A6708
InstallProtocolInterface: 4311EDC0-6054-46D4-9E40-893EA952FCCC 13F4A6720
PROGRESS CODE: V03040003 I0
Loading driver 93B80004-9FB3-11D4-9A3A-0090273FC14D
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F54C1C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F469000 EntryPoint=0x0013F46928A PciBusDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F550918
ProtectUefiImageCommon - 0x3F54C1C0
  - 0x000000013F469000 - 0x000000000001D540
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F485588
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F4855D0
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F4855E8
InstallProtocolInterface: 19CB87AB-2CB9-4665-8360-DDCF6054F79D 13F4855C8
PROGRESS CODE: V03040003 I0
Loading driver 51CCF399-4FDF-4E55-A45B-E123F84D456A
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5380C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F4E3000 EntryPoint=0x0013F4E328A ConPlatformDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F538A18
ProtectUefiImageCommon - 0x3F5380C0
  - 0x000000013F4E3000 - 0x0000000000008240
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F4EAB48
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F4EABA8
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F4EABC0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F4EAB78
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F4EABA8
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F4EABC0
PROGRESS CODE: V03040003 I0
Loading driver 408EDCEC-CF6D-477C-A5A8-B4844E3DE281
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F537040
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F45A000 EntryPoint=0x0013F45A28A ConSplitterDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F537F18
ProtectUefiImageCommon - 0x3F537040
  - 0x000000013F45A000 - 0x000000000000E380
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F4677F0
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F4678E0
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F4678F8
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F467880
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F467910
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F467928
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F4678B0
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F467940
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F467958
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F467820
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F467970
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F467988
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F467850
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F4679A0
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F4679B8
InstallProtocolInterface: 387477C1-69C7-11D2-8E39-00A0C969723B 13F467420
InstallProtocolInterface: DD9E7534-7762-4698-8C14-F58517A625AA 13F467450
InstallProtocolInterface: 31878C87-0B75-11D5-9A4F-0090273FC14D 13F4674C0
InstallProtocolInterface: 8D59D32B-C655-4AE9-9B15-F25904992A43 13F467518
InstallProtocolInterface: 387477C2-69C7-11D2-8E39-00A0C969723B 13F4675E0
InstallProtocolInterface: 387477C2-69C7-11D2-8E39-00A0C969723B 13F4676F0
PROGRESS CODE: V03040003 I0
Loading driver 9E863906-A40F-4875-977F-5B93FF237FC6
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F5344C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F44A000 EntryPoint=0x0013F44A28A TerminalDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F534A18
ProtectUefiImageCommon - 0x3F5344C0
  - 0x000000013F44A000 - 0x000000000000F780
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F458688
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F4588D0
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F4588E8
PROGRESS CODE: V03040003 I0
Loading driver 6B38F7B4-AD98-40E9-9093-ACA2B5A253C4
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F533B40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F440000 EntryPoint=0x0013F44028A DiskIoDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F533E18
ProtectUefiImageCommon - 0x3F533B40
  - 0x000000013F440000 - 0x0000000000009440
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F448D48
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F448E00
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F448E18
PROGRESS CODE: V03040003 I0
Loading driver 1FA1F39E-FEFF-4AAE-BD7B-38A070A3B609
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F533140
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F433000 EntryPoint=0x0013F43328A PartitionDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F533718
ProtectUefiImageCommon - 0x3F533140
  - 0x000000013F433000 - 0x000000000000CCC0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F43F448
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F43F498
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F43F4B0
PROGRESS CODE: V03040003 I0
Loading driver CD3BAFB6-50FB-4FE8-8E4E-AB74D2C1A600
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F52CB40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F4DD000 EntryPoint=0x0013F4DD28A EnglishDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F52C098
ProtectUefiImageCommon - 0x3F52CB40
  - 0x000000013F4DD000 - 0x0000000000005200
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 1D85CD7F-F43D-11D2-9A0C-0090273FC14D 13F4E1928
InstallProtocolInterface: A4C751FC-23AE-4C3E-92E9-4964CF63F349 13F4E1960
PROGRESS CODE: V03040003 I0
Loading driver 0167CCC4-D0F7-4F21-A3EF-9E64B7CDCE8B
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F52C1C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F4D4000 EntryPoint=0x0013F4D428A ScsiBus.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F52C498
ProtectUefiImageCommon - 0x3F52C1C0
  - 0x000000013F4D4000 - 0x0000000000008BC0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F4DC548
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F4DC578
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F4DC590
PROGRESS CODE: V03040003 I0
Loading driver 0A66E322-3740-4CCE-AD62-BD172CECCA35
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F52BBC0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F40D000 EntryPoint=0x0013F40D28A ScsiDisk.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F52BA98
ProtectUefiImageCommon - 0x3F52BBC0
  - 0x000000013F40D000 - 0x0000000000012BC0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F41F148
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F41F1A8
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F41F1C0
PROGRESS CODE: V03040003 I0
Loading driver 961578FE-B6B7-44C3-AF35-6BC705CD2B1F
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F52B240
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F3FB000 EntryPoint=0x0013F3FB28A Fat.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F52A018
ProtectUefiImageCommon - 0x3F52B240
  - 0x000000013F3FB000 - 0x0000000000011E80
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F40C488
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F40C4B8
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F40C4D0
PROGRESS CODE: V03040003 I0
Loading driver 905F13B0-8F91-4B0A-BD76-E1E78F9422E4
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F52A140
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F3EE000 EntryPoint=0x0013F3EE28A UdfDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F52AB18
ProtectUefiImageCommon - 0x3F52A140
  - 0x000000013F3EE000 - 0x000000000000C6C0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F3F9E48
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F3F9E88
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F3F9EA0
PROGRESS CODE: V03040003 I0
Loading driver E4F61863-FE2C-4B56-A8F4-08519BC439DF
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F52A540
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F3E0000 EntryPoint=0x0013F3E028A VlanConfigDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F51AF98
ProtectUefiImageCommon - 0x3F52A540
  - 0x000000013F3E0000 - 0x000000000000D200
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F3EC870
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F3ECB10
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F3ECB28
PROGRESS CODE: V03040003 I0
Loading driver 025BBFC7-E6A9-4B8B-82AD-6815A1AEAF4A
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F51A140
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F3B8000 EntryPoint=0x0013F3B828A MnpDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F51A398
ProtectUefiImageCommon - 0x3F51A140
  - 0x000000013F3B8000 - 0x0000000000013100
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F3CA788
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F3CA840
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F3CA858
PROGRESS CODE: V03040003 I0
Loading driver 529D3F93-E8E9-4E73-B1E1-BDF6A9D50113
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F51A540
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F3D3000 EntryPoint=0x0013F3D328A ArpDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4F4F98
ProtectUefiImageCommon - 0x3F51A540
  - 0x000000013F3D3000 - 0x000000000000C280
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F3DEB48
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F3DEB78
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F3DEB90
PROGRESS CODE: V03040003 I0
Loading driver 94734718-0BBC-47FB-96A5-EE7A5AE6A2AD
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4F47C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F390000 EntryPoint=0x0013F39028A Dhcp4Dxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4F4618
ProtectUefiImageCommon - 0x3F4F47C0
  - 0x000000013F390000 - 0x0000000000013CC0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F3A24C8
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F3A2560
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F3A2578
PROGRESS CODE: V03040003 I0
Loading driver 9FB1A1F3-3B71-4324-B39A-745CBB015FFF
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4F2040
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F34C000 EntryPoint=0x0013F34C28A Ip4Dxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4F2D18
ProtectUefiImageCommon - 0x3F4F2040
  - 0x000000013F34C000 - 0x0000000000021540
PROGRESS CODE: V03040002 I0
#### EfiCreateProtocolNotifyEvent  DEBUG 0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F36C538
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F36C5B0
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F36C5C8
PROGRESS CODE: V03040003 I0
Loading driver 6D6963AB-906D-4A65-A7CA-BD40E5D6AF2B
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4F23C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F3A6000 EntryPoint=0x0013F3A628A Udp4Dxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4F2698
ProtectUefiImageCommon - 0x3F4F23C0
  - 0x000000013F3A6000 - 0x00000000000115C0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F3B6B08
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F3B6B48
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F3B6B60
LibGetTime: RtcEpochSeconds non volatile variable was not found - Using compilation time epoch.
PROGRESS CODE: V03040003 I0
Loading driver DC3641B8-2FA8-4ED3-BC1F-F9962A03454B
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4F1B40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F37E000 EntryPoint=0x0013F37E28A Mtftp4Dxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4F1E98
ProtectUefiImageCommon - 0x3F4F1B40
  - 0x000000013F37E000 - 0x0000000000011AC0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F38F088
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F38F0C8
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F38F0E0
PROGRESS CODE: V03040003 I0
Loading driver 1A7E4468-2F55-4A56-903C-01265EB7622B
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4F1240
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F30A000 EntryPoint=0x0013F30A28A TcpDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4F1618
ProtectUefiImageCommon - 0x3F4F1240
  - 0x000000013F30A000 - 0x00000000000204C0
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F329340
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F329478
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F329490
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F329370
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F329478
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F329490
PROGRESS CODE: V03040003 I0
Loading driver B95E9FDA-26DE-48D2-8807-1F9107AC5E3A
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4F08C0
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F2EC000 EntryPoint=0x0013F2EC28A UefiPxeBcDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4F0818
ProtectUefiImageCommon - 0x3F4F08C0
  - 0x000000013F2EC000 - 0x000000000001DF80
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F308ED8
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F309010
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F309028
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F308F08
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F309010
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F309028
PROGRESS CODE: V03040003 I0
Loading driver 2FB92EFA-2EE0-4BAE-9EB6-7464125E1EF7
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4EFC40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F370000 EntryPoint=0x0013F37028A UhciDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4EFB18
ProtectUefiImageCommon - 0x3F4EFC40
  - 0x000000013F370000 - 0x000000000000DB00
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F37D348
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F37D378
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F37D390
PROGRESS CODE: V03040003 I0
Loading driver BDFE430E-8F2A-4DB0-9991-6F856594777E
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4EF240
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F33B000 EntryPoint=0x0013F33B28A EhciDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4EF598
ProtectUefiImageCommon - 0x3F4EF240
  - 0x000000013F33B000 - 0x0000000000010940
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F34B078
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F34B0A8
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F34B0C0
PROGRESS CODE: V03040003 I0
Loading driver B7F50E91-A759-412C-ADE4-DCD03E7F7C28
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4EEB40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F2BA000 EntryPoint=0x0013F2BA28A XhciDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4EEA98
ProtectUefiImageCommon - 0x3F4EEB40
  - 0x000000013F2BA000 - 0x0000000000018680
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F2D1A90
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F2D1B30
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F2D1B48
PROGRESS CODE: V03040003 I0
Loading driver 240612B7-A063-11D4-9A3A-0090273FC14D
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4EE240
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F2DB000 EntryPoint=0x0013F2DB28A UsbBusDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4EE698
ProtectUefiImageCommon - 0x3F4EE240
  - 0x000000013F2DB000 - 0x0000000000010C00
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F2EB270
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F2EB360
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F2EB378
PROGRESS CODE: V03040003 I0
Loading driver 2D2E62CF-9ECF-43B7-8219-94E7FC713DFE
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4EDC40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F32E000 EntryPoint=0x0013F32E28A UsbKbDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4EDB18
ProtectUefiImageCommon - 0x3F4EDC40
  - 0x000000013F32E000 - 0x000000000000CF80
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F339F48
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F33A700
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F33A718
PROGRESS CODE: V03040003 I0
Loading driver 9FB4B4A7-42C0-4BCD-8540-9BCC6711F83E
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F4ED340
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013F2AF000 EntryPoint=0x0013F2AF28A UsbMassStorageDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F4ED718
ProtectUefiImageCommon - 0x3F4ED340
  - 0x000000013F2AF000 - 0x000000000000AC00
PROGRESS CODE: V03040002 I0
InstallProtocolInterface: 18A031AB-B443-4D1A-A5C0-0C09261E9F71 13F2B93A0
InstallProtocolInterface: 107A772C-D5E1-11D4-9A46-0090273FC14D 13F2B9490
InstallProtocolInterface: 6A7A5CFF-E8D9-4F70-BADA-75AB3025CE14 13F2B94A8
PROGRESS CODE: V03040003 I0
PROGRESS CODE: V03041001 I0
[Bds] Entry...
[BdsDxe] Locate Variable Policy protocol - Success
Variable Driver Auto Update Lang, Lang:eng, PlatformLang:en Status: Success
PROGRESS CODE: V03051005 I0
[Variable]END_OF_DXE is signaled
Initialize variable error flag (FF)
Check gPlatformConsole 0
CONSOLE_IN variable set VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenMsg(E0C14753-F9BE-11D2-9A0C-0090273FC14D) : Success
CONSOLE_OUT variable set VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenMsg(E0C14753-F9BE-11D2-9A0C-0090273FC14D) : Success
[Bds]RegisterKeyNotify: 000C/0000 80000000/00 Success
[Bds]RegisterKeyNotify: 0000/000D 80000000/00 Success
Terminal - Mode 0, Column = 80, Row = 25
Terminal - Mode 1, Column = 80, Row = 50
Terminal - Mode 2, Column = 100, Row = 31
PROGRESS CODE: V01040001 I0
InstallProtocolInterface: 387477C1-69C7-11D2-8E39-00A0C969723B 13F4324C0
InstallProtocolInterface: DD9E7534-7762-4698-8C14-F58517A625AA 13F4325A8
InstallProtocolInterface: 387477C2-69C7-11D2-8E39-00A0C969723B 13F4324D8
InstallProtocolInterface: 09576E91-6D3F-11D2-8E39-00A0C969723B 13F432898
InstallProtocolInterface: D3B36F2C-D551-11D4-9A46-0090273FC14D 0
InstallProtocolInterface: D3B36F2B-D551-11D4-9A46-0090273FC14D 0
[Bds]OsIndication: 0000000000000000
[Bds]=============Begin Load Options Dumping ...=============
  Driver Options:
  SysPrep Options:
  Boot Options:
    Boot0000: UiApp              0x0109
    Boot0001: UEFI Shell                 0x0001
  PlatformRecovery Options:
    PlatformRecovery0000: Default PlatformRecovery               0x0001
[Bds]=============End Load Options Dumping=============
[Bds]BdsWait ...Zzzzzzzzzzzz...
[Bds]Exit the waiting!
PROGRESS CODE: V03051007 I0
[Bds]Stop Hotkey Service!
[Bds]UnregisterKeyNotify: 000C/0000 Success
[Bds]UnregisterKeyNotify: 0000/000D Success
PROGRESS CODE: V03051001 I0
Memory  Previous  Current    Next
 Type    Pages     Pages     Pages
======  ========  ========  ========
  09    00000000  00000000  00000000
  0A    00000000  00000000  00000000
  00    00000000  00000000  00000000
  06    00000000  00000056  0000006B
  05    00000000  00000054  00000069
  03    00000000  000002EA  000003A4
  04    00000000  000009EB  00000C65
  01    00000000  00000000  00000000
  02    00000000  00000000  00000000
Memory Type Information settings change.
[Bds]Booting UEFI Shell
[Bds] Expand MemoryMapped(0xB,0x435D1000,0x43AD0FFF)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1) -> MemoryMapped(0xB,0x435D1000,0x43AD0FFF)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
PROGRESS CODE: V03058000 I0
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 13F429D40
InvalidateInstructionCacheRange:RISC-V unsupported function.
Loading driver at 0x0013EF26000 EntryPoint=0x0013EF26240 Shell.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 13F42E518
ProtectUefiImageCommon - 0x3F429D40
  - 0x000000013EF26000 - 0x0000000000123D00
PROGRESS CODE: V03058001 I0
[HiiDatabase]: Memory allocation is required after ReadyToBoot, which may change memory map and cause S4 resume issue.
[HiiDatabase]: Memory allocation is required after ReadyToBoot, which may change memory map and cause S4 resume issue.
[HiiDatabase]: Memory allocation is required after ReadyToBoot, which may change memory map and cause S4 resume issue.
[HiiDatabase]: Memory allocation is required after ReadyToBoot, which may change memory map and cause S4 resume issue.
[HiiDatabase]: Memory allocation is required after ReadyToBoot, which may change memory map and cause S4 resume issue.
UEFI Interactive Shell v2.287477C2-69C7-11D2-8E39-00A0C969723B 13F427920
EDK IIlProtocolInterface: 752F3136-4E16-4FDC-A22A-E5F46812F4CA 13F427498
UEFI v2.70 (EDK II, 0x00010000)008-7F9B-4F30-87AC-60C9FEF5DA4E 13EFE0FD0
map: No mapping found.
Press ESC in 4 seconds to skip startup.nsh or any other key to continue.
Shell>
Shell>
Shell>

```
