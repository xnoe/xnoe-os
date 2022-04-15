#include "ata.h"

uint16_t controlBase[4] = {0x1f0, 0x170, 0x1e8, 0x168};
uint16_t dcrBase[4] = {0x3f6, 0x376, 0x3e6, 0x366};

uint8_t ATA::readStatus() {
  uint8_t status;
  for (int i=0; i<15; i++)
    status=StatusRegister.readb();
  return status;
}

uint8_t ATA::pollTillNotBSY() {
  uint8_t lastStatus;
  while (((lastStatus=StatusRegister.readb()) & 0x80))
    if (lastStatus&0x1)
      return lastStatus;
  return 0;
}

uint8_t ATA::pollTillDRQ() {
  uint8_t lastStatus;
  while (!((lastStatus=StatusRegister.readb()) & 0x8))
    if (lastStatus&0x1)
      return lastStatus;
  return 0;
}

ATA::ATA(uint32_t bus) :
DataRegister(controlBase[bus]+0),
ErrorRegister(controlBase[bus]+1),
FeaturesRegister(controlBase[bus]+1),
SectorCountRegister(controlBase[bus]+2),
LBALo(controlBase[bus]+3),
LBAMid(controlBase[bus]+4),
LBAHi(controlBase[bus]+5),
DriveSelectRegister(controlBase[bus]+6),
StatusRegister(controlBase[bus]+7),
CommandRegister(controlBase[bus]+7) {
  this->isValid = 0;
  
  DriveSelectRegister.writeb(0xA0);
  LBALo.writeb(0);
  LBAMid.writeb(0);
  LBAHi.writeb(0);
  CommandRegister.writeb(0xEC);

  if (!readStatus()) {
    this->isValid = false;
  } else {
    pollTillNotBSY();
    uint8_t LBAmid;
    uint8_t LBAhi;
    if ((LBAmid = LBAMid.readb()) || (LBAhi = LBAHi.readb())) {
      this->isValid = false;
    } else {
      uint8_t status = pollTillDRQ();
      if (!(status&0x1)) {
        for (int i=0; i<256; i++)
          ((uint16_t*)identifyResult)[i] = DataRegister.readw();
        this->isValid = true;
      }
    }
  }

  if (this->isValid) {
    this->totalLBA28Sectors = *((uint32_t*)(identifyResult+60));
    this->diskSize = this->totalLBA28Sectors * 512;
  }
}

bool ATA::validDevice() {
  return this->isValid;
}

void ATA::ATARead(uint32_t sector, uint8_t* buffer) {
  driveLock.lock();
  DriveSelectRegister.writeb(0xE0 | ((sector >> 24) & 0xf));
  SectorCountRegister.writeb(1);
  LBALo.writeb((uint8_t)sector);
  LBAMid.writeb((uint8_t)(sector>>8));
  LBAHi.writeb((uint8_t)(sector>>16));
  CommandRegister.writeb(0x20);
  pollTillNotBSY();
  for (int i=0; i<256; i++)
    ((uint16_t*)buffer)[i] = DataRegister.readw();
  driveLock.unlock();
}

void ATA::ATAWrite(uint32_t sector, uint8_t* buffer) {
  driveLock.lock();
  DriveSelectRegister.writeb(0xE0 | ((sector >> 24) & 0xf));
  SectorCountRegister.writeb(1);
  LBALo.writeb((uint8_t)sector);
  LBAMid.writeb((uint8_t)(sector>>8));
  LBAHi.writeb((uint8_t)(sector>>16));
  CommandRegister.writeb(0x30);
  pollTillNotBSY();
  for (int i=0; i<256; i++)
    DataRegister.writew(((uint16_t*)buffer)[i]);
  CommandRegister.writeb(0xe7);
  driveLock.unlock();
}

ATAReadWriter::ATAReadWriter(uint32_t owner, uint32_t bus): 
ATA(bus),
ReadWriter(owner) {
  this->currentPosition = 0;
}

uint32_t ATAReadWriter::read(uint32_t count, uint8_t* buffer) {
  driveLock.lock();
  uint32_t remainingBytes = count;
  uint32_t inThisSector = currentPosition % 512;

  uint32_t index=0;

  uint32_t c=0;

  ATARead(currentPosition / 512, sectorBuffer);
  while (remainingBytes) {
    if (currentPosition >= diskSize)
      break;
    buffer[index++] = sectorBuffer[inThisSector++];
    remainingBytes--;
    currentPosition++;
    if (inThisSector == 512) {
      ATARead(currentPosition / 512, sectorBuffer);
      inThisSector = 0;
    }
    c++;
  }
  driveLock.unlock();
  return c;
}
uint32_t ATAReadWriter::write(uint32_t count, uint8_t* buffer) {
  driveLock.lock();
  uint32_t remainingBytes = count;
  uint32_t inThisSector = currentPosition % 512;

  uint32_t index=0;

  uint32_t c=0;

  ATARead(currentPosition / 512, sectorBuffer);
  while (remainingBytes) {
    if (currentPosition >= diskSize)
      break;
    sectorBuffer[inThisSector++] = buffer[index++];
    remainingBytes--;
    currentPosition++;
    if (inThisSector == 512) {
      ATAWrite((currentPosition / 512) - 1, sectorBuffer);
      ATARead(currentPosition / 512, sectorBuffer);
      inThisSector = 0;
    }
    c++;
  }
  // Perform a final write to ensure that we've written everything to disk.
  ATAWrite((currentPosition / 512), sectorBuffer);
  driveLock.unlock();
  return c;
}
uint32_t ATAReadWriter::size() {
  return this->diskSize;
}
uint32_t ATAReadWriter::seek(uint32_t position) {
  this->currentPosition = position;
}