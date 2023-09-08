import time


def Process(db, input):
    t = 1
    try:
        t = int(input)
    except:
        return (False, 'input must be a time duration')
    print('sleeping for {} seconds'.format(t))
    time.sleep(t)
    return (True, '')
