// Define host functions.

#include "host.h"
#include <stdbool.h>

void printnl();
void printf(char* input, uint32_t text_colour, uint32_t bg_colour);
void* malloc(uint32_t size, char reset, char skip_reuse, uint32_t process_id);
void* realloc(void* ptr, uint32_t size, bool reset, uint32_t process_id);
bool free(void* ptr);
void printf_log(char* name, char* input, const uint32_t name_colour, const uint32_t input_colour);

void* laihost_malloc(size_t size) {
    return malloc(size, false, false, 0);
}

void* laihost_realloc(void* ptr, size_t newsize, size_t oldsize) {
    return realloc(ptr, newsize, true, 0);
}

void laihost_free(void* ptr, size_t size) {
    free(ptr);
}

struct RSDPDescriptor {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __attribute__ ((packed));

struct SDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};

struct RSDT {
  struct SDTHeader h;
  void* table_pointers[];
} __attribute__((packed));

struct GenericAddressStructure
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
};

struct DSDT {
    struct SDTHeader h;
    char aml[];
};

struct FADT
{
    struct   SDTHeader h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
    uint8_t  Reserved;
 
    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;
    uint16_t BootArchitectureFlags;
 
    uint8_t  Reserved2;
    uint32_t Flags;
    struct GenericAddressStructure ResetReg;
 
    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
};

bool verify_signature(void* location_to_probe, uint32_t length, char* signature) {
    char* signature_location = (char*)location_to_probe;

    // Look for "RSD PTR " string.
    for (uint32_t i = 0; i < length; i++) {
        // If a character does not match, return false.
        if (signature_location[i] != signature[i]) return false;
    }

    return true;
}

void* locate_rsdp() {
    
    // Start address of EBDA.
    uint8_t* ebda = (uint8_t*)0x9FC00;
    
    // Look for RSDP in the bios extended area.
    for (uint32_t i = 0; i < 64; i++) {
        // RSDP is aligned on a 16-bit boundary.
        uint8_t* segment = (ebda + (i * 16));
        
        // Verify signature.
        if (verify_signature(segment, 8, "RSD PTR ") == true) {
            
            // RSDP found, return address.
            return segment;
        }
    }
    
    // Address of possible RSDP area.
    uint8_t* mba = (uint8_t*)0x000E0000;
    
    for (uint32_t i = 0; i < 8192; i++) {
        uint8_t* segment = mba + (i * 16);
        
        // Verify signature.
        if (verify_signature(segment, 8, "RSD PTR ") == true) {
            
            // RSDP found, return address.
            return segment;
        }
    }
    
    // Halt the system.
    //asm volatile("cli; hlt");
    asm volatile("cli");
    asm volatile("hlt");
    
    return NULL;
}

void* laihost_scan(const char* signature, size_t index) {
    if (memcmp("DSDT", signature, 4) == 0) {
        struct FADT* fadt = (struct FADT*)laihost_scan("FACP", 0);
        if (fadt == NULL) return NULL;
        else return (void*)fadt->Dsdt;
    }

    struct RSDPDescriptor* rsdp = locate_rsdp();
    struct RSDT* rsdt = (struct RSDT*)rsdp->RsdtAddress;

    uint32_t entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
    uint32_t current_index = 0;
 
    for (uint32_t i = 0; i < entries; i++) {
        // Get table of header.
        struct SDTHeader* header = rsdt->table_pointers[i];

        // Check if signature matches requested table.
        if (verify_signature(header, 4, (char*)signature) == true) {
            if (index == current_index) return header;
            else current_index++;
        }
    }

    return NULL;
}

void laihost_log(int type, const char* text) {
    uint32_t color;
    if (type == LAI_DEBUG_LOG) color = 0xFFFFFF;
    else if (type == LAI_WARN_LOG) color = 0xfa9b2f;

    printf_log("ACPI LAI", (char*)text, 0x00b3ff, color);
}

void laihost_panic(const char* text) {
    printf_log("ACPI LAI", (char*)text, 0x00b3ff, 0xFF0000);
    printf_log("Kernel", "System has been halted.", 0x00b3ff, 0xFFFFFF);
    asm volatile("cli; hlt");
    __builtin_unreachable();
}

void* laihost_map(size_t address, size_t count) {
    return (void*)address;
}

void laihost_unmap(void *pointer, size_t count) {
    return;
}

void laihost_outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void laihost_outw(uint16_t port, uint16_t val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

void laihost_outd(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t laihost_inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint16_t laihost_inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t laihost_ind(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    uint32_t address = (uint32_t)((seg << 16) | (bus << 8) | (slot << 3) | (fun << 0) | (offset & 0xFC));
    asm volatile("outl %0, %1" : : "a"(address), "Nd"(0xCF8));
    return laihost_inb(0xCFC + (offset & 0x03));
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    uint32_t address = (uint32_t)((seg << 16) | (bus << 8) | (slot << 3) | (fun << 0) | (offset & 0xFC));
    asm volatile("outl %0, %1" : : "a"(address), "Nd"(0xCF8));
    return laihost_inw(0xCFC + (offset & 0x03));
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    uint32_t address = (uint32_t)((seg << 16) | (bus << 8) | (slot << 3) | (fun << 0) | (offset & 0xFC));
    asm volatile("outl %0, %1" : : "a"(address), "Nd"(0xCF8));
    return laihost_ind(0xCFC + (offset & 0x03));
}

void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val) {
    uint32_t address = (uint32_t)((seg << 16) | (bus << 8) | (slot << 3) | (fun << 0) | (offset & 0xFC));
    asm volatile("outl %0, %1" : : "a"(address), "Nd"(0xCF8));
    laihost_outb(0xCFC + (offset & 0x03), val);
}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val) {
    uint32_t address = (uint32_t)((seg << 16) | (bus << 8) | (slot << 3) | (fun << 0) | (offset & 0xFC));
    asm volatile("outl %0, %1" : : "a"(address), "Nd"(0xCF8));
    laihost_outw(0xCFC + (offset & 0x03), val);
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val) {
    uint32_t address = (uint32_t)((seg << 16) | (bus << 8) | (slot << 3) | (fun << 0) | (offset & 0xFC));
    asm volatile("outl %0, %1" : : "a"(address), "Nd"(0xCF8));
    laihost_outd(0xCFC + (offset & 0x03), val);
}

void laihost_sleep(uint64_t ms) {
    return;
}