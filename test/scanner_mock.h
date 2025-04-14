#include "../src/scanner/scanner.h"

#include <gmock/gmock.h>

class MockScanner : public Scanner {
  public:
    MOCK_METHOD0(scan_token, Token());
};
