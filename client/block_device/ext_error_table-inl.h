#ifndef DATTO_CLIENT_BLOCK_DEVICE_EXT_ERROR_TABLE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_EXT_ERROR_TABLE_H_

#include <string>

extern "C" {
#include <ext2fs/ext2_err.h>
}

namespace datto_linux_client {

class ExtErrorTable {
 public:
  ExtErrorTable() {
   add_error_table(&et_ext2_error_table);
  }

  std::string get_error(int return_code) const {
    return std::string(error_message(return_code));
  }

  ~ExtErrorTable() {
    remove_error_table(&et_ext2_error_table);
  }

  ExtErrorTable(const ExtErrorTable&) = delete;
  ExtErrorTable& operator=(const ExtErrorTable&) = delete;
};
} // datto_linux_client

#endif //  DATTO_CLIENT_BLOCK_DEVICE_EXT_ERROR_TABLE_H_

