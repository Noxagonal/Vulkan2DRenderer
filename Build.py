
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



# Add build systems here.        ( Description,                                     CMake generator name,                   Add -A "x64"
build_systems.append( BuildSystem( "CMake default",                                 "",                                     False ) )
build_systems.append( BuildSystem( "Microsoft Visual Studio 2019",                  "Visual Studio 16 2019",                True ) )
build_systems.append( BuildSystem( "Microsoft Visual Studio 2017",                  "Visual Studio 15 2017 Win64",          False ) )
build_systems.append( BuildSystem( "CodeBlocks - Ninja",                            "CodeBlocks - Ninja",                   False ) )
build_systems.append( BuildSystem( "CodeBlocks - Unix Makefiles",                   "CodeBlocks - Unix Makefiles",          False ) )
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

cls = lambda: os.system('cls' if os.name=='nt' else 'clear')

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
        print( "* [ x ] Back" )
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
        print( "* [ b ] Continue" )
        print( "* [ x ] Back" )
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
        print( "* [ 1 ] Build release" )
        print( "* [ 2 ] Build debug" )
        print( "* " )
        print( "* [ x ] Cancel" )
        print( "* " )
        print( "************************************************************" )
        print( "" )
        option = input( "Selection: " )
        option = option.lower()

        if option == "1":
            return [ True, "Release" ]
        elif option == "2":
            return [ True, "Debug" ]
        elif option == "x":
            return [ False, [] ]


def ConfigureAndBuildProjectMenu( quick_setup = False ):
    build_sys = SelectBuildSystem( quick_setup )
    if not build_sys[ 0 ]:
        return

    build_opt = SelectBuildOptions( quick_setup )
    if not build_opt[ 0 ]:
        return

    build_type = [ True, "Release" ]
    if not quick_setup:
        build_type = SelectBuildType()
    if not build_type[ 0 ]:
        return
        
    build_path = "build/" + build_type[ 1 ]

    call_parameters = [ "cmake" ]
    if build_sys:
        call_parameters += build_sys[ 1 ]
    call_parameters.append( "-Wno-dev" )
    call_parameters += build_opt[ 1 ]
    call_parameters.append( "-D" )
    call_parameters.append( "CMAKE_BUILD_TYPE=" + build_type[ 1 ] )
    call_parameters.append( "-S" )
    call_parameters.append( "." )
    call_parameters.append( "-B" )
    call_parameters.append( build_path )
    
    if not os.path.exists( "build" ):
        os.mkdir( "build" )
    if not os.path.exists( build_path ):
        os.mkdir( build_path )

    subprocess.run( call_parameters )
    if not quick_setup:
        print( "\n\nDone." )
        input( "Press enter..." )

def CompileMenu( quick_setup = False ):
    build_type = "Release"
    while not quick_setup:
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

    subprocess.run( [ "cmake", "--build", "build/" + build_type, "--config", build_type ] )
    if not quick_setup:
        print( "\n\nDone." )
        input( "Press enter..." )

    
def InstallMenu( quick_setup = False ):
    build_type = "Release"
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

    subprocess.run( [ "cmake", "--install", "build/" + build_type, "--config", build_type ] )
    if not quick_setup:
        print( "\n\nDone." )
        input( "Press enter..." )

def MainMenu():
    while True:
        cls()
        print( "************************************************************" )
        print( "* Main menu:" )
        print( "* " )
        print( "* Select task." )
        print( "* " )
        print( "* [ q ] Quick build and install using default settings." )
        print( "* [ 1 ] Configure and build project files." )
        print( "* [ 2 ] Compile." )
        print( "* [ 3 ] Install." )
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
            return
        elif option == "1":
            ConfigureAndBuildProjectMenu()
        elif option == "2":
            CompileMenu()
        elif option == "3":
            InstallMenu()


if __name__ == "__main__":
    MainMenu()

