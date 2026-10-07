#include <iostream>
#include <string>
#include <string_view>

#include "http/http_parser.hpp"


// constexpr size_t kMaxTargetLength = 8192;

// --- Helper Functions ---
// Helper for escaping newlines in print outputs
std::string escapeString(std::string_view str) {
    std::string result;
    for (char c : str) {
        if (c == '\r') result += "\\r";
        else if (c == '\n') result += "\\n";
        else result.push_back(c);
    }
    return result;
}

// Expects a successful parse and verifies the parsed fields
int expect_success(const std::string& testName, std::string input, 
                   const std::string& expMethod, const std::string& expTarget, 
                   const std::string& expVersion) 
{
    RequestLine line{"OLD_METHOD", "OLD_TARGET", "OLD_VERSION"};
    
    bool ok = parseRequestLine(input, line);
    if (!ok || line.method != expMethod || line.target != expTarget || line.version != expVersion) {
        std::cout << "[FAIL] " << testName << " (Input: \"" << escapeString(input) << "\")\n";
        return 1; // 1 failure
    }
    std::cout << "[PASS] " << testName << "\n";
    return 0;
}

// Expects a failure and verifies that the output struct remains completely untouched
int expect_failure(const std::string& testName, std::string input) 
{
    RequestLine line{"PRESERVED_METHOD", "PRESERVED_TARGET", "PRESERVED_VERSION"};
    
    bool ok = parseRequestLine(input, line);
    if (ok || line.method != "PRESERVED_METHOD" || 
        line.target != "PRESERVED_TARGET" || 
        line.version != "PRESERVED_VERSION") {
        std::cout << "[FAIL] " << testName << " (Expected failure, but got success or struct was modified)\n";
        return 1; // 1 failure
    }
    std::cout << "[PASS] " << testName << "\n";
    return 0;
}

// --- Main Program ---

int main() {
    int failures = 0;

    std::cout << "=== Running HTTP Request Line Parser Tests ===\n";

    // --- Success Cases ---
    failures += expect_success("Basic GET",                              "GET / HTTP/1.1\r\n",                          "GET", "/",                        "HTTP/1.1");
    failures += expect_success("GET with index.html",                   "GET /index.html HTTP/1.0\r\n",                "GET", "/index.html",              "HTTP/1.0");
    failures += expect_success("GET with query string",                 "GET http://example.com/a?x=1 HTTP/1.1\r\n",   "GET", "http://example.com/a?x=1", "HTTP/1.1");
    failures += expect_success("POST method",                           "POST /submit HTTP/1.1\r\n",                   "POST", "/submit",                 "HTTP/1.1");
    failures += expect_success("Full request with headers",             "GET / HTTP/1.1\r\nHost: x\r\n\r\n",           "GET", "/",                        "HTTP/1.1");

    // Target exactly at the limit — must succeed
    {
        const std::string maxTarget = "/" + std::string(kMaxTargetLength - 1, 'a'); // e.g. "/aaa...a", kMaxTargetLength chars total
        const std::string req       = "GET " + maxTarget + " HTTP/1.1\r\n";
        failures += expect_success("Target exactly kMaxTargetLength", req, "GET", maxTarget, "HTTP/1.1");
    }

    // --- Failure Cases ---
    failures += expect_failure("No CRLF",                               "GET / HTTP/1.1");
    failures += expect_failure("Bare LF (no CR)",                       "GET / HTTP/1.1\n");
    failures += expect_failure("Two consecutive spaces",                "GET  / HTTP/1.1\r\n");
    failures += expect_failure("Leading space",                         " GET / HTTP/1.1\r\n");
    failures += expect_failure("No version (missing target/version)",   "GET /\r\n");
    failures += expect_failure("Extra tokens on request line",          "GET / HTTP/1.1 extra\r\n");
    failures += expect_failure("Lowercase method",                      "get / HTTP/1.1\r\n");
    failures += expect_failure("Unsupported HTTP version",              "GET / HTTP/2.0\r\n");
    failures += expect_failure("Lowercase HTTP version",                "GET / http/1.1\r\n");
    failures += expect_failure("Empty string",                          "");
    failures += expect_failure("CRLF alone",                            "\r\n");

    // Target one byte over the limit — must fail
    {
        const std::string longTarget = "/" + std::string(kMaxTargetLength, 'a'); // kMaxTargetLength + 1 chars total
        const std::string req        = "GET " + longTarget + " HTTP/1.1\r\n";
        failures += expect_failure("Target 1 byte over kMaxTargetLength", req);
    }

    // Empty target — must fail
    failures += expect_failure("Empty target",                          "GET  HTTP/1.1\r\n");

    // Space inside a header value must not be mistaken for a third request-line token
    failures += expect_failure("Space in header must not leak into request line", "GET /\r\nHost: a b\r\n\r\n");

    // Target containing a tab character
    failures += expect_failure("Target containing a tab",               "GET /a\tb HTTP/1.1\r\n");

    std::cout << "==============================================\n";
    if (failures > 0) {
        std::cout << "Result: " << failures << " test case(s) FAILED.\n";
        return 1;
    }
    std::cout << "Result: All test cases PASSED successfully!\n";
    
    return 0;
}