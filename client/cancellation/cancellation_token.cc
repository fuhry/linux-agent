#include "cancellation/cancellation_token.h"

namespace datto_linux_client {

CancellationToken() : should_cancel_(false) {}

void CancellationToken::Cancel() {
  should_cancel_ = true;
}
bool ShouldCancel() {
  return should_cancel_;
}

} // datto_linux_client
