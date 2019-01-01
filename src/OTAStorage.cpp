#include "OTAStorage.h"

#if defined(ARDUINO_ARCH_SAMD)
static const uint32_t pageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
#ifdef ARDUINO_SAM_ZERO
#define BOOTLOADER_SIZE        (0x4000)
#else
#define BOOTLOADER_SIZE        (0x2000)
#endif
#elif defined(ARDUINO_ARCH_NRF5)
extern "C" {
char * __isr_vector();
}
#elif defined(__AVR__)
#include <avr/wdt.h>
#include <avr/boot.h>
#define MIN_BOOTSZ (4 * SPM_PAGESIZE)
#endif

OTAStorage::OTAStorage() :
#if defined(ARDUINO_ARCH_SAMD)
        SKETCH_START_ADDRESS(BOOTLOADER_SIZE),
        PAGE_SIZE(pageSizes[NVMCTRL->PARAM.bit.PSZ]),
        MAX_FLASH(PAGE_SIZE * NVMCTRL->PARAM.bit.NVMP)
#elif defined(ARDUINO_ARCH_NRF5)
        SKETCH_START_ADDRESS((uint32_t) __isr_vector),
        PAGE_SIZE((size_t) NRF_FICR->CODEPAGESIZE),
        MAX_FLASH(PAGE_SIZE * (uint32_t) NRF_FICR->CODESIZE)
#elif defined(__AVR__)
        SKETCH_START_ADDRESS(0),
        PAGE_SIZE(SPM_PAGESIZE),
        MAX_FLASH((uint32_t) FLASHEND + 1)
#endif
{
  bootloaderSize = 0;
#ifdef __AVR__
  cli();
  uint8_t highBits = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
  sei();
  if (!(highBits & bit(FUSE_BOOTRST))) {
    byte v = (highBits & ((~FUSE_BOOTSZ1 ) + (~FUSE_BOOTSZ0 )));
    bootloaderSize = MIN_BOOTSZ << ((v >> 1) ^ 3);
  }
#endif
}

void ExternalOTAStorage::apply() {
#ifdef __AVR__
  wdt_enable(WDTO_15MS);
  while (true);
#else
  NVIC_SystemReset();
#endif
}
