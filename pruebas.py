import ujson


def foo(dbg=print):
    x = {
        'a': 5,
        "b": "asdopkasd",
        "c": {
            "s": "bleehehhhhh"
        },
        "d": {},
        "e": {"asdasd": "hi"}
    }
    # x = {}

    raw = ujson.dumps(x)
    dbg("JSON:", repr(raw))
    dbg('length =', len(raw))

    dbg('')
    dbg("DESERIALIZE")
    xdes = ujson.loads(raw)
    dbg('OBJ=', xdes)
    dbg(xdes.__ijson__())
    # print(xdes['c'].__ijson__())

    xdes['c'].dbg()
    xdes['c'].__setitem__('x', "HEY")

    dbg('')
    dbg("RESERIALIZE")
    dbg(ujson.dumps(xdes, indent=4))

def noop(*a, **k):
    pass

def run():
    n = 1
    dbg_func = print if n==1 else noop
    for _ in range(n):
        foo(dbg=dbg_func)
        dbg_func('--')

if __name__ == "__main__":
    run()