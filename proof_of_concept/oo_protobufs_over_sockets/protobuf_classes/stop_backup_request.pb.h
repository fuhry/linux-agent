// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: stop_backup_request.proto

#ifndef PROTOBUF_stop_5fbackup_5frequest_2eproto__INCLUDED
#define PROTOBUF_stop_5fbackup_5frequest_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
// @@protoc_insertion_point(includes)

namespace datto_linux_client {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_stop_5fbackup_5frequest_2eproto();
void protobuf_AssignDesc_stop_5fbackup_5frequest_2eproto();
void protobuf_ShutdownFile_stop_5fbackup_5frequest_2eproto();

class StopBackupRequest;

// ===================================================================

class StopBackupRequest : public ::google::protobuf::Message {
 public:
  StopBackupRequest();
  virtual ~StopBackupRequest();
  
  StopBackupRequest(const StopBackupRequest& from);
  
  inline StopBackupRequest& operator=(const StopBackupRequest& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const StopBackupRequest& default_instance();
  
  void Swap(StopBackupRequest* other);
  
  // implements Message ----------------------------------------------
  
  StopBackupRequest* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const StopBackupRequest& from);
  void MergeFrom(const StopBackupRequest& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required string block_path = 1;
  inline bool has_block_path() const;
  inline void clear_block_path();
  static const int kBlockPathFieldNumber = 1;
  inline const ::std::string& block_path() const;
  inline void set_block_path(const ::std::string& value);
  inline void set_block_path(const char* value);
  inline void set_block_path(const char* value, size_t size);
  inline ::std::string* mutable_block_path();
  inline ::std::string* release_block_path();
  
  // @@protoc_insertion_point(class_scope:datto_linux_client.StopBackupRequest)
 private:
  inline void set_has_block_path();
  inline void clear_has_block_path();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  ::std::string* block_path_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];
  
  friend void  protobuf_AddDesc_stop_5fbackup_5frequest_2eproto();
  friend void protobuf_AssignDesc_stop_5fbackup_5frequest_2eproto();
  friend void protobuf_ShutdownFile_stop_5fbackup_5frequest_2eproto();
  
  void InitAsDefaultInstance();
  static StopBackupRequest* default_instance_;
};
// ===================================================================


// ===================================================================

// StopBackupRequest

// required string block_path = 1;
inline bool StopBackupRequest::has_block_path() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void StopBackupRequest::set_has_block_path() {
  _has_bits_[0] |= 0x00000001u;
}
inline void StopBackupRequest::clear_has_block_path() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void StopBackupRequest::clear_block_path() {
  if (block_path_ != &::google::protobuf::internal::kEmptyString) {
    block_path_->clear();
  }
  clear_has_block_path();
}
inline const ::std::string& StopBackupRequest::block_path() const {
  return *block_path_;
}
inline void StopBackupRequest::set_block_path(const ::std::string& value) {
  set_has_block_path();
  if (block_path_ == &::google::protobuf::internal::kEmptyString) {
    block_path_ = new ::std::string;
  }
  block_path_->assign(value);
}
inline void StopBackupRequest::set_block_path(const char* value) {
  set_has_block_path();
  if (block_path_ == &::google::protobuf::internal::kEmptyString) {
    block_path_ = new ::std::string;
  }
  block_path_->assign(value);
}
inline void StopBackupRequest::set_block_path(const char* value, size_t size) {
  set_has_block_path();
  if (block_path_ == &::google::protobuf::internal::kEmptyString) {
    block_path_ = new ::std::string;
  }
  block_path_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* StopBackupRequest::mutable_block_path() {
  set_has_block_path();
  if (block_path_ == &::google::protobuf::internal::kEmptyString) {
    block_path_ = new ::std::string;
  }
  return block_path_;
}
inline ::std::string* StopBackupRequest::release_block_path() {
  clear_has_block_path();
  if (block_path_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = block_path_;
    block_path_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace datto_linux_client

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_stop_5fbackup_5frequest_2eproto__INCLUDED
