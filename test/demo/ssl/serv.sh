
#openssl req -newkey rsa:2048 -nodes -keyout server.key -x509 -days 365 -out server.crt


openssl s_server -accept 1443 -cert server.crt -key server.key

