cmake_minimum_required(
   VERSION 3.27)

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
   main.cpp
   mainicon.ico
   math-facts.qrc
   math-facts.rc)

set(
   qt_version_major
   5)

find_package(
   Qt5
   QUIET
   COMPONENTS
      Core
      Gui
      Widgets
      UiTools)

if (NOT QT5_FOUND)
   find_package(
      Qt6
      REQUIRED
      COMPONENTS
         Core
         Gui
         Widgets
         UiTools)

   set(
      qt_version_major
      6)
endif ( )

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
