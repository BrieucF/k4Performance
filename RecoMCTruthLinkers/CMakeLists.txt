#[[
Copyright (c) 2020-2024 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

file(GLOB _plugin_sources components/*.cpp)
gaudi_add_module(k4PerformancePlugins
                 SOURCES ${_plugin_sources}
                 LINK Gaudi::GaudiKernel
                      k4FWCore::k4FWCore
                      EDM4HEP::edm4hep
                      )

install(TARGETS k4PerformancePlugins
  EXPORT ${CMAKE_PROJECT_NAME}Targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/@{CMAKE_PROJECT_NAME}"
  COMPONENT dev)

include(CTest)

function(set_test_env _testname)
  set_property(TEST ${_testname} APPEND PROPERTY ENVIRONMENT
    LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}:$<TARGET_FILE_DIR:k4PerformancePlugins>:$<TARGET_FILE_DIR:ROOT::Core>:$<TARGET_FILE_DIR:k4FWCore::k4FWCore>:$<TARGET_FILE_DIR:EDM4HEP::edm4hep>:$<TARGET_FILE_DIR:podio::podio>:$ENV{LD_LIBRARY_PATH}
    PYTHONPATH=${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/genConfDir:$<TARGET_FILE_DIR:k4FWCore::k4FWCore>/../python:$ENV{PYTHONPATH}
    PATH=$<TARGET_FILE_DIR:k4FWCore::k4FWCore>/../bin:$ENV{PATH}
    K4PROJECTTEMPLATE=${CMAKE_CURRENT_LIST_DIR}/
    )
endfunction()

#add_test(NAME CreateExampleEventData
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               COMMAND k4run k4Performance/options/createExampleEventData.py)
#set_test_env(CreateExampleEventData)


