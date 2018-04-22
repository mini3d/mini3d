// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#define MINI3D_TEST
#ifdef MINI3D_TEST

#include <vector>
#include <string>

#include "math/uvec3.hpp"

using namespace std;

int main() {

    vector<pair<const char*, vector<pair<const char*, bool(*)()>>>> suites = {
        { "mini3d_math/vec3.cpp", math_uvec3 } };

    int pass = 0;
    int fail = 0;
    
	for (auto suite : suites) {
        printf("Begin test suite: %s ------ \n\n", suite.first);
        
        int suitePass = 0;
        int suiteFail = 0;
        
		for (auto test : suite.second) {
            bool result = test.second();
            (result ? suitePass : suiteFail)++;
            printf("%s: %s\n", test.first, result ? "pass" : "fail");
		}
        printf("Result test suite %s (pass/fail):  %d/%d\n\n", suite.first, suitePass, suiteFail);
        
        pass += suitePass;
        fail += suiteFail;
	}
    printf("Result Total (pass/fail):  %d/%d\n", pass, fail);
    
    return 0;
}

#endif
