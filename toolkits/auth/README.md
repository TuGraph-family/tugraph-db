# SSL authentication

## Requirements

openssl

## Generate keys

Just run gen.sh. When openssl req asks for "Common Name (e.g. server FQDN or YOUR name)", put the FQDN of the host on
which the Thrift program will run.

## Enable ssl authentication on server

Copy `server-key.pem` `server-cert.pem`, `client-cert.pem` to the server. Modify the `lgraph.json`, add options below.

```json
    "ssl_auth" : true,
"server_key": "server-key.pem",
"server_cert": "server-cert.pem",
"client_cert": "client-cert.pem",
```

## Enable ssl authentication on client

Copy `server-cert.pem` `client-key.pem`, `client-cert.pem` to the client. Create Session like codes below.

```python3
from LightningGraphClient import *

session = Session("localhost", 9091, "server-cert.pem", "client-key.pem", "client-cert.pem")
```
