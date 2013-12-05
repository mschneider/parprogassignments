import sys
from validatelib import *

if __name__ == '__main__':
    res = ExecutionInfo.runall([
        ExecutionInfo('assigment example full', './heatmap', [20, 7, 17, 'task2.1_hotspots.csv'], TextFileInfo('output.txt', '^.*?$', 'task2.1_ideal_full.txt')),
        ExecutionInfo('assigment example coords', './heatmap', [20, 7, 17, 'task2.1_hotspots.csv', 'task2.1_coords.csv'], NumberListFileInfo('output.txt', '^.*?$', 'task2.1_ideal_coords.txt')),
        ExecutionInfo('odd column count', './heatmap', [231, 257, 123, 'task2.1_hotspots_medium.csv', 'task2.1_coords_medium.csv'], NumberListFileInfo('output.txt', '^.*?$', 'task2.1_ideal_coords_medium_odd.txt'))
    ])
    
    print()
    
    res += ExecutionInfo('medium load', './heatmap', [200, 200, 200, 'task2.1_hotspots_medium.csv', 'task2.1_coords_medium.csv'], NumberListFileInfo('output.txt', '^.*?$', 'task2.1_ideal_coords_medium.txt')).measure()
    sys.exit(res)