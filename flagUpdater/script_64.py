import base64
file = open("signature_decrypt64.flag.gpg", "r");
encoded = file.read()
data = base64.b64decode(encoded)
print(data)

