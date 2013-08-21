#ifndef DATTO_CLIENT_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_H_

#include <boost/noncopyable.hpp>
#include <string>

namespace datto_linux_client {
class BlockDevice : private boost::noncopyable {
  public:
    virtual std::string block_path() const;
    virtual ~BlockDevice();
    virtual bool IsConsistent() = 0;
  protected:
    explicit BlockDevice(std::string block_path);
    std::string block_path_;
};
}

#endif // DATTO_CLIENT_BLOCK_DEVICE_H_
