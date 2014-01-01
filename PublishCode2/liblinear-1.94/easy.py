#!/usr/bin/env python

import sys
import os
from subprocess import *

if len(sys.argv) <= 1:
	print('Usage: {0} training_file [testing_file]'.format(sys.argv[0]))
	raise SystemExit

# svm, grid, and gnuplot executable files

is_win32 = (sys.platform == 'win32')
if not is_win32:
	#svmscale_exe = "./svm-scale"
	svmtrain_exe = "./train"
	svmpredict_exe = "./predict"
	#grid_py = "./grid.py"
	#gnuplot_exe = "/usr/bin/gnuplot"
else:
        # example for windows
	#svmscale_exe = r"..\windows\svm-scale.exe"
	svmtrain_exe = r"..\windows\train.exe"
	svmpredict_exe = r"..\windows\predict.exe"
	#gnuplot_exe = r"c:\tmp\gnuplot\binary\pgnuplot.exe"
	#grid_py = r".\grid.py"

#assert os.path.exists(svmscale_exe),"svm-scale executable not found"
assert os.path.exists(svmtrain_exe),"svm-train executable not found"
assert os.path.exists(svmpredict_exe),"svm-predict executable not found"
#assert os.path.exists(gnuplot_exe),"gnuplot executable not found"
#assert os.path.exists(grid_py),"grid.py not found"

train_pathname = sys.argv[1]
assert os.path.exists(train_pathname),"training file not found"
file_name = os.path.split(train_pathname)[1]
model_file = file_name + ".model"


if len(sys.argv) > 2:
	test_pathname = sys.argv[2]
	file_name = os.path.split(test_pathname)[1]
	assert os.path.exists(test_pathname),"testing file not found"
	predict_test_file = file_name + ".predict"




cmd = '{0} "{1}"'.format(svmtrain_exe,train_pathname)
Popen(cmd, shell = True, stdout = PIPE).communicate()
print('Training...')


Popen(cmd, shell = True, stdout = PIPE).communicate()	

cmd = '{0} "{1}" "{2}" "{3}"'.format(svmpredict_exe, test_pathname, model_file, predict_test_file)

Popen(cmd, shell = True, stdout = PIPE).communicate()
print('Testing...')

