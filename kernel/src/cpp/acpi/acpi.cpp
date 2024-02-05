#include "acpi.h"
#include "lai/core.h"
#include "../io/video.h"
#include "../../../../shared/conversions.h"
#include "../abstractions/io_port.h"

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

struct GenericAddressStructure {
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

struct FADT {
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
    GenericAddressStructure ResetReg;
 
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
    video::printf_log("ACPI", "Searching for RSDP signature in EBDA...");
    
    // Start address of EBDA.
    uint8_t* ebda = (uint8_t*)0x9FC00;
    
    // Look for RSDP in the bios extended area.
    for (uint32_t i = 0; i < 64; i++) {
        // RSDP is aligned on a 16-bit boundary.
        uint8_t* segment = (ebda + (i * 16));
        
        // Verify signature.
        if (verify_signature(segment, 8, "RSD PTR ") == true) {
            video::printf_log("ACPI", "Found RSDP signature in EBDA.");
            
            // RSDP found, return address.
            return segment;
        }
    }
    
    // RSDP has not been in the EBDA found past this point.
    video::printf_log("ACPI", "RSDP signature not found in EBDA.");
    
    video::printf_log("ACPI", "Searching for RSDP signature in main BIOS area...");
    
    // Address of possible RSDP area.
    uint8_t* mba = (uint8_t*)0x000E0000;
    
    for (uint32_t i = 0; i < 8192; i++) {
        uint8_t* segment = mba + (i * 16);
        
        // Verify signature.
        if (verify_signature(segment, 8, "RSD PTR ") == true) {
            video::printf_log("ACPI", "Found RSDP signature in main BIOS area.");
            
            // RSDP found, return address.
            return segment;
        }
    }
    
    // RSDP cannot be found past this point.
    video::printf_log("ACPI", "Could not locate the RSDP, halting system...", VGA_COLOUR::LIGHT_BLUE, VGA_COLOUR::LIGHT_RED);
    
    // Halt the system.
    asm volatile("cli");
    asm volatile("hlt");
    
    return nullptr;
}

void* find_table(void* RootSDT, char* signature) {
    RSDT* rsdt = (RSDT*)RootSDT;

    // Calculate amount of tables.
    uint32_t entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
 
    for (uint32_t i = 0; i < entries; i++) {
        // Get table of header.
        SDTHeader* header = (SDTHeader*)rsdt->table_pointers[i];

        // Check if signature matches requested table.
        if (verify_signature(header, 4, signature) == true) {
            // Return location of table.
            return header;
        }
    }

    // Table could not be found.
    return nullptr;
}

// Caches.
RSDT* rsdt;
FADT* fadt;

void acpi::initialise() {
    RSDPDescriptor* rsdp = (RSDPDescriptor*)locate_rsdp();
    rsdt = reinterpret_cast<RSDT*>(rsdp->RsdtAddress);
    fadt = (FADT*)find_table(rsdt, "FACP");

    video::printf_log("ACPI", structures::string("Revision detected as ").concat(conversions::u_int_to_char(rsdt->h.Revision)).concat("."));
    video::printf_log("ACPI", "Configuring LAI...");

    lai_set_acpi_revision(rsdt->h.Revision);
    lai_create_namespace();

    /*
    video::printf_log("ACPI", "Waiting for ACPI to hand over control...");

    io_port::bit_8::out(fadt->SMI_CommandPort, fadt->AcpiEnable);
    while ((*reinterpret_cast<uint8_t*>(fadt->PM1aControlBlock) & 1) == 0);

    video::printf_log("ACPI", "Change registered, the ACPI is ours now");
    */
}

void acpi::enter_sleep_state(sleep_type type) {
    char* state_name;
    if (type == sleep_type::S0_NONE) state_name = "\\_S0";
    else if (type == sleep_type::S1_REDUCED) state_name = "\\_S1";
    else if (type == sleep_type::S5_MECHANICALOFF) state_name = "\\_S5";

    lai_nsnode_t* s_node = lai_resolve_path(NULL, state_name);

    // Used to track AML interpretation state, not important.
    lai_state_t state;
    lai_init_state(&state);

    // Placeholder variables.
    lai_variable_t package;
    lai_variable_t slp_typa;
    lai_variable_t slp_typb;
    
    // Read the AML sleep package data.
    lai_eval(&package, s_node, &state);
    lai_obj_get_pkg(&package, 0, &slp_typa);
    lai_obj_get_pkg(&package, 1, &slp_typb);

    // ACPI spec requires you to call _GTS and _PTS before sleeping.
    // Only do so when found.

    // Create object describing sleep state (in case needed).
    lai_variable_t s_object;
    s_object.type = LAI_INTEGER;
    s_object.integer = type & 0xFF;

    lai_nsnode_t* pts_node = lai_resolve_path(NULL, "\\_PTS");
    if (pts_node) {
        lai_init_state(&state);

        // Call the function.
        lai_eval_largs(NULL, pts_node, &state, &s_object, NULL);
        lai_finalize_state(&state);
    }

    lai_nsnode_t* gts_node = lai_resolve_path(NULL, "\\_GTS");
    if (gts_node) {
        lai_init_state(&state);

        // Call the function.
        lai_eval_largs(NULL, gts_node, &state, &s_object, NULL);
        lai_finalize_state(&state);
    }

    uint16_t data;

    data = io_port::bit_16::in(fadt->PM1aControlBlock);
    data &= 0xE3FF;
    data |= (slp_typa.integer << 10) | ACPI_SLEEP;
    io_port::bit_16::out(fadt->PM1aControlBlock, data);

    // In case FADT has B1.
    if (fadt->PM1bControlBlock) {
        data = io_port::bit_16::in(fadt->PM1bControlBlock);
        data &= 0xE3FF;
        data |= (slp_typb.integer << 10) | ACPI_SLEEP;
        io_port::bit_16::out(fadt->PM1bControlBlock, data);
    }
}