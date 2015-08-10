import subprocess
import sys



clangExe = sys.argv[1]
target = sys.argv[2]
fakeSource = sys.argv[3]

libs = sys.argv[4:]

astMergeArgs = list()
for lib in libs:
    astMergeArgs += ['-ast-merge', lib]

cmdLine = [clangExe, '-cc1', '-emit-pch', '-o', target]
cmdLine += astMergeArgs


process = subprocess.Popen(cmdLine, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
for line in iter(process.stdout.readline, ''):
    print(line, end='')
    sys.stdout.flush() # please see comments regarding the necessity of this line 
process.wait()
errcode = process.returncode
sys.exit(errcode)
