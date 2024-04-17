cmake_minimum_required(VERSION 3.28)
if (WIN32)
    project(ArkModIC LANGUAGES CXX)
elseif(UNIX)
    project(ArkModIC)
endif()
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(LIBRAIRIES_DIR "${CMAKE_SOURCE_DIR}/!libsanddlls/libs-include")
set(ALL_INCLUDE_DIR "${LIBRAIRIES_DIR}/ThreadedLoggerForCPP-main/libs/include")

set(SOURCES_FILES ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

#======================= INCLUSION OF Qt =======================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR})
find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${FORMS_DIR}/*.qrc"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

# Add the executable
if (WIN32) 
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
elseif(UNIX)
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()
# Add the target includes for ArkModIC 
target_include_directories(${PROJECT_NAME} PRIVATE ${ALL_INCLUDE_DIR} ${FORMS_DIR} ${INCLUDE_DIR} ${SOURCE_DIR})

# Récupérer le chemin des DLLs de Qt à partir de la variable système QTDIR
set(Qt_DLL_Path $ENV{QTDIR}/bin)

# Récupérer le chemin des DLLs et des plugins de Qt à partir de la variable système QTDIR
set(Qt_Bin_Path $ENV{QTDIR}/bin)
set(Qt_Plugins_Path $ENV{QTDIR}/plugins)

# Liste des DLLs nécessaires
set(DLL_List
    "Qt6Core.dll"
    "Qt6Network.dll"
    "Qt6Widgets.dll"
    "Qt6Gui.dll"
)

# Liste des plugins nécessaires
set(Plugins_List
    "platforms"
    "styles"
    # Ajoutez d'autres répertoires de plugins si nécessaire
)

# Chemin de destination (dossier de sortie de l'exécutable)
set(Destination_Path "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# Copier les DLLs nécessaires après la construction si elles n'existent pas déjà
foreach(DLL ${DLL_List})
    # Chemin complet de la DLL
    set(DLL_Path "${Qt_Bin_Path}/${DLL}")

    # Copier la DLL uniquement si elle n'existe pas déjà dans le dossier de sortie
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_Path}"
        "${Destination_Path}"
        COMMENT "Copying ${DLL}"
    )
endforeach()

# Copier les plugins nécessaires après la construction
foreach(Plugin ${Plugins_List})
    # Chemin complet du répertoire de plugins
    set(Plugin_Path "${Qt_Plugins_Path}/${Plugin}")

    # Copier le répertoire de plugins uniquement s'il n'existe pas déjà dans le dossier de sortie
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${Plugin_Path}"
        "${Destination_Path}/${Plugin}"
        COMMENT "Copying ${Plugin}"
    )
endforeach()

#===================== LINKING LIBRARIES =======================#
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Network Qt6::Widgets)


#===================== GENERATE COMPILATIONS DATABASE =======================#
set(common_compile_options
   -std=c++${CMAKE_CXX_STANDARD}
   -Wall 
   -compilerandversion=${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}
   -Wmicrosoft 
   -Wno-invalid-token-paste 
   -Wno-unknown-pragmas 
   -Wno-unused-value 
   -fsyntax-only 
   "\\\"-D_MT\\\"" 
   "\\\"-D_DLL\\\"" 
   "\\\"-DWIN32\\\"" 
   "\\\"-D_WINDOWS\\\"" 
   "\\\"-DCMAKE_INTDIR=/\\\"Debug/\\\"\\\"" 
   "\\\"-D_DEBUG_FUNCTIONAL_MACHINERY\\\""   
)

# Add each include directory to the compilation options
foreach(INCLUDE_DIR ${ALL_INCLUDE_DIR})
    list(APPEND common_compile_options -I\\\"${INCLUDE_DIR}\\\")
endforeach()

# Add an extra quote to the last element
list(LENGTH ALL_INCLUDE_DIR INCLUDE_DIR_COUNT)
math(EXPR LAST_INDEX "${INCLUDE_DIR_COUNT} - 1")
list(GET ALL_INCLUDE_DIR ${LAST_INDEX} LAST_INCLUDE_DIR)
string(CONCAT LAST_INCLUDE_DIR_WITH_QUOTE ${LAST_INCLUDE_DIR} "\"")
list(POP_BACK common_compile_options)
list(APPEND common_compile_options ${LAST_INCLUDE_DIR_WITH_QUOTE})

# Convert the list to a string with spaces between the options
string(REPLACE ";" " " common_compile_options_string "${common_compile_options}")

# Filter the source files to keep only .cpp and .hpp files
list(FILTER SOURCES INCLUDE REGEX "\\.(cpp|hpp)$")

set(compile_commands "")
foreach(SOURCE_FILE ${SOURCES})
    # Create a JSON string for each .cpp or .hpp source file
    string(APPEND compile_commands
        "    {\n"
        "        \"directory\": \"${CMAKE_CURRENT_BINARY_DIR}\",\n"
        "        \"command\": \"\\\"${CMAKE_CXX_COMPILER}\\\" -x c++ \\\"${SOURCE_FILE}\\\""
    )

    # Add the common compile options
    foreach(OPTION ${common_compile_options})
        string(APPEND compile_commands " ${OPTION}") 
    endforeach()

    # Add the rest of the command and JSON metadata
    string(APPEND compile_commands
        ",\n"
        "        \"file\": \"${SOURCE_FILE}\"\n"
        "    },\n"
    )
endforeach()

# Remove the final comma
string(REGEX REPLACE ",\n$" "" compile_commands "${compile_commands}")

# Add brackets to form a JSON list
set(compile_commands "[\n${compile_commands}\n]")

# Write the entries to a JSON file
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json "${compile_commands}")

# Add the custom target to generate compile_commands.json
add_custom_target(generate_compile_commands
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    COMMENT "Generating compile_commands.json"
)
