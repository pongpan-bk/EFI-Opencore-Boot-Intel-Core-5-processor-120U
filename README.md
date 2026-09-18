# EFI OpenCore — Intel Core 5 (120U) Hackintosh

> OpenCore EFI สำหรับเครื่อง **Acer** พร้อม CPU **Intel Core 5 120U (Raptor Lake-U)** / macOS Sonoma 14.8.9
> ใช้ iGPU เดิม `0xa7ac8086` ผ่าน **MyIntelGPU.kext** (ไม่มี AppleIntelKBLGraphics — panic ไม่รองรับ GPU นี้)

---

## รายละเอียดเครื่อง (Hardware)

| Component | รายละเอียด |
|---|---|
| CPU | Intel Core 5 120U (Raptor Lake-U) |
| iGPU | Intel UHD `0xa7ac8086` (Acer, SUBSYS 192E1025) — ขับด้วย MyIntelGPU.kext |
| จอ | eDP 1920x1080@60, panel `KD156N2930A06` (KDB 0x2C82 / 0x0924) |
| Audio | Realtek ALC (alcid=13) |
| LAN | Realtek RTL8111 |
| WiFi/BT | Realtek rtw88 / RealtekBluetoothFirmware |
| CardReader | Realtek (RTS) |
| Touchpad | AlpsHID / VoodooI2C |
| SMBIOS | MacBookPro16,2 |

## สเปคระบบ (เปิดใช้งาน)

- **macOS**: Sonoma 14.8.9 (x86_64)
- **OpenCore**: 2.0.0
- **Boot-args**: `-v keepsyms=1 alcid=13`
- **csr-active-config**: `0x67` (SIP ปิดบางส่วน — อนุญาต kext ที่ไม่ signature / UAKL)
- **Kernel**: kext ทั้งหมดโหลดจาก `/Library/Extensions` + OC Kexts (ตาม config.plist)

---

## โครงสร้าง EFI

```
EFI/
├── BOOT/
│   ├── BOOTx64.efi
│   ├── fbx64.efi
│   └── mmx64.efi
└── OC/
    ├── ACPI/        (18 SSDT — PLUG, EC, PNLF, USBX, RHUB, SBUS, XOSI, ...)
    ├── Drivers/     (19 drivers — OpenRuntime, OpenCanopy, HfsPlus, ExFatDxe, AudioDxe, ...)
    ├── Kexts/       (42 kexts — VirtualSMC, Lilu, WhateverGreen, MyIntelGPU, AppleALC, ...)
    ├── Resources/   (Audio, Font, Image, Label — OpenCanopy GUI)
    ├── Tools/       (15 tools — OpenShell, ResetSystem, CsrUtil, CleanNvram, ...)
    ├── Backup/      (config.plist.bak-20260917, oldConfig.plist)
    ├── OpenCore.efi
    └── config.plist (62484 bytes)
```

### Kexts สำคัญ (มีใน `OC/Kexts/`)

| Kext | หน้าที่ |
|---|---|
| **MyIntelGPU.kext** | iGPU driver ตัวเอง (GPU 0xa7ac8086 ไม่มีใน Mac จริง → เขียนเอง, repo: [IntelReviveGPU-Gen-10-12-on-Hackintosh](https://github.com/pongpan-bk/IntelReviveGPU-Gen-10-12-on-Hackintosh)) |
| VirtualSMC + SMCBatteryManager + SMCProcessor + SMCSuperIO + SMCLightSensor | SMC เลียนแบบฮาร์ดแวร์ Mac |
| Lilu + WhateverGreen | foundation + แก้ GPU/display | 
| AppleALC | Audio codec (alcid=13) |
| CPUFriend + CPUFriendDataProvider | แก้เฟือง CPU (power management) |
| CpuTopologyRebuild | แก้ topology CPU สำหรับ U-series |
| NVMeFix | power management NVMe |
| VoodooI2C + VoodooI2CHID + AlpsHID + VoodooPS2Controller | Touchpad/Keyboard |
| rtw88 + RtWlanU + RealtekBluetoothFirmware | WiFi/BT Realtek |
| RealtekRTL8111 + RealtekCardReader / Friend | LAN + CardReader |
| USBToolBox + UTBMap + UTBDefault | USB map |
| ECEnabler, FeatureUnlock, HibernationFixup, RTCMemoryFixup, RestrictEvents, BrightnessKeys, CodecCommander, Display-756e6b6e-717, NullEthernet, XHCI-unsupported, BlueToolFixup, AppleHDAController, AppleHDAHardwareConfigDriver, AMFIPass | ส่วนเสริม |

---

## วิธีติดตั้ง (Installation)

### เตรียม USB / ติดตั้งบนเครื่อง
1. Format  USB → **FAT32 (MS-DOS)** — หรือใช้ EFI partition โดยตรง
2. Copy โฟลเดอร์ `EFI/` ไปที่ root ของ USB / EFI partition
3. Boot ผ่าน BIOS → เลือก USB (UEFI) หรือจาก OpenCore picker
4. ตั้งค่า BIOS (Acer):
   - `VT-d` → **Disabled** (หรือใช้ SSDT-RMNE ช่วย)
   - `Secure Boot` → **Disabled**
   - `CFG Lock` → Disabled (ถ้าเปิดได้) / ใช้ `ControlMsrE2.efi` ช่วย
   - `DVMT Pre-Allocated` → 64MB ขึ้นไป (สำคัญกับ iGPU)
   - Boot mode → **UEFI**

### MyIntelGPU (iGPU)
- ขับ iGPU ด้วย kext **MyIntelGPU** (โหลดจาก `/Library/Extensions`)
- ต้องแน่ใจว่า kext มี `Contents/PlugIns/IntelXeMetal.bundle` ครบ (Metal)
- ถ้า kext ไม่ครบ → ระบบจะ glitch / System Settings crash
- หลังลง kext ใหม่ ทุกรอบ ต้อง rebuild cache:
  ```bash
  sudo kmutil install --allow-missing-kdk --volume-root /
  ```
- CI build (GitHub Actions) จะผลิต kext + pkg ที่สมบูรณ์: ดู [IntelReviveGPU-Gen-10-12-on-Hackintosh](https://github.com/pongpan-bk/IntelReviveGPU-Gen-10-12-on-Hackintosh)

---

## หมายเหตุ / Notes

- `config.plist.bak-20260917` + `oldConfig.plist` เก็บสำรองไว้ใน `OC/Backup/`
- `opencore-*.txt` = boot logs สำหรับ debug (ไม่ควร commit — `.gitignore` ครอบไว้แล้ว)
- ค่า boot-args `-v` ไว้ดู verbose boot — ถ้าใช้งานจริงจะลบออกก็ได้

## Credit

- [acidanthera/OpenCorePkg](https://github.com/acidanthera/OpenCorePkg)
- [acidanthera/Lilu](https://github.com/acidanthera/Lilu) และตระกูล kext ของ acidanthera
- [pongpan-bk/IntelReviveGPU-Gen-10-12-on-Hackintosh](https://github.com/pongpan-bk/IntelReviveGPU-Gen-10-12-on-Hackintosh) (MyIntelGPU)