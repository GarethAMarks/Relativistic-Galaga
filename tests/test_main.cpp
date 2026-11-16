#include <cstdio>
#include <cstdlib>

// Forward declarations of test suites defined in other files.
bool testLorentz();
bool testAberration();
bool testDoppler();

int main() {
    int failures = 0;

    auto run = [&](const char* name, bool (*fn)()) {
        bool ok = fn();
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
        if (!ok) ++failures;
    };

    run("Lorentz",     testLorentz);
    run("Aberration",  testAberration);
    run("Doppler",     testDoppler);

    std::printf("\n%s  (%d failure(s))\n",
                failures == 0 ? "ALL PASS" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}
