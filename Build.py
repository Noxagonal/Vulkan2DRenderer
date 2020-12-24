#!/bin/python3

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

build_systems = []
build_options = []



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
build_options.append( BuildOption( "Enable library debugging options",              "VK2D_DEBUG",                           False ) )
build_options.append( BuildOption( "Build static library (EXPERIMENTAL)",           "VK2D_BUILD_STATIC_LIBRARY",            False ) )
build_options.append( BuildOption( "Build tests",                                   "VK2D_BUILD_TESTS",                     False ) )
build_options.append( BuildOption( "Build examples",                                "VK2D_BUILD_EXAMPLES",                  True  ) )
build_options.append( BuildOption( "Build documentation (Requires Doxygen)",        "VK2D_BUILD_DOCS",                      False ) )
build_options.append( BuildOption( "Build documentation for entire source code",    "VK2D_BUILD_DOCS_FOR_COMPLETE_SOURCE",  False ) )



import sys
import os
import subprocess
import copy
import shutil

is_windows = os.name == 'nt'
cls = lambda: os.system('cls' if is_windows else 'clear')
tool_build_folder = "build_tool"
tool_build_install_path = tool_build_folder + "/install"



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
        print( "* Select build system." )
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
        print( "* Select build options." )
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
        print( "* Select build type." )
        print( "* " )
        print( "* [ 1 ] Release" )
        print( "* [ 2 ] Debug" )
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
        elif option == "x":
            return [ False, [] ]


def ConfigureAndBuildProjectMenu( quick_setup = False ):
    build_type = []
    if quick_setup:
        if is_windows:
            # On windows, build both release and debug by default
            build_type = [ True, [ "Release", "Debug" ] ]
        else:
            # On linux, build only release by default
            build_type = [ True, [ "Release" ] ]
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

    if not os.path.exists( tool_build_folder ):
        os.mkdir( tool_build_folder )

    for bt in build_type[ 1 ]:
        build_path = tool_build_folder + "/" + bt

        call_parameters = [ "cmake" ]
        if build_sys:
            call_parameters += build_sys[ 1 ]
        call_parameters += [ "-Wno-dev" ]
        call_parameters += build_opt[ 1 ]
        if is_windows:
            # On windows, make sure we're installing into a folder.
            # Might expand this to Linux as well in the future when creating packages.
            call_parameters += [ "-D", "CMAKE_INSTALL_PREFIX=./" + tool_build_install_path ]
        call_parameters += [ "-D", 'CMAKE_BUILD_TYPE=' + bt ]
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
        if is_windows:
            # Build both release and debug versions for Windows.
            subprocess.run( [ "cmake", "--build", tool_build_folder + "/Release", "--config", "Release" ] )
            subprocess.run( [ "cmake", "--build", tool_build_folder + "/Debug", "--config", "Debug" ] )
        else:
            # Build only release version for Linux.
            subprocess.run( [ "cmake", "--build", tool_build_folder + "/Release", "--config", "Release" ] )
    else:
        build_type = ""
        while True:
            cls()
            print( "************************************************************" )
            print( "* Compile menu:" )
            print( "* " )
            print( "* Select task." )
            print( "* " )
            print( "* [ 1 ] Compile release" )
            print( "* [ 2 ] Compile debug" )
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
        subprocess.run(  )
        print( "\n\nDone." )
        input( "Press enter..." )



################################################################
### Install menu
################################################################
def InstallMenu( quick_setup = False ):
    if not is_windows:
        # No installing on Linux for now. TODO for later...
        return

    if quick_setup:
        if is_windows:
            # Build both release and debug versions for Windows.
            subprocess.run( [ "cmake", "--install", tool_build_folder + "/Release", "--config", "Release" ] )
            subprocess.run( [ "cmake", "--install", tool_build_folder + "/Debug", "--config", "Debug" ] )
        else:
            # Build only release version for Linux.
            subprocess.run( [ "cmake", "--install", tool_build_folder + "/Release", "--config", "Release" ] )
    else:
        build_type = ""
        while not quick_setup:
            cls()
            print( "************************************************************" )
            print( "* Install menu:" )
            print( "* " )
            print( "* Select task." )
            print( "* " )
            print( "* [ 1 ] Install release" )
            print( "* [ 2 ] Install debug" )
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
    if not is_windows:
        # No installing on Linux for now. TODO for later...
        return

    package_type = ""
    while True:
        cls()
        print( "************************************************************" )
        print( "* Main menu:" )
        print( "* " )
        print( "* Select task" )
        print( "* " )
        print( "* [ 1 ] Create 7zip package" )
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
            package_type = "7Z"
            break

    packaging_source_path = "Packaging"
    packaging_build_path = tool_build_folder + "/" + packaging_source_path

    build_parameters = [ "cmake" ]
    build_parameters += [ "-Wno-dev" ]
    if is_windows:
        # On windows, make sure we're copying package into a folder.
        # Might expand this to Linux as well in the future when creating packages.
        fixed_path = os.getcwd() + "/" + tool_build_install_path
        print( "FIXED PATH:", fixed_path )
        build_parameters += [ "-D", "CMAKE_INSTALL_PREFIX=./" + tool_build_install_path ]
    #call_parameters += [ "-D", 'CMAKE_BUILD_TYPE=Release' ]
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
    cpack_parameters += [ "-B", "../install_packages" ]
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
        if is_windows:
            # No installing on Linux for now. TODO for later...
            print( "* [ 3 ] Install" )
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
            return
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
