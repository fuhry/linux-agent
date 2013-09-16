#ifndef DATTO_CLIENT_BLOCK_DEVICE_LOCAL_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_LOCAL_BLOCK_DEVICE_H_

#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <string>

namespace datto_linux_client {

class BlockDevice : private boost::noncopyable {

 public:
  // Creates a BlockDevice from the given block_path
  // An exception should be thrown (likely a BlockDeviceException) if the
