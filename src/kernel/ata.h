#ifndef ATA_PIO
#define ATA_PIO

#include <stdbool.h>

#include "io.h"
#include "types.h"
#include "strings.h"
#include "allocate.h"
#include "global.h"
#include "ioport.h"

#include "stdio/readwriter.h"

// Bus 0
// Control Ports: 1F0-1F7
// DCR / Alt Status: 3F6
// IRQ: 14

// Bus 1
// Control Ports: 170-177
// DCR / Alt Status: 376
// IRQ: 15

// Bus 2
// Control Ports: 1E8-1EF
// DCR / Alt Status: 3E6
// IRQ: Determine via PCI

// Bus 3 
// Control Ports: 168-16F
// DCR / Alt Status: 366
// IRQ: Determine via PCI

// Control Port Map 
// Name            | RW | Offset | Size28 | Size48
// Data Register     RW   0        2        2
// Error Register    R    1        1        2
// Features Reg.     W    1        1        2
// Sector Count Reg. RW   2        1        2
// LBAlo             RW   3        1        2
// LBAmid            RW   4        1        2
// LBAhi             RW   5        1        2
// Drive Select      RW   6        1        1
// Status Reg        R    7        1        1
// Command Reg       W    7        1        1

// DCR Port Map
// Name            | RW | Offset | Size28 | Size48
// Alt. Status       R    0        1        1
// Device Control    W    1        1        1
// Device Address    R    1        1        1

enum ATADriveType {
  ATA,
  ATAPI,
  SATA
};

class ATA {
private:
  ATADriveType type;
protected:
  uint32_t bus;

  uint32_t totalLBA28Sectors;

  uint32_t diskSize;
  uint8_t identifyResult[512];

  Port DataRegister;
  Port ErrorRegister;
  Port FeaturesRegister;
  Port SectorCountRegister;
  Port LBALo;
  Port LBAMid;
  Port LBAHi;
  Port DriveSelectRegister;
  Port StatusRegister;
  Port CommandRegister;

  bool isValid;

  uint8_t readStatus();
  uint8_t pollTillNotBSY();
  uint8_t pollTillDRQ();
public:
  ATA(uint32_t bus);

  bool validDevice();

  void ATARead(uint32_t sector, uint8_t* buffer);
  void ATAWrite(uint32_t sector, uint8_t* buffer);
};

class ATAReadWriter: public ReadWriter, public ATA {
private:
  uint8_t sectorBuffer[512];
  uint32_t currentPosition;

public:
  ATAReadWriter(uint32_t owner, uint32_t bus);

  uint32_t read(uint32_t count, uint8_t* buffer) override;
  uint32_t write(uint32_t count, uint8_t* buffer) override;
  uint32_t size() override;
  uint32_t seek(uint32_t position) override;
};

#endif