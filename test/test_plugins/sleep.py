import time
def Process(db, input):
    t = 1
    try:
        t = float(input)
    except:
        return (False, 'input must be a time duration')
    print('sleeping for {} seconds'.format(t))
    time.sleep(t)
    return (True, 'slept for {} seconds'.format(t))