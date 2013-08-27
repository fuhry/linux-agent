#include "device_mapper/dm_info_task.h"
#include <glog/logging.h>
#include <iostream>
#include <unistd.h>

int main(int argc, char **argv) {
  FLAGS_v = 3;
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(*argv);

  try {
    datto_linux_client::DmInfoTask subject("/dev/mapper/ubuntu32--ng-root");
    subject.Run();
    datto_linux_client::DmInfo info = subject.info();
    LOG(INFO) << "Exists:         " << info.exists;
    LOG(INFO) << "Suspended:      " << info.suspended;
    LOG(INFO) << "Live table:     " << info.live_table;
    LOG(INFO) << "Inactive table: " << info.inactive_table;
  } catch (std::string& s) {
    LOG(FATAL) << "string: " << s;
  } catch (char const* s) {
    LOG(FATAL) << "char const *: " << s;
  } catch (std::exception& e) {
    LOG(FATAL) << "exception: " << e.what();
  } catch (...) {
    LOG(FATAL) << "caught unexpected type";
  }
  return 0;
}
