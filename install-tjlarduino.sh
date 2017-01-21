#!/bin/bash

##########################################
# install-tjlarduino.sh
#
# Intended to be a placeholder until 
# I learn how to add a sudo make 
# install/uninstall to CMake
#
##########################################

function cleanUp() {
    #suRemoveFile "$ui/arduino.h"
    echo "All cleaned up"
}

ui="/usr/include"
ul="/usr/lib"
olt="/opt/LibraryBuilds/arduinopc"
sourceDir="/opt/GitHub/arduinopc"
buildDir="/opt/LibraryBuilds/arduinopc"
ub="/usr/bin"

function displayHelp() {
    echo "Usage: install-arduinopc.sh [--install/--uninstall]"
}

cygwinCheck=$(uname -a | grep -i 'cygwin')

if [[ -z "$cygwinCheck" ]]; then
   if [[ "$EUID" != "0" ]]; then
      SUDO=sudo
   fi
else
    SUDO=""
fi

function showSuccess() {
    echo "success"
}

function showFailure() {
    echo "failure"
    cleanUp
}

function suRemoveFile() {
    echo -n "Removing \"$1\"..."
    rm -f "$1"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function suLinkFile() {
    echo -n "Linking \"$1\" to \"$2\"..."
    ln -s -f "$1" "$2"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function suRemoveFile() {
    echo -n "Removing \"$1\"..."
    $SUDO rm -f "$1"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function suLinkFile() {
    echo -n "Linking \"$1\" to \"$2\"..."
    $SUDO ln -s -f "$1" "$2"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}


function copyFile() {
    echo -n "Copying \"$1\" to \"$2\"..."
    cp -R "$1" "$2"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function appendStringToFile() {
    echo -n "Putting string \"$1\" into file $2..."
    echo "$1" >> "$2"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function changeDirectory() {
    echo -n "Entering directory \"$1\"..."
    cd "$1"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function createDirectory() {
    echo -n "Creating directory \"$1\"..."
    mkdir "$1"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function runGitClone() {
    echo -n "Cloning \"$1\" using git..."
    git clone "$1"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function runCmake() {
    echo -n "Running cmake from source directory \"$1\"..."
    cmake "$1"
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function runMake() {
    echo -n "Running make..."
    make
    if [[ "$?" -ne "0" ]]; then
        showFailure
        return 1
    else 
        showSuccess
        return 0
    fi
}

function bailout() {
    rm -rf "$buildDir"
}

function generateDesktopFile() {
    copyFile "$utilityDir/$skeletonDesktopFileName" "$buildDir/$desktopFileName" || { echo "Failed to generate desktop file, exiting"; exit 1; }
    copyFile "$iconPath" "$buildDir/" || { echo "Failed to generate desktop file, exiting"; exit 1; }
    appendStringToFile "Exec=$buildDir$programName" "$buildDir/$desktopFileName"
    appendStringToFile "Icon=$buildDir$iconName" "$buildDir/$desktopFileName"
}

function retrieveDependancy() {
    createDirectory "$buildDir/lib" || { echo "Failed to make dependancy lib directory, bailing out"; exit 1; }
    changeDirectory "$buildDir/lib"  || { echo "Failed to enter dependancy lib directory, bailing out"; exit 1; }
    runGitClone "$dependancySource" || { echo "Failed to retrieve dependancy source, bailing out"; exit 1; }
}

function buildDependancy() {
    sudo ./$depenancyInstallCommand || { echo "Failed to build dependancy, bailing out"; exit 1; }
}

if [[ "$1" == "-u" || "$1" == "--u" || "$1" == "-uninstall" || "$1" == "--uninstall" || "$1" == "uninstall" ]]; then
    
    suRemoveDirectory "$buildDir/" || { echo "Could not remove directory, bailing out"; exit 1;}
    suRemoveFile "$ui/serialport.h" || { echo "Could not remove file, bailing out"; exit 1;}
    suRemoveFile "$ui/arduino.h" || { echo "Could not remove file, bailing out"; exit 1;}
    suRemoveFile "$ui/tstream.h" || { echo "Could not remove file, bailing out"; exit 1;}

    if [[ -z "$cygwinCheck" ]]; then
        suRemoveFile "$ul/libtjlarduino.so" || { echo "Could not remove file, bailing out"; exit 1;}
    else
        suRemoveFile "$ul/cygtjlarduino.dll" || { echo "Could not remove file, bailing out"; exit 1;}
    fi

    suRemoveFile "$ul/libtjlarduino.a" || { echo "Could not remove file, bailing out"; exit 1;}

    echo "tjlarduino uninstalled successfully"

elif [[ "$1" == "-h" || "$1" == "--h" || "$1" == "-help" || "$1" == "--help" || "$1" == help ]]; then

    displayHelp
    exit 0

else
    trap bailout INT QUIT TERM
    if [[ $# -gt 0 ]]; then
        #if [[ "$1" == "--uninstall" ]]; then
        #    echo "Success"
        #    exit 0
        #fi
        buildDir="$1"
        if ! [[ -d "$buildDir" ]]; then
            createDirectory "$buildDir" || { echo "Unable to create build directory \"$buildDir\", exiting"; exit 1; }
        fi
    else
        #buildDir="$filePath/$buildDir"
        buildDir="$olt"
        if ! [[ -d "$buildDir" ]]; then
            createDirectory "$buildDir" || { echo "Unable to make build directory \"$buildDir\", exiting"; exit 1; }
        fi
    fi

    suLinkFile "$sourceDir/serialport/include/serialport.h" "$ui/" || { echo "Could not link file, bailing out"; exit 1;}
    suLinkFile "$sourceDir/arduino/include/arduino.h" "$ui/" || { echo "Could not link file, bailing out"; exit 1;}
    suLinkFile "$sourceDir/tstream/include/tstream.h" "$ui/" || { echo "Could not link file, bailing out"; exit 1;}

    changeDirectory "$buildDir" || { echo "Could not enter build directory, bailing out"; exit 1;}
    runCmake "$sourceDir" || { echo "Cmake failed, bailing out"; exit 1;}
    runMake || { echo "Make failed, bailing out"; exit 1;}

    if [[ -z "$cygwinCheck" ]]; then
        suLinkFile "$buildDir/libtjlarduino.so" "$ul/" || { echo "Could not link file, bailing out"; exit 1;}
    else
        suLinkFile "$buildDir/cygtjlarduino.dll" "$ul/" || { echo "Could not link file, bailing out"; exit 1;}
    fi

    suLinkFile "$buildDir/libtjlarduino.a" "$ul/" || { echo "Could not link file, bailing out"; exit 1;}

    echo "tjlarduino installed successfully"
fi
