## Building Woes ##
### ext ###
* Such a good filesystem. No complaints here!

### XFS ###
* Current builds are changing and causing code breakage (initialization differences)
* The XFS source in this project is modified:
  * Includes are now flat files
  * Void pointers are now cast - C++ compatibility

### ReiserFS ###
* ReiserFS has C++ incompatibility with undefined enum width
