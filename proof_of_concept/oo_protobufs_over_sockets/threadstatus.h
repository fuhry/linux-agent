#ifndef THREAD_STATUS
#define THREAD_STATUS

#include <string>

#include <stdexcept>
#include <memory>

//#include <sys/un.h>
//#include <sys/socket.h>
//#include <unistd.h>
//#include <errno.h>
//#include <arpa/inet.h>
//
namespace ThreadStatus {

  using std::string;

class ThreadStatusException : public std::runtime_error {
  public :
    explicit ThreadStatusException(std::string &what) : runtime_error(what){};
};


class ThreadStatus {

  private: 

    enum {
      RUNNING,
      PAUSED,
      STOPPED
    } state_;

    string device_name_;

    bool change_state_;

  public:

    explicit ThreadStatus() : state_(STOPPED), device_name_(""), change_state_(false) {};
    ThreadStatus(const string &devname) : state_(STOPPED), device_name_(devname), change_state_(false) {};

    inline bool SetChangeState(bool newstate) {
      change_state_ = newstate;
      return change_state_;
    }

    inline bool change_state() {
      return change_state_;
    }

    inline string SetDeviceName(string &devname) {
      device_name_ = devname;
      return device_name_;
    }

    inline string device_name() {
      return device_name_;
    }

    inline bool IsRunning() {
      return (state_ == RUNNING) ? true : false;
    }

    inline bool IsStopped() {
      return (state_ == STOPPED) ? true : false;
    }

    inline bool IsPaused() {
      return (state_ == PAUSED) ? true : false;
    }

    inline void SetRunning() {
      state_ = RUNNING;
    }

    inline void setPaused() {
      state_ = PAUSED;
    }

    inline void setStopped() {
      state_ = STOPPED;
    }




};

}   // namespace ThreadStatus

#endif //THREAD_STATUS



    


  



