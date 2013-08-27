#include <string>
#include <cstdlib>
#include "freezable_block_device.h"

namespace datto_linux_client {

FreezableBlockDevice::FreezableBlockDevice(std::string block_path) :
  BlockDevice(block_path) { };

FreezableBlockDevice::~FreezableBlockDevice() {
  if (is_frozen_) {
    try {
      Unfreeze();
    } catch (std::exception &e) {
      // If we are in this catch block, we failed to unfreeze the hard drive.
      //
      // TODO At this point there isn't too much we can do, if smarter handling
      // was needed for this drive, the caller of this function should have called
      // Unfreeze manually and handled the exception there
      //
      // TODO: error: "Unable to unfreeze hard drive: " + BlockDevice::block_path());
      std::abort();
    }
  }
}

void FreezableBlockDevice::Freeze() {
  // TODO
  // TODO: Don't forget to mlockall
}

void FreezableBlockDevice::Unfreeze() {
  // TODO
}

bool FreezableBlockDevice::IsConsistent() const {
  return is_frozen_;
}

}

