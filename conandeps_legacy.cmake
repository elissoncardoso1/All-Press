message(STATUS "Conan: Using CMakeDeps conandeps_legacy.cmake aggregator via include()")
message(STATUS "Conan: It is recommended to use explicit find_package() per dependency instead")

find_package(OpenSSL)
find_package(nlohmann_json)
find_package(spdlog)
find_package(Crow)
find_package(Boost)
find_package(GTest)

set(CONANDEPS_LEGACY  openssl::openssl  nlohmann_json::nlohmann_json  spdlog::spdlog  Crow::Crow  boost::boost  gtest::gtest )