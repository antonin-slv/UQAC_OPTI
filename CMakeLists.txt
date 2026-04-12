
cmake_minimum_required(VERSION 3.16)

project(UQAC_OPTI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_PREFIX_PATH "C:/DEV/Qt2/6.11.0/msvc2022_64")


find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.10)

set(NOMAD_HOME_PATH $ENV{NOMAD_HOME})
set(SGTELIB "${NOMAD_HOME_PATH}/ext/sgtelib")

# 2. Vérification de sécurité (pour éviter les erreurs de lien cryptiques)
if(NOT NOMAD_HOME_PATH)
    message(FATAL_ERROR "La variable d'environnement NOMAD_HOME n'est pas définie.
    Assurez-vous de l'avoir ajoutée à votre système.")
endif()


qt_add_executable(QTapp
        PROJET1/MainQT.cpp
        PROJET1/src/centraleQTWrap.h
        PROJET1/src/SolverQtWrappers.h
        PROJET1/src/SolverManagerQt.h
        PROJET1/src/SolverNomadQTWrapper.h
        #PROJET1/src/NomadEvaluator.hpp

)

qt_add_qml_module(QTapp
        URI UQAC_OPTI
        VERSION 1.2
        RESOURCE_PREFIX "/"
        QML_FILES
            PROJET1/MainQT.qml
            PROJET1/ui/DPSettings.qml
            PROJET1/ui/NomadSettings.qml
        SOURCES
            PROJET1/src/centraleQTWrap.h
            PROJET1/src/SolverQtWrappers.h
            PROJET1/src/SolverManagerQt.h
            PROJET1/src/SolverNomadQTWrapper.h
)
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
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(QTapp PRIVATE
        "PROJET1/src"
        "${NOMAD_HOME_PATH}/src"
        "${SGTELIB}/src")
link_directories("${NOMAD_HOME_PATH}/bin")

add_executable(UqacOptiProjetHiv2026 PROJET1/Main.cpp
        PROJET1/src/Calculator.cpp
        PROJET1/src/Centrale.cpp
        PROJET1/src/DPResourceAllocation.cpp
        PROJET1/src/DPResourceAllocationFast.cpp
        "PROJET1/src/ResourceAllocationSolver.cpp"
        )
#target_link_libraries(UqacOptiProjetHiv2026 PRIVATE OpenXLSX::OpenXLSX)

target_link_libraries(QTapp PRIVATE
        Qt6::Quick
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::QuickControls2
        "C:/Program Files/DEV/nomad.3.9.1/bin/nomad.lib"
)

#set_target_properties(QTapp PROPERTIES WIN32_EXECUTABLE FALSE )

install(TARGETS QTapp
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Commande pour copier le dossier data vers le dossier de build
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/PROJET1/data"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

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
            COMMENT "Signature de l'exécutable avec Signtool..."
            VERBATIM

    )
else()
    message(WARNING "Signtool non trouvé. L'exécutable ne sera pas signé.")
endif()

add_custom_command(TARGET QTapp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NOMAD_HOME_PATH}/bin/nomad.dll"
        $<TARGET_FILE_DIR:QTapp>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NOMAD_HOME_PATH}/bin/sgtelib.dll"
        $<TARGET_FILE_DIR:QTapp>
)

###
add_custom_command(TARGET QTapp POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
        --verbose 0
        --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/PROJET1"
        "$<TARGET_FILE:QTapp>"
        COMMENT "Déploiement des dépendances Qt via windeployqt..."
)

if(NOT WINDEPLOYQT_EXECUTABLE)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
endif()

# 2. Définition du dossier de sortie (à la racine de ton build)
set(DEPLOY_ROOT_DIR "${CMAKE_BINARY_DIR}/DISTRIBUTION")
set(DEPLOY_BIN_DIR "${DEPLOY_ROOT_DIR}/bin")

add_custom_target(deploy_package
        # Nettoyage du dossier précédent
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DEPLOY_ROOT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_BIN_DIR}"

        # Copie de l'exécutable
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:QTapp>" "${DEPLOY_BIN_DIR}/"

        # Copie des DLL Nomad et Sgtelib
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NOMAD_HOME_PATH}/bin/nomad.dll" "${DEPLOY_BIN_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NOMAD_HOME_PATH}/bin/sgtelib.dll" "${DEPLOY_BIN_DIR}/"

        # Copie du dossier data
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/PROJET1/data" "${DEPLOY_ROOT_DIR}/data"

        # Exécution de windeployqt sur le dossier de distribution
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
        --verbose 0
        --no-compiler-runtime
        --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/PROJET1"
        "${DEPLOY_BIN_DIR}/QTapp.exe"

        # Création du lanceur .bat à la racine (pour ne pas voir les DLL)
        COMMAND ${CMAKE_COMMAND} -E echo "@echo off" > "${DEPLOY_ROOT_DIR}/Lancer_Application.bat"
        COMMAND ${CMAKE_COMMAND} -E echo "start bin/QTapp.exe" >> "${DEPLOY_ROOT_DIR}/Lancer_Application.bat"

        COMMENT "Création du package de distribution dans ${DEPLOY_ROOT_DIR}..."
)
