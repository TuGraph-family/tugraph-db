import argparse
import requests

urls = [
    'http://brickschema.org/ttl/ghc_brick.ttl',
    'http://brickschema.org/ttl/gtc_brick.ttl',
    'http://brickschema.org/ttl/soda_brick.ttl',
    'http://brickschema.org/ttl/ebu3b_brick.ttl',
    'http://brickschema.org/ttl/rice_brick.ttl',
]

ap = argparse.ArgumentParser(description='Download and Clean Brick Building Examples')
ap.parse_args()

for url in urls:
    file_name = url.split('/')[-1]
    r = requests.get(url)
    open(file_name, 'wb').write(r.content)
    print
    '%s downloaded.' % (file_name)

for url in urls:
    file_name = url.split('/')[-1]
    f = open(file_name, 'r+')
    s = f.read()
    f.seek(0, 0)
    f.write(s.replace('http://brickschema.org', 'https://brickschema.org'))
    f.close()
    print
    '%s cleaned.' % (file_name)
