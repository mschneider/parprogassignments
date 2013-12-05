import sys
from validatelib import *

if __name__ == '__main__':
    result = ExecutionInfo('assigment example', './decrypt', ['task3.2_pwsmall.txt', 'task3.2_dict.txt'], TextFileInfo('output.txt', '^((user906;Bahnhof.*?user\d*;.*?)|(user\d*;.*?user906;Bahnhof))$')).run()

    try:
        solution = TextFileInfo('taskCryptSolution.txt', '^((user906;Bahnhof.*?user\d*;.*?)|(user\d*;.*?user906;Bahnhof))$')
        solution.ensurefileexists()
        solution.checkfile()
    except ErrorMessage as e:
        e.show()
        result += e.errorcode
    
    sys.exit(result)