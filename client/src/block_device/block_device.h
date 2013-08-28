#ifndef DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_H_

#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <string>

namespace datto_linux_client {
class BlockDevice : private boost::noncopyable {
 public:
  virtual std::string block_path() const;
  virtual uint32_t major() const;
  virtual uint32_t minor() const;

  virtual uint64_t SizeInBytes() const;

  virtual ~BlockDevice();
 protected:
  explicit BlockDevice(std::string block_path);
  BlockDevice(uint32_t major, uint32_t minor);
  std::string block_path_;
  uint32_t major_;
  uint32_t minor_;
};
}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_H_
