// TODO: Integrate this with Google Test
#include <block_device/block_device.h>

#include <iostream>

int main(int argc, char **argv) {

  using std::cerr;
  using std::cout;
  using std::endl;

  using datto_linux_client::BlockDeviceException;
  using datto_linux_client::BlockDevice;

  if (argc < 2) {
    cerr << "Error: must provide one argument (a block device path name)\n";
    exit(1);
  }

  std::string block_dev = argv[1];

  BlockDevice * bd1;
  try {
    bd1 = new BlockDevice(block_dev);
  }
  catch (BlockDeviceException &e) {
    cerr << "Caught BlockDeviceException while creating new BlockDevice object:\n";
    cerr << e.what();
    cerr << "\n";
    exit(1);
  }

  cout << "Block Path Name: " << bd1->block_path() << endl;
  cout << "\tMajor number is " << bd1->major() << endl;
  cout << "\tMinor number is " << bd1->minor() << endl;
  cout << "\tDevice Size is " << bd1->DeviceSizeBytes() << endl;
  cout << "\tBlock Size is " << bd1->BlockSizeBytes() << endl;

  int fd;
  try {
    fd = bd1->Open();
  }
  catch (BlockDeviceException &e) {
    cerr << "Caught BlockDeviceException while creating opening BlockDevice object:\n";
    cerr << e.what();
    cerr << "\n";
    exit(1);
  }

  cout << "\tDevice opened, file descriptor is " << fd << endl;

  exit(0);

}




