"""Compile and exercise the actual injected guard without requiring an ESP32 SDK."""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from webserver_security import guarded_parser

fixture = """
#include <cassert>
#include <cstdint>
#include <string>
using String = std::string;
class WiFiClient {};
bool bodyReached = false;
class WebServer { public: bool _parseForm(WiFiClient&, String, uint32_t); };
bool WebServer::_parseForm(WiFiClient& client, String boundary, uint32_t len){
  (void)client; (void)len;
  bodyReached = true;
  int fastBoundaryLen = 4 + boundary.length() + 1;
  char fastBoundary[ fastBoundaryLen ];
  (void)fastBoundary;
  return true;
}
int main() {
  WebServer web; WiFiClient client;
  for (size_t length : {size_t(1), size_t(70)}) {
    bodyReached = false;
    assert(web._parseForm(client, String(length, 'A'), 0) && bodyReached);
  }
  for (size_t length : {size_t(0), size_t(71), size_t(12000)}) {
    bodyReached = false;
    assert(!web._parseForm(client, String(length, 'A'), 0) && !bodyReached);
  }
}
"""
with tempfile.TemporaryDirectory(prefix="ctek-webserver-security-") as folder:
    source = Path(folder) / "guard.cpp"
    executable = Path(folder) / "guard_test"
    source.write_text(guarded_parser(fixture))
    subprocess.run([os.environ.get("CXX", "g++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-no-pie",
                    str(source), "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)

for changed in ("unexpected upstream parser", fixture.replace("_parseForm(", "_parseFormNew(")):
    try:
        guarded_parser(changed)
    except ValueError:
        pass
    else:
        raise AssertionError("Een gewijzigde upstream parser werd zonder controle geaccepteerd")
print("WebServer-beveiligingstest geslaagd: grens 70, te lange invoer vóór stackallocatie geweigerd.")
