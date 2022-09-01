import argparse
import requests

urls = [
    'http://brickschema.org/schema/1.0.2/Brick.ttl',
    'http://brickschema.org/schema/1.0.2/BrickFrame.ttl',
    'http://brickschema.org/schema/1.0.2/BrickTag.ttl',
    'http://brickschema.org/schema/1.0.2/BrickUse.ttl',
]

ap = argparse.ArgumentParser(description='Download Brick Schema')
ap.parse_args()

for url in urls:
    file_name = url.split('/')[-1]
    r = requests.get(url)
    open(file_name, 'wb').write(r.content)
    print
    '%s downloaded.' % (file_name)
