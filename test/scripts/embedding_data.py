def write_to_files():
    start, end = 1, 1000000
    numbers_per_file = 100000
    for i in range(10):
        filename = f'chunk_{i}.txt'
        with open(filename, 'w') as file:
            file_start = start + i * numbers_per_file
            file_end = file_start + numbers_per_file
            for number in range(file_start, file_end):
                line = 'CREATE (:Chunk {id:%d, embedding:$embedding})\n' %(number)
                file.write(line)


write_to_files()
