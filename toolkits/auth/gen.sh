#!/bin/bash
openssl genrsa -out server-key.pem 8192
openssl req -new -x509 -key server-key.pem -out server-cert.pem
openssl genrsa -out client-key.pem 8192
openssl req -new -x509 -key client-key.pem -out client-cert.pem
