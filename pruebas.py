import pytest
import ujson
import copy

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

def edit_del(obj):
    del obj['e']['asdasd']
    assert obj['e'] == {}

def edit_setdefault(obj):
    obj['e'].setdefault('x', 4)
    obj['e'].setdefault('y')
    assert obj['e'] == {"asdasd": "hi", 'x': 4, 'y': None}

@pytest.mark.parametrize('func_edit', [
    edit_assignment,
    # edit_pop,
    edit_setdefault,
    edit_del
])
def test_edit(func_edit):
    obj = reserialize(SAMPLE_OBJ)
    func_edit(obj)
    assert_caches_properly_invalidated(obj)
