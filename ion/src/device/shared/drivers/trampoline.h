#ifndef ION_DEVICE_SHARED_DRIVERS_TRAMPOLINE_H
#define ION_DEVICE_SHARED_DRIVERS_TRAMPOLINE_H

namespace Ion {
namespace Device {
namespace Trampoline {

#define TRAMPOLINE_ATTRIBUTES __attribute__((externally_visible))

#define TRAMPOLINE_SUSPEND 0
#define TRAMPOLINE_AUTHENTICATION (1+TRAMPOLINE_SUSPEND)
#define TRAMPOLINE_EXTERNAL_FLASH_MASS_ERASE (1+TRAMPOLINE_AUTHENTICATION)
#define TRAMPOLINE_EXTERNAL_FLASH_ERASE_SECTOR (1+TRAMPOLINE_EXTERNAL_FLASH_MASS_ERASE)
#define TRAMPOLINE_EXTERNAL_FLASH_WRITE_MEMORY (1+TRAMPOLINE_EXTERNAL_FLASH_ERASE_SECTOR)
#define NUMBER_OF_TRAMPOLINE_FUNCTIONS (TRAMPOLINE_EXTERNAL_FLASH_WRITE_MEMORY+1)

}
}
}

#endif
