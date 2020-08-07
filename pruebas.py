import pytest
import ujson
import copy
import sys

SAMPLE_OBJ = {
    'a': 5,
    "b": "asdopkasd",
    "c": {
        "s": "bleehehhhhh"
    },
    "d": {},
    "e": {"asdasd": "hi"}
}


def reserialize(x):
    json_str = ujson.dumps(x)
    new_obj = ujson.loads(json_str)
    return new_obj


def assert_caches_properly_invalidated(obj):
    """ Returns false if any item's cached json differs from its actual content """
    if hasattr(obj, '__ijson__'):
        cached_json = obj.__ijson__()
        if cached_json is not None:
            assert obj == ujson.loads(cached_json)

    if isinstance(obj, dict):
        for k, v in obj.items():
            assert_caches_properly_invalidated(v)
    if isinstance(obj, list):
        for v in obj:
            assert_caches_properly_invalidated(v)

    assert reserialize(obj) == obj


def test_simple():
    assert_caches_properly_invalidated(reserialize(SAMPLE_OBJ))


def edit_assignment(obj):
    obj['e']['x'] = 'NEW VALUE'
    assert obj['e']['x'] == 'NEW VALUE'
    return obj


def edit_pop(obj):
    obj['e'].pop('asdasd')
    assert obj['e'] == {}
    obj['e'].pop('does-not-exist', None)
    with pytest.raises(KeyError):
        obj['e'].pop('does-not-exist2')


def edit_popitem(obj):
    assert obj['c'].popitem() == ('s', 'bleehehhhhh')
    with pytest.raises(KeyError):
        assert obj['c'].popitem()
    assert obj['c'] == {}


def edit_del(obj):
    del obj['e']['asdasd']
    assert obj['e'] == {}


def edit_setdefault(obj):
    obj['e'].setdefault('x', 4)
    obj['e'].setdefault('y')
    assert obj['e'] == {"asdasd": "hi", 'x': 4, 'y': None}


def edit_update1(obj):
    obj['e'].update([('new', 'value')])
    assert obj['e'] == {"asdasd": "hi", 'new': 'value'}


def edit_update2(obj):
    obj['e'].update(new='value')
    assert obj['e'] == {"asdasd": "hi", 'new': 'value'}


def edit_update3(obj):
    obj['e'].update({'new': 'value'})
    assert obj['e'] == {"asdasd": "hi", 'new': 'value'}


def edit_clear(obj):
    obj['e'].clear()
    assert obj['e'] == {}


@pytest.mark.parametrize('func_edit', [
    edit_assignment,
    edit_pop,
    edit_popitem,
    edit_setdefault,
    edit_del,
    edit_update1,
    edit_update2,
    edit_update3,
    edit_clear
])
def test_edit(func_edit):
    obj = reserialize(SAMPLE_OBJ)
    func_edit(obj)
    assert_caches_properly_invalidated(obj)


def one():
    x = copy.deepcopy(SAMPLE_OBJ)
    raw = ujson.dumps(x)
    for _ in range(5):
        a = ujson.loads(raw)


def leaktest():
    # sample = {'a': 4, 'b': 5, 'c': {}, 'd': {}}
    sample = SAMPLE_OBJ
    x = sample

    refs = []
    for i in range(200):
        one()
        x = reserialize(x)
        refs.append(x['e'])
        assert_caches_properly_invalidated(x)
        if i % 100 == 0:
            print(sys.gettotalrefcount())
    print(refs)


if __name__ == "__main__":
    # print(ujson.__file__)
    leaktest()
    # x = ujson.loads(ujson.dumps({'a': 4}))
    # del x
    print(sys.gettotalrefcount())
    print("exit")