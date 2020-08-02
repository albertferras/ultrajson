import ujson

x = {
    'a': 5,
    "b": "asdopkasd",
    "c": {
        "s": "bleehehhhhh"
    },
    "d": {}
}
# x = {}

raw = ujson.dumps(x)
print("JSON:", repr(raw))
print('length =', len(raw))

print('')
print("DESERIALIZE")
xdes = ujson.loads(raw)
print('OBJ=', xdes)
print(xdes.__ijson__())
# print(xdes['c'].__ijson__())


print('')
print("RESERIALIZE")
print(ujson.dumps(xdes))
