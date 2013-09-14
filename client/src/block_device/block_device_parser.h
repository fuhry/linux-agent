#ifndef DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_PARSER_H_
#define DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_PARSER_H_

#include <boost/noncopyable.hpp>
#include <vector>
#include <memory>

#include "block_device/block_device.h"
#include "block_device/partition.h"

namespace datto_linux_client {

class BlockDeviceParser : private boost::noncopyable {

 public:
  // Open() the block device in the constructor
  // The block device should throw an exception if it is already open.
  // This should prevent
  explicit BlockDeviceParser(BlockDevice &block_device);

  // Use the /dev/ directory to get the list of partitions
  const std::vector<Partition> GetPartitions();

  // If the block device contains a boot sector
  // Google '0x55 0xAA'
  const bool IsBootable();

  // Return either the MBR or GPT as binary data
  // Throw an exception if the block device doesn't contain either
  //
  // TODO: Is there a better word than PartitionTable to describe both
  //       the MBR and GPT?
  const std::unique_ptr<const std::vector<unsigned char>> GetPartitionTable();

  // Remember to close the block device
  ~BlockDeviceParser();

 private:
  BlockDevice &block_device_;
  
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_PARSER_H_
