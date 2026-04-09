
cmake_minimum_required(VERSION 3.16)

project(UQAC_OPTI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/DEV/Qt2/6.11.0/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(QTapp
        PROJET1/MainQT.cpp
        PROJET1/src/NomadSolver.cpp
)

qt_add_qml_module(QTapp
        URI UQAC_OPTI
        QML_FILES
        PROJET1/MainQT.qml
        SOURCES PROJET1/src/centraleQTWrap.h
)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)


FetchContent_Declare(
  OpenXLSX
  GIT_REPOSITORY https://github.com/troldal/OpenXLSX.git
  GIT_TAG      master #ou autre tag
)
set(OPENXLSX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(OPENXLSX_BUILD_TESTS OFF CACHE INTERNAL "")
set(OPENXLSX_BUILD_DOCS OFF CACHE INTERNAL "")


FetchContent_MakeAvailable(OpenXLSX)

find_package(OpenMP REQUIRED)

set(NOMAD_HOME_PATH $ENV{NOMAD_HOME})
set(SGTELIB "${NOMAD_HOME_PATH}/ext/sgtelib")

# 2. Vérification de sécurité (pour éviter les erreurs de lien cryptiques)
if(NOT NOMAD_HOME_PATH)
    message(FATAL_ERROR "La variable d'environnement NOMAD_HOME n'est pas définie.
    Assurez-vous de l'avoir ajoutée à votre système.")
endif()

message(STATUS "NOMAD trouvé à l'adresse : ${NOMAD_HOME_PATH}")

include_directories("${NOMAD_HOME_PATH}/src"
        "${SGTELIB}/src")
link_directories("${NOMAD_HOME_PATH}/bin")

add_executable(UqacOptiProjetHiv2026 PROJET1/Main.cpp
        PROJET1/src/Calculator.cpp
        PROJET1/src/Centrale.cpp
        PROJET1/src/DPResourceAllocation.cpp
        PROJET1/src/DPResourceAllocationFast.cpp
        "PROJET1/src/ResourceAllocationSolver.cpp"
        )
add_executable(NomadExec PROJET1/NomadMain.cpp
        PROJET1/src/Calculator.cpp
        PROJET1/src/Centrale.cpp
        PROJET1/src/DPResourceAllocation.cpp
        PROJET1/src/DPResourceAllocationFast.cpp
        "PROJET1/src/ResourceAllocationSolver.cpp"
        PROJET1/src/NomadEvaluator.hpp
)
target_link_libraries(UqacOptiProjetHiv2026 PRIVATE OpenXLSX::OpenXLSX)
target_compile_features(UqacOptiProjetHiv2026 PRIVATE cxx_std_17)
target_compile_features(NomadExec PRIVATE cxx_std_17)
target_link_libraries(NomadExec "C:/Program Files/DEV/nomad.3.9.1/bin/nomad.lib" )


set_target_properties(QTapp PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(QTapp
        PRIVATE Qt6::Quick
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        "C:/Program Files/DEV/nomad.3.9.1/bin/nomad.lib"
)

install(TARGETS QTapp
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
# Commande pour copier le dossier data vers le dossier de build
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/PROJET1/data"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command(TARGET NomadExec POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NOMAD_HOME_PATH}/bin/nomad.dll"
        $<TARGET_FILE_DIR:NomadExec>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NOMAD_HOME_PATH}/bin/sgtelib.dll"
        $<TARGET_FILE_DIR:NomadExec>
)

find_program(SIGNTOOL_EXECUTABLE signtool)

if(SIGNTOOL_EXECUTABLE)
    message(STATUS "Signtool trouvé : ${SIGNTOOL_EXECUTABLE}")

    # 2. Paramètres de signature (À adapter selon ton certificat)
    set(CERTIFICATE_PATH "HandmadeDev")
    set(CERTIFICATE_PASSWORD "")

    # 3. Commande de signature après le build
    add_custom_command(TARGET QTapp POST_BUILD
            COMMAND "${SIGNTOOL_EXECUTABLE}" sign /a /s My /n "${CERTIFICATE_PATH}"
            /fd SHA256
            /v "$<TARGET_FILE:QTapp>"
            COMMENT "Signature de l'exécutable NomadExec avec Signtool..."
            VERBATIM
    )
else()
    message(WARNING "Signtool non trouvé. L'exécutable ne sera pas signé.")
endif()
