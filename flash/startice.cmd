+q  // Enter quiet mode
+mon
//________________________________________________________
// startice.cmd: Created by MAJIC Setup Wizard version 3.2 
// Creation Date: 11/14/2005 16:22:46
// Processor type: LX4180 -v4180
// Project: Rtl8111DP
// Description: 8111DP firmware
//________________________________________________________
dv "Reading startice.cmd file\n"
//
// Software Settings
//
eo semi_hosting_enabled= off   // Semihosting support
//
// Target Information Options
//
eo trgt_resets_jtag  = no   // Target reset does not reset JTAG controller
//
// MAJIC Settings
//
eo ice_jtag_clock_freq= 10   // JTAG clock frequency (MHz)
eo ice_jtag_use_trst  = off  // Do not use TRST* to reset JTAG controller
eo ice_reset_output   = off  // reset command does not pulse MAJICs reset output
eo ice_power_sense    = TRST // Sense target voltage level on TRST pin
//
// Physical Memory Configuration
//
mc *:P, inv 	// start with all physical memory as invalid
//
// Address Range          PWE  Access 
// ---------------------- ---- ------ 
MC 00200000:P 0021FFFF:P, PWE, DMA    // ISRAM
MC 01000000:P 01FFFFFF:P, PWE, DMA    // Flash
MC 00100000:P 0011FFFF:P, PWE, JAM    // DMEM
MC 14000000:P 140000FF:P, PWE, JAM    // Flash Controller
MC 1FC00000:P 1FC03FFF:P, PWE, JAM    // Boot Rom
//
// Trace aliases (MAJIC-Plus or MAJIC-mx only)
//
ea dts  do ice_trig*; do trace*  // display trace control settings
ea kts eo tt=auto; eo tta=stop; eo tg=none; // kill trace settings (set defaults)
//
dv "Finished reading startice.cmd\n"
//eb t0 = 0 // proceed with bootcode

dv "loading ...\n"

+edb
+q
fo dash_dbg.exe
dummy  b c {"Starting Programming -- Configuration\n"  ; fr m newconf.img   0x80100000; c}
dummy0 b c {"Starting Programming -- Part1\n"  ; fr m 1.img   0x80100000; c}
dummy1 b c {if stage == 0 {"Starting Programming -- Part2\n"; fr m 2.img 0x80100000; c}{"PART1 ERROR\n"}}
dummy2 b c {if stage == 0 {"Starting Programming -- Part3\n"  ; fr m 3.img 0x80100000; c}{"PART2 ERROR\n"}} 
//dummy3 b c {if stage == 0 {"Starting Programming -- SMBIOS\n"; fr m smbios.bin 0x80100000; c}{"PART3 ERROR\n"}} 
//dummy4 b c {if stage == 0 {"Starting Programming -- ROMDATA\n"; fr m romdata.bin 0x80100000; c}{"SMBIOS ERROR\n"}} 
//dummy5 b c {if stage == 0 {"Starting Programming -- IMEM1\n"; fr m IMEM1.img 0x80100000; c}{"PART3 ERROR\n"}} 
//dummy6 b c {if stage == 0 {"Starting Programming -- IMEM2\n"; fr m IMEM2.img 0x80100000; c}{"IMEM2 ERROR\n"}} 
dummy8 b c {if stage == 0 {"Programming Complete"; q}{"PART3 ERROR"}}
r


-mon
