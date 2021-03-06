#!/bin/python3

import sys
import os
import subprocess
import copy
import shutil

is_windows = os.name == 'nt'
cls = lambda: os.system('cls' if is_windows else 'clear')
tool_build_folder = "build_tool"
tool_build_install_path = tool_build_folder + "/install"

class BuildSystem:
    def __init__( self, description, cmake_option, include_architecture ):
        self.description = description
        self.cmake_option = cmake_option
        self.include_architecture = include_architecture

    def GetOptionString( self ):
        ret = []
        if self.cmake_option:
            ret.append( "-G" )
            ret.append( self.cmake_option )
        if self.include_architecture:
            ret.append( "-A" )
            ret.append( "x64" )
        return ret

class BuildOption:
    def __init__( self, description, cmake_name, on_by_default ):
        self.description = description
        self.cmake_name = cmake_name
        if on_by_default:
            self.value = "ON"
        else:
            self.value = "OFF"

    def Toggle( self ):
        if self.value == "OFF":
            self.value = "ON"
        else:
            self.value = "OFF"

    def GetOptionString( self ):
        ret = []
        ret.append( "-D" )
        ret.append( self.cmake_name + "=" + self.value )
        return ret

build_systems       = []
build_options       = []
packages_windows    = []
packages_linux      = []



################################################################
### Build configurations and options.
################################################################

# Add build systems here.        ( Description,                                     CMake generator name,                   Add -A "x64"
build_systems.append( BuildSystem( "CMake default",                                 "",                                     False ) )
build_systems.append( BuildSystem( "Microsoft Visual Studio 2019",                  "Visual Studio 16 2019",                True ) )
build_systems.append( BuildSystem( "Microsoft Visual Studio 2017",                  "Visual Studio 15 2017 Win64",          False ) )
build_systems.append( BuildSystem( "Code::Blocks - Ninja",                          "CodeBlocks - Ninja",                   False ) )
build_systems.append( BuildSystem( "Code::Blocks - Unix Makefiles",                 "CodeBlocks - Unix Makefiles",          False ) )
default_build_system_index = 0

# Add build options here.        ( Description,                                     CMake option name,                      On by default
build_options.append( BuildOption( "Enable library debugging features",             "VK2D_ENABLE_LIBRARY_DEBUG_FEATURES",   False ) )
build_options.append( BuildOption( "Build static library (EXPERIMENTAL)",           "VK2D_BUILD_STATIC_LIBRARY",            False ) )
build_options.append( BuildOption( "Build tests",                                   "VK2D_BUILD_TESTS",                     False ) )
build_options.append( BuildOption( "Build examples",                                "VK2D_BUILD_EXAMPLES",                  True  ) )
build_options.append( BuildOption( "Build documentation (Requires Doxygen)",        "VK2D_BUILD_DOCS",                      False ) )
build_options.append( BuildOption( "Build documentation for entire source code",    "VK2D_BUILD_DOCS_FOR_COMPLETE_SOURCE",  False ) )

# Package build options here, per platform.
# Windows              ( Description,                           CPack generator name
packages_windows.append( [ "Create 7zip package",               "7Z" ] )

# Linux (debian)     ( Description,                             CPack generator name
packages_linux.append( [ "Create debian package",               "DEB" ] )
packages_linux.append( [ "Create 7zip package",                 "7Z" ] )
packages_linux.append( [ "Create .tar.bz2 package",             "TBZ2" ] )
packages_linux.append( [ "Create .tar.gz package",              "TGZ" ] )

quick_setup_build_configurations = []
if is_windows:
    quick_setup_build_configurations = [ "Release", "Debug" ]
else:
    quick_setup_build_configurations = [ "Release" ]



################################################################
### Configure and build project menus
################################################################
def SelectBuildSystem( quick_setup ):
    build_systems_copy = copy.deepcopy( build_systems )

    if quick_setup:
        return [ True, build_systems_copy[ default_build_system_index ].GetOptionString() ]

    while True:
        cls()
        print( "************************************************************" )
        print( "* Configure and build project files:" )
        print( "* " )
        print( "* Select build system:" )
        print( "* " )
        for i in range( 0, len( build_systems_copy ) ):
            bs = build_systems_copy[ i ]
            print( "* [ " + str( i + 1 ) + " ] " + bs.description )
        print( "* " )
        print( "* [ x ] Cancel" )
        print( "* " )
        print( "************************************************************" )
        print( "" )
        option = input( "Selection: " )
        option = option.lower()

        if option == "x":
            return [ False, [] ]
        elif option.isdigit():
            system_index = int( option ) - 1
            if system_index < len( build_systems_copy ):
                return [ True, build_systems_copy[ system_index ].GetOptionString() ]

def SelectBuildOptions( quick_setup ):
    build_options_copy = copy.deepcopy( build_options )

    if quick_setup:
        ret = []
        for b in build_options_copy:
            ret += b.GetOptionString()
        return [ True, ret ]

    while True:
        cls()
        print( "************************************************************" )
        print( "* Configure and build project files:" )
        print( "* " )
        print( "* Select build options:" )
        print( "* " )
        for i in range( 0, len( build_options_copy ) ):
            bo = build_options_copy[ i ]
            print( "* [ " + str( i + 1 ) + " ] " + bo.description + ":", bo.value )
        print( "* " )
        print( "* [ b ] Build" )
        print( "* [ x ] Cancel" )
        print( "* " )
        print( "************************************************************" )
        print( "" )
        option = input( "Selection: " )
        option = option.lower()

        if option == "b":
            ret = []
            for b in build_options_copy:
                ret += b.GetOptionString()
            return [ True, ret ]
        elif option == "x":
            print( "Exiting..." )
            return [ False, [] ]
        elif option.isdigit():
            option_index = int( option ) - 1
            if option_index < len( build_options_copy ):
                build_options_copy[ option_index ].Toggle()

def SelectBuildType():
    while True:
        cls()
        print( "************************************************************" )
        print( "* Configure and build project files:" )
        print( "* " )
        print( "* Select build type:" )
        print( "* " )
        print( "* [ 1 ] Release" )
        print( "* [ 2 ] Debug" )
        print( "* " )
        print( "* [ d ] Library development (in \"build\" directory)" )
        print( "* " )
        print( "* [ x ] Cancel" )
        print( "* " )
        print( "************************************************************" )
        print( "" )
        option = input( "Selection: " )
        option = option.lower()

        if option == "1":
            return [ True, [ "Release" ] ]
        elif option == "2":
            return [ True, [ "Debug" ] ]
        elif option == "d":
            return [ True, [ "dev" ] ]
        elif option == "x":
            return [ False, [] ]


def ConfigureAndBuildProjectMenu( quick_setup = False ):
    build_type = []
    if quick_setup:
        build_type = [ True, quick_setup_build_configurations ]
    else:
        build_type = SelectBuildType()
        if not build_type[ 0 ]:
            return

    build_sys = SelectBuildSystem( quick_setup )
    if not build_sys[ 0 ]:
        return

    build_opt = SelectBuildOptions( quick_setup )
    if not build_opt[ 0 ]:
        return

    for bt in build_type[ 1 ]:
        build_path = tool_build_folder + "/" + bt
        is_dev_build = False

        # If this is a build branch, use a different build path
        if bt == "dev":
            is_dev_build = True
            build_path = "build"
        else:
            if not os.path.exists( tool_build_folder ):
                os.mkdir( tool_build_folder )


        call_parameters = [ "cmake" ]
        if build_sys:
            call_parameters += build_sys[ 1 ]
        call_parameters += [ "-Wno-dev" ]
        call_parameters += build_opt[ 1 ]
        if is_dev_build:
            call_parameters += [ "-D", "CMAKE_INSTALL_PREFIX=./install/" ]
        else:
            call_parameters += [ "-D", "CMAKE_INSTALL_PREFIX=./" + tool_build_install_path ]
        if not is_dev_build:
            call_parameters += [ "-D", 'CMAKE_BUILD_TYPE=' + bt ]
        call_parameters += [ "-D", "VK2D_SEPARATE_DEBUG_NAME=ON" ]
        call_parameters += [ "-S", "." ]
        call_parameters += [ "-B", build_path ]

        if not os.path.exists( build_path ):
            os.mkdir( build_path )

        subprocess.run( call_parameters )

    if not quick_setup:
        print( "\n\nDone." )
        input( "Press enter..." )



################################################################
### Compile menu
################################################################
def CompileMenu( quick_setup = False ):
    if quick_setup:
        for c in quick_setup_build_configurations:
            subprocess.run( [ "cmake", "--build", tool_build_folder + "/" + c, "--config", c] )
    else:
        build_type = ""
        while True:
            cls()
            print( "************************************************************" )
            print( "* Compile menu:" )
            print( "* " )
            print( "* Select task." )
            print( "* " )
            print( "* [ 1 ] Compile Release" )
            print( "* [ 2 ] Compile Debug" )
            print( "* " )
            print( "* [ x ] Back" )
            print( "* " )
            print( "************************************************************" )
            print( "" )
            option = input( "Selection: " )
            option = option.lower()

            if option == "x":
                return
            elif option == "1":
                build_type = "Release"
                break
            elif option == "2":
                build_type = "Debug"
                break

        call_parameters = [ "cmake" ]
        call_parameters += [ "--build", tool_build_folder + "/" + build_type ]
        call_parameters += [ "--config", build_type ]
        subprocess.run( call_parameters )
        print( "\n\nDone." )
        input( "Press enter..." )



################################################################
### Install menu
################################################################
def InstallMenu( quick_setup = False ):
    if quick_setup:
        for c in quick_setup_build_configurations:
            subprocess.run( [ "cmake", "--install", tool_build_folder + "/" + c, "--config", c] )
    else:
        build_type = ""
        while not quick_setup:
            cls()
            print( "************************************************************" )
            print( "* Install menu:" )
            print( "* " )
            print( "* Select task." )
            print( "* " )
            print( "* [ 1 ] Install Release" )
            print( "* [ 2 ] Install Debug" )
            print( "* " )
            print( "* [ x ] Back" )
            print( "* " )
            print( "************************************************************" )
            print( "" )
            option = input( "Selection: " )
            option = option.lower()

            if option == "x":
                return
            elif option == "1":
                build_type = "Release"
                break
            elif option == "2":
                build_type = "Debug"
                break

        call_parameters = [ "cmake" ]
        call_parameters += [ "--install", tool_build_folder + "/" + build_type ]
        call_parameters += [ "--config", build_type ]
        subprocess.run( call_parameters )
        print( "\n\nDone." )
        input( "Press enter..." )



################################################################
### Create package menu
################################################################
def CreatePackageMenu():
    package_options = []
    if is_windows:
        package_options = packages_windows
    else:
        package_options = packages_linux
    
    package_type = ""
    while True:
        cls()
        print( "************************************************************" )
        print( "* Main menu:" )
        print( "* " )
        print( "* Select task" )
        print( "* " )
        for i in range( 0, len( package_options ) ):
            po = package_options[ i ]
            print( "* [ " + str( i + 1 ) + " ] " + po[ 0 ] )
        print( "* " )
        print( "* [ x ] Back" )
        print( "* " )
        print( "************************************************************" )
        print( "" )
        option = input( "Selection: " )
        option = option.lower()

        if option == "x":
            return
        elif option.isdigit():
            option_index = int( option ) - 1
            if option_index < len( package_options ):
                po = package_options[ option_index ]
                package_type = po[ 1 ]
                break
    
    packaging_source_path = "Packaging"
    packaging_build_path = tool_build_folder + "/" + packaging_source_path
    
    #shutil.rmtree( tool_build_folder + "/" + packaging_source_path )

    build_parameters = [ "cmake" ]
    build_parameters += [ "-Wno-dev" ]
    build_parameters += [ "-D", "CMAKE_INSTALL_PREFIX=./" + tool_build_install_path ]
    build_parameters += [ "-S", packaging_source_path ]
    build_parameters += [ "-B", packaging_build_path ]
    subprocess.run( build_parameters )

    install_parameters = [ "cmake" ]
    install_parameters += [ "--install", packaging_build_path ]
    install_parameters += [ "--config", "Release" ]
    subprocess.run( install_parameters )

    # This is a bit hacky, remporarily changing working directory for cpack.
    current_dir = os.getcwd()
    os.chdir( packaging_build_path )
    cpack_parameters = [ "cpack" ]
    cpack_parameters += [ "-G", package_type ]
    #cpack_parameters += [ "-B", "install_packages" ]
    subprocess.run( cpack_parameters )
    os.chdir( current_dir )

    print( "\n\nDone." )
    input( "Press enter..." )



################################################################
### Main menu
################################################################
def MainMenu():
    while True:
        cls()
        print( "************************************************************" )
        print( "* Main menu:" )
        print( "* " )
        print( "* Select task" )
        print( "* " )
        print( "* [ q ] Quick build and install using default settings" )
        print( "* " )
        print( "* [ 1 ] Configure and build project files" )
        print( "* [ 2 ] Compile" )
        print( "* [ 3 ] Install (into local folder)" )
        print( "* " )
        print( "* [ p ] Create install package" )
        print( "* " )
        print( "* [ x ] Exit" )
        print( "* " )
        print( "************************************************************" )
        print( "" )
        option = input( "Selection: " )
        option = option.lower()

        if option == "x":
            return
        elif option == "q":
            ConfigureAndBuildProjectMenu( True )
            CompileMenu( True )
            InstallMenu( True )
            print( "Done." )
            input( "Press enter..." )
        elif option == "p":
            CreatePackageMenu()
        elif option == "1":
            ConfigureAndBuildProjectMenu()
        elif option == "2":
            CompileMenu()
        elif option == "3":
            InstallMenu()


if __name__ == "__main__":
    MainMenu()
