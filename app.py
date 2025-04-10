################################################################################
#Date: November 28th, 2024                                                     #
#File: app.py                                                                  #
#Authour: Ben Haubrich                                                         #
#Synopsis: Convenience script for building, running, analyzing, and debugging an
#          an application on various platforms                                 #
################################################################################
import argparse
import subprocess
from shutil import which, rmtree
from os import rename, remove, chdir, environ
from pathlib import Path
from platform import system
from getpass import getuser

"""
Sometimes the name that you use to search on your system with `which` is not the same as the name that is used to install the program.
This function converts from the name you used to search with `which` to the name used to install.
"""
def installationName(programName):
   if system() == 'Darwin' or system() == 'Linux':
      if 'ninja' == programName:
         return 'ninja-build'
         
   return programName

def installProgram(systemName, programName):
   if systemName == 'Darwin':
      if None == which(programName):
         print('Installing: ' + programName)
         subprocess.run(["brew", "install", installationName(programName)])
   elif systemName == 'Linux':
      if None == which(programName):
         print('Installing: ' + programName)
         subprocess.run(["sudo", "apt", "install", installationName(programName)])

def setupForPlatform(systemName):
  rootPermissionRequired = False
  executableSuffix = '.elf'
  debugger = 'gdb'

  if systemName == 'Darwin':
      cxxCompiler = which('clang++')
      cCompiler =  which('clang')
      debugger = 'lldb'
      executableSuffix = '.Mach-O'

      requiredSoftware = ['cmake', 'ninja', 'git', 'openocd', 'wget']
      for software in requiredSoftware:
         installProgram(systemName, software)

      return systemName, cCompiler, cxxCompiler, executableSuffix, debugger, rootPermissionRequired

  elif systemName == 'Linux':
      cxxCompiler = which('g++')
      cCompiler =  which('gcc')
      #In order to run the operating system on linux with the real-time scheduler settings you must run as root.
      rootPermissionRequired = True

      requiredSoftware = ['cmake', 'ninja', 'git', 'openocd', 'wget']
      for software in requiredSoftware:
         installProgram(systemName, software)

      return systemName, cCompiler, cxxCompiler, executableSuffix, debugger, rootPermissionRequired

if __name__ == '__main__':

  parser = argparse.ArgumentParser(prog='slidePot.py',
                                       description='Run cmake projects for various platforms',
                                       epilog='Created by Ben Haubrich April 19th, 2024')
  #This first positional argument holds one or more arguments (nargs='+') so that when new positional commands are add below
  #They are contained within the list of arguments for the first positional argument. In this way, a list of possible
  #commands can be searched through by the name of the commands given.
  parser.add_argument('command', type=ascii, nargs='+', default='None',
                          help=argparse.SUPPRESS
                     )

  parser.add_argument('clean', type=ascii, nargs='?', default='None',
                          help='Clean up the files from the last build'
                     )
  parser.add_argument('build', type=ascii, nargs='?', default='None',
                          help='Build the project in the selected directory'
                     )
  parser.add_argument('run', type=ascii, nargs='?', default='None',
                          help='Run the executable on the current platform. Does not run for the target.'
                     )
  parser.add_argument('doxygen', type=ascii, nargs='?', default='None',
                          help='Build the Doxygen documentation'
                     )
  parser.add_argument('valgrind', type=ascii, nargs='?', default='None',
                          help='Run valgrind with the selected analyzer. default is memcheck.'
                     )

  parser.add_argument('-c', '--project-dir', default='.',
                    help='The directory to build the project which contains a top-level CMakeLists.txt. Defaults to current directory'
                    )
  parser.add_argument('-b', '--build-type', nargs='+', type=ascii, default='Debug',
                    help='Build version to build. Defaults to "Debug". Choose from: "Debug", "Release"',
                    )
  parser.add_argument('-x', '--toolchain', nargs='?', type=ascii, default=None,
                    help='Use the specified toolchain file instead the system default.',
                    )
  parser.add_argument('-t', '--target', nargs='?', type=ascii, default=None,
                    help='Compile for the target given by target'
                    )
  parser.add_argument('-v', '--valgrind-check', nargs='+', type=ascii, default='memcheck',
                     help='Run valgrind. default is memcheck. Choose from: "memcheck", "cachegrind", "callgrind", "helgrind", "drd", "massif", "dhat"',
                     )

  args = parser.parse_args()

  #Uncomment for help with debugging.
  #print("{}".format(args))
  executableName = 'SlidePotentiometer'
  systemName, cCompiler, cxxCompiler, executableSuffix, debugger, rootPermissionRequired = setupForPlatform(system())
  if (args.target != None):
      buildDirectoryName = args.target.strip('\'') + '_build'
  else:
      buildDirectoryName = systemName + '_build'

  cmakeBuildDirectory = Path(args.project_dir + '/' + buildDirectoryName)

  if '\'clean\'' in args.command:
    if cmakeBuildDirectory.exists():
      rmtree(args.project_dir + '/' + buildDirectoryName)

  if '\'build\'' in args.command and '\'test\'' not in args.command:

    cmakeBuildDirectory.mkdir(parents=True, exist_ok=True)
    chdir(buildDirectoryName)

    cmakeCommand = ['cmake',
                    '-G Ninja',
                    '-DCMAKE_C_COMPILER=' + cCompiler,
                    '-DCMAKE_CXX_COMPILER=' + cxxCompiler,
                    '-S' + '../' + args.project_dir.strip('\'')]
    
    if (args.toolchain != None):
       cmakeCommand.append('-DCMAKE_TOOLCHAIN_FILE=' + args.toolchain.strip('\''))

    if (args.target != None):
       cmakeCommand.append('-D' + args.target.strip('\'') + '=1')
       if (args.target.strip('\'') == 'Tm4c123'):
           installProgram(systemName, 'lm4flash')

    if (args.build_type[0].strip('\'').lower() == 'debug'):
      subprocess.run(cmakeCommand)
    elif (args.build_type[0].strip('\'').lower() == 'release'):
      cmakeCommand.append('-DRELEASE_BUILD=1')
      subprocess.run(cmakeCommand)
    else:
      subprocess.run(cmakeCommand)

    subprocess.run(['ninja'])

    chdir('..')

  if '\'run\'' in args.command:
      if True == rootPermissionRequired and getuser() != 'root' and systemName == 'Linux':
          print("The operating system uses realtime scheduling which on this platform requires root permission.")
          print("https://stackoverflow.com/questions/46874369/thread-explicit-scheduling-posix-api-gives-error")
          exit()
      else:
          subprocess.run([buildDirectoryName + '/' + executableName + executableSuffix], shell=True)

  if '\'doxygen\'' in args.command:
      if which('doxygen') == None:
         print("Doxygen is not installed. Install it (Y/n)?")
         response = input()
         if 'Y' == response:
            installProgram(systemName, 'doxygen')
         else:
            exit()

      subprocess.run(['doxygen', 'Doxygen/Doxyfile'])
   
  if '\'valgrind\'' in args.command:
      if (rootPermissionRequired and getuser() != 'root'):
         print("Re-run with sudo to do valgrind tests")
         exit()

      if which('valgrind') == None:
         print("Valgrind is not installed. Install it (Y/n)?")
         response = input()
         if 'Y' == response:
            installProgram(systemName, 'valgrind')
         else:
            exit()

      subprocess.run(['valgrind', '--tool=' + args.valgrind_check.strip('\''), '--track-origins=yes', '--leak-check=full', '--read-inline-info=yes', '-s', buildDirectoryName + '/' + executableName + executableSuffix], shell=True)

exit(0)
