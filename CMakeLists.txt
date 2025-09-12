cmake_minimum_required(
   VERSION 3.27)

set(
   CMAKE_CXX_STANDARD
   20)
set(
   CMAKE_CXX_STANDARD_REQUIRED
   yes)
set(
   CMAKE_CXX_EXTENSIONS
   no)

set(
   CMAKE_AUTOMOC
   on)
set(
   CMAKE_AUTORCC
   on)

project(
   math-facts)

set(
   target_name
   math-facts)

add_executable(
   ${target_name}
   WIN32
      arithmetic-problem.cpp
      arithmetic-problem.hpp
      main.cpp
      mainicon.ico
      math-facts.ini
      math-facts.qrc
      math-facts.rc
      math-facts-widget.cpp
      math-facts-widget.hpp
      problem.cpp
      problem.hpp
      time-problem.cpp
      time-problem.hpp)

find_package(
   Qt6
   QUIET
   COMPONENTS
      Core
      Gui
      Widgets
      UiTools)

if (NOT Qt6_FOUND)

   find_package(
      Qt5
      REQUIRED
      COMPONENTS
         Core
         Gui
         Widgets
         UiTools)

endif ( )

set(
   qt_version_major
   ${Qt5_VERSION_MAJOR}${Qt6_VERSION_MAJOR})

target_link_libraries(
   ${target_name}
   PRIVATE
      Qt::Core
      Qt::Gui
      Qt::Widgets
      Qt::UiTools)

string(
   CONCAT
   vs_debugger_environment_gexpr
   "PATH=%PATH%;"
   "$<IF:"
      "$<CONFIG:debug>,"
         "$<PATH:GET_PARENT_PATH,$<TARGET_PROPERTY:Qt${qt_version_major}::Core,IMPORTED_LOCATION_DEBUG>>;"
         "$<PATH:GET_PARENT_PATH,$<TARGET_PROPERTY:Qt${qt_version_major}::Core,IMPORTED_LOCATION_DEBUG>>/../plugins,"
         ""
         "$<PATH:GET_PARENT_PATH,$<TARGET_PROPERTY:Qt${qt_version_major}::Core,IMPORTED_LOCATION>>;"
         "$<PATH:GET_PARENT_PATH,$<TARGET_PROPERTY:Qt${qt_version_major}::Core,IMPORTED_LOCATION>>/../plugins"
   ">")

set_target_properties(
   ${target_name}
   PROPERTIES
      VS_DEBUGGER_ENVIRONMENT "${vs_debugger_environment_gexpr}")

include(GNUInstallDirs)

install(
   TARGETS
      ${target_name}
   RUNTIME_DEPENDENCY_SET "Qt")

install(
   FILES
      math-facts.ini
   DESTINATION
      "${CMAKE_INSTALL_BINDIR}")

if (WIN32)

   string(
      CONCAT
      qt_core_import_directory_absolute
      "$<PATH:GET_PARENT_PATH,"
         "$<TARGET_PROPERTY:"
            "Qt${qt_version_major}::Core,"
            "IMPORTED_LOCATION$<UPPER_CASE:_$<CONFIG>>"
         ">"
      ">")

   install(
      RUNTIME_DEPENDENCY_SET "Qt"
      PRE_EXCLUDE_REGEXES
         "api-ms-"
         "ext-ms-"
      POST_EXCLUDE_REGEXES
         ".*system32/.*\\.dll"
      DIRECTORIES
         "${qt_core_import_directory_absolute}")

   install(
      DIRECTORY
         "${qt_core_import_directory_absolute}/../Qt6"
      DESTINATION "${CMAKE_INSTALL_BINDIR}")

endif ( )
