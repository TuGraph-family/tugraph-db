import numpy as np
filename = 'chunk.txt'
with open(filename, 'w', encoding='utf-8') as f:
    for id in range(1, 1000000):
        line = 'CREATE (:Chunk {id:%d, embedding:%s})\n' %(id)
        f.write(line)