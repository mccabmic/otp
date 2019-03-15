def encrypt(message, key):
    my_string = []
    for i in range(len(message)):
        char = (ord(message[i]) + ord(key[i]) - 65) % 27 + 65
        if char == 91:
            char = 32
        my_string.append(chr(char))

    return ''.join(my_string)

def decrypt(message, key):
    my_string = []
    for i in range(len(message)):
        if ord(message[i]) == 32:
            char = (91 - 65 - ord(key[i])) % 27 + 65
        else:
            char = (ord(message[i]) - 65 - ord(key[i])) % 27 + 65
        my_string.append(chr(char))

    return ''.join(my_string)

message = "FUCKYOU"
key = "ABCDEFG"
cipher = encrypt(message, key)
print(cipher)
print(decrypt(cipher, key))