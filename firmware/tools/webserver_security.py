"""Backport the multipart boundary limit to a build-local Arduino 2.0.17 copy.

GHSA-8cmm-3887-r32j / CVE-2026-42854: the upstream parser allocates a
stack array using the supplied boundary length, before upload authorization.
The installed SDK remains untouched. Fail the build if its parser changes.
"""
from pathlib import Path

ENTRY = "bool WebServer::_parseForm(WiFiClient& client, String boundary, uint32_t len){\n"
GUARD = "  if (boundary.length() == 0 || boundary.length() > 70) return false;\n"


def guarded_parser(source):
    if source.count(ENTRY) != 1 or source.count("char fastBoundary[ fastBoundaryLen ];") != 1:
        raise ValueError("WebServer-parser gewijzigd: beoordeel de beveiligingspatch opnieuw")
    return source.replace(ENTRY, ENTRY + GUARD, 1)


def register_patch(environment):
    def replace_parser(env, node):
        original = Path(node.srcnode().get_abspath())
        patched = Path(env.subst("$BUILD_DIR")) / "ctek-security" / "Parsing.cpp"
        patched.parent.mkdir(parents=True, exist_ok=True)
        text = guarded_parser(original.read_text())
        if not patched.exists() or patched.read_text() != text:
            patched.write_text(text)
        print("CTEK-beveiliging: multipart boundary begrensd op 70 tekens")
        return env.File(str(patched))

    environment.AddBuildMiddleware(replace_parser, "*/libraries/WebServer/src/Parsing.cpp")


if "Import" in globals():
    Import("env")
    register_patch(env)
