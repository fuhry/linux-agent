# Logging
## GLog
We use [GLog](https://code.google.com/p/google-glog/) for logging. It is [well documented](http://google-glog.googlecode.com/svn/trunk/doc/glog.html) and straight forward to use.

### Installing
The `install_deps` script should install glog for you.

## How much to log
Prefer logging too much to logging too little. Debugging, both during development and in the field, is much, much easier with good logging. Strong log messages will *greatly* reduce the burden on support. Don't worry about the performance overhead of logging until there is reason to.
