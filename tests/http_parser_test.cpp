#include <iostream>
#include <string>
#include <string_view>

struct RequestLine {
    std::string method;
    std::string target;
    std::string version;
};

constexpr size_t kMaxTargetLength = 8192;

bool parseRequestLine(const std::string& rawRequest, RequestLine& outLine, size_t kMaxTargetLength = 8192)
{
    size_t pos1 = rawRequest.find("\r\n");

    if (pos1 == std::string::npos) {
        return false;
    }

    // (Request line = GET /index.html HTTP/1.1)
    // Find the first space | 
    size_t first_space = rawRequest.find(' ');
    
    if (first_space == std::string::npos) {
        return false;
    }
    
    std::string_view method_part = rawRequest.substr(0, first_space);

    if(method_part.empty()) {
        return false;
    }

    // method part should be UPPERCASE only.
    for(char c:method_part){
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 'A' || uc > 'Z') {
            return false;
        }
    }

    // Find the second space (starting the search after the first space) | target part
    size_t  second_space = rawRequest.find(' ', first_space + 1);
    
    if(second_space == std::string::npos) {
        return false;
    }

    std::string_view target_part = rawRequest.substr(first_space + 1, (second_space - first_space + 1));

    // Validate Target: No control characters (< 0x20 or 0x7F)
    for (char c : target_part) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20 || uc == 0x7F) {
            return false;
        }
    }


    // find third space if exist then return false.
    size_t third_space = rawRequest.find(' ', second_space + 1);
    
    if(third_space != std::string::npos){
        return false;
    }

    std::string_view version_part = rawRequest.substr(second_space + 1);

    if (version_part != "HTTP/1.0" && version_part != "HTTP/1.1") {
        return false;
    }

    outLine.method = method_part;
    outLine.target = target_part;
    outLine.version = version_part;

    return true;

}

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
                   const std::string& expVersion, size_t maxLen = 2048) {
    RequestLine line{"OLD_METHOD", "OLD_TARGET", "OLD_VERSION"};
    
    bool ok = parseRequestLine(input, line, maxLen);
    if (!ok || line.method != expMethod || line.target != expTarget || line.version != expVersion) {
        std::cout << "[FAIL] " << testName << " (Input: \"" << escapeString(input) << "\")\n";
        return 1; // 1 failure
    }
    std::cout << "[PASS] " << testName << "\n";
    return 0;
}

// Expects a failure and verifies that the output struct remains completely untouched
int expect_failure(const std::string& testName, std::string input, size_t maxLen = 2048) {
    RequestLine line{"PRESERVED_METHOD", "PRESERVED_TARGET", "PRESERVED_VERSION"};
    
    bool ok = parseRequestLine(input, line, maxLen);
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

    // Success Cases
    failures += expect_success("Basic GET", "GET / HTTP/1.1\r\n", "GET", "/", "HTTP/1.1");
    failures += expect_success("GET with index.html", "GET /index.html HTTP/1.0\r\n", "GET", "/index.html", "HTTP/1.0");
    failures += expect_success("GET with query string (target kept whole)", "GET http://example.com/a?x=1 HTTP/1.1\r\n", "GET", "http://example.com/a?x=1", "HTTP/1.1");
    failures += expect_success("POST method", "POST /submit HTTP/1.1\r\n", "POST", "/submit", "HTTP/1.1");
    failures += expect_success("Full request with headers", "GET / HTTP/1.1\r\nHost: x\r\n\r\n", "GET", "/", "HTTP/1.1");

    // Failure Cases
    failures += expect_failure("No CRLF", "GET / HTTP/1.1");
    failures += expect_failure("Bare LF (no CR)", "GET / HTTP/1.1\n");
    failures += expect_failure("Two consecutive spaces", "GET  / HTTP/1.1\r\n");
    failures += expect_failure("Leading space", " GET / HTTP/1.1\r\n");
    failures += expect_failure("No version (missing target/version)", "GET /\r\n");
    failures += expect_failure("Extra tokens on request line", "GET / HTTP/1.1 extra\r\n");
    failures += expect_failure("Lowercase method", "get / HTTP/1.1\r\n");
    failures += expect_failure("Unsupported HTTP version", "GET / HTTP/2.0\r\n");
    failures += expect_failure("Lowercase HTTP version", "GET / http/1.1\r\n");
    failures += expect_failure("Empty string", "");
    failures += expect_failure("CRLF alone", "\r\n");
    
    // Target length boundary check (kMaxTargetLength = 5, target size = 6)
    failures += expect_failure("Target 1 byte longer than max length", "GET /123456 HTTP/1.1\r\n", 5);
    
    // Target containing a tab character
    failures += expect_failure("Target containing a tab", "GET /a\tb HTTP/1.1\r\n");

    std::cout << "==============================================\n";
    if (failures > 0) {
        std::cout << "Result: " << failures << " test case(s) FAILED.\n";
        std::cin.get();
        return 1;
    } else {
        std::cout << "Result: All test cases PASSED successfully!\n";
        std::cin.get();
        return 0;
    }
}