## FS PARSER ##
###Usage###
`make` will build both a test executable and a static library/header file to the `dist` directory.

The `test` directory includes various scripts to verify behavioural correctness.

###Requirements###

####C compiler####
* Must support C89 and C99 (`clang` or `gcc`)

####Included libraries####
* `xfs`
* `btrfs`

####Required external libraries####
* `ext2fs`
* `reiserfs`
* `dal` (included with reiserfs)
* POSIX
