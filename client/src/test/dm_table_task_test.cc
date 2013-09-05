#include "device_mapper/dm_table_task.h"
#include <glog/logging.h>
#include <iostream>
#include <unistd.h>

int main(int argc, char **argv) {
  FLAGS_v = 3;
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(*argv);

  try {
    datto_linux_client::DmTableTask subject("/dev/mapper/ubuntu32--ng-root");
    subject.Run();
    subject.targets();
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
