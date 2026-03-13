# CMake generated Testfile for 
# Source directory: D:/chamoix-engine/.external/tinyxml2
# Build directory: D:/chamoix-engine/out/.external/tinyxml2
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(xmltest "D:/chamoix-engine/out/.external/tinyxml2/Debug/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  PASS_REGULAR_EXPRESSION ", Fail 0" WORKING_DIRECTORY "D:/chamoix-engine/.external/tinyxml2" _BACKTRACE_TRIPLES "D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;49;add_test;D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(xmltest "D:/chamoix-engine/out/.external/tinyxml2/Release/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  PASS_REGULAR_EXPRESSION ", Fail 0" WORKING_DIRECTORY "D:/chamoix-engine/.external/tinyxml2" _BACKTRACE_TRIPLES "D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;49;add_test;D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(xmltest "D:/chamoix-engine/out/.external/tinyxml2/MinSizeRel/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  PASS_REGULAR_EXPRESSION ", Fail 0" WORKING_DIRECTORY "D:/chamoix-engine/.external/tinyxml2" _BACKTRACE_TRIPLES "D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;49;add_test;D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(xmltest "D:/chamoix-engine/out/.external/tinyxml2/RelWithDebInfo/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  PASS_REGULAR_EXPRESSION ", Fail 0" WORKING_DIRECTORY "D:/chamoix-engine/.external/tinyxml2" _BACKTRACE_TRIPLES "D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;49;add_test;D:/chamoix-engine/.external/tinyxml2/CMakeLists.txt;0;")
else()
  add_test(xmltest NOT_AVAILABLE)
endif()
