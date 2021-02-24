cmake_minimum_required(VERSION 3.13 FATAL_ERROR) #upgrade to 17 if we use conan

project(example-service LANGUAGES CXX)
set(PROJECT_VERSION "0.1")
set(CMAKE_CXX_STANDARD 11)
set(PROJECT_APP "example-service")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

find_package(Boost 1.62
	COMPONENTS system
	REQUIRED
)

find_package(PkgConfig)
pkg_check_modules(DBUS_CPP dbus-cpp
	REQUIRED
)
pkg_check_modules(DBUS dbus-1
	REQUIRED
)

link_directories(${DBUS_CPP_LIBRARY_DIRS} ${DBUS_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS})

include_directories(
  ${Boost_INCLUDE_DIRS}
	${DBUS_CPP_INCLUDE_DIRS}
	${DBUS_INCLUDE_DIRS}
)

add_executable(${PROJECT_APP}
	main.cpp
)

set(PLATFORM_LIBS "-ldl -latomic -lpthread")
target_link_libraries(${PROJECT_APP}
  ${Boost_LIBRARIES}
	${PLATFORM_LIBS}
	${DBUS_CPP_LIBRARIES}
	${DBUS_LIBRARIES}
)

install(DIRECTORY DESTINATION bin)
install(TARGETS ${PROJECT_APP} DESTINATION bin)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/info.x37v.example.conf
  DESTINATION share/dbus-1/session.d/
  PERMISSIONS
  OWNER_WRITE OWNER_READ
  GROUP_READ
  WORLD_READ
  )

#setup deb build
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  INCLUDE(InstallRequiredSystemLibraries)

  SET(CPACK_GENERATOR "DEB")

  SET(CPACK_PACKAGE_DESCRIPTION "example")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "example.")
  SET(CPACK_PACKAGE_VENDOR "xnor")
  SET(CPACK_PACKAGE_CONTACT "alex@x37v.info")

  SET(CPACK_DEBIAN_PACKAGE_VERSION "${PROJECT_VERSION}")

  SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}")

  SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.28)")

  SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
  SET(CPACK_DEBIAN_PACKAGE_SECTION "admin")

  INCLUDE(CPack)

ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
