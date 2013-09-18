## Building Woes ##
### ext ###
* Such a good filesystem. No complaints here!

### XFS ###
* Current builds are changing and causing code breakage (initialization differences)
* The XFS source in this project is modified:
  * Includes are now flat files
  * Void pointers are now cast - C++ compatibility; Error:  
```
In file included from ./libxfs/include/libxfs.h:33:
./libxfs/include/swab.h:183:13: error: cannot initialize a variable of type
      '__uint8_t *' (aka 'unsigned char *') with an lvalue of type 'void *'
        __uint8_t *__p =  p;
```
  * Variables named `delete` are renamed via macro hax - C++ compatibility; Error:  
```
./libxfs/include/xfs_ialloc.h:91:8: error: expected ')'
       int             *delete,
```
### ReiserFS ###
* ReiserFS has C++ incompatibility with undefined enum width; Error:  
```
/usr/include/reiserfs/gauge.h:29:14: error: ISO C++ forbids forward references
      to 'enum' types
typedef enum reiserfs_gauge_kind reiserfs_gauge_kind_t;
```
