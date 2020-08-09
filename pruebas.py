import pytest
import ujsoncached
import copy
import sys

SAMPLE_OBJ = {
    'a': 5,
    "b": "asdopkasd",
    "c": {
        "s": "bleehehhhhh"
    },
    "d": {},
    "e": {"asdasd": "hi"},
    "alist": ["5", "2", "3"],
    "blist": ["1", "2", {"k": ["value"]}]
}

refcnt = lambda: sys.gettotalrefcount() if hasattr(sys, 'gettotalrefcount') else '?'


def reserialize(x, use_cached=True):
    json_str = ujsoncached.dumps(x)
    new_obj = ujsoncached.loads(json_str, use_cached=use_cached)
    return new_obj


def assert_caches_properly_invalidated(obj, use_cached=True):
    """ Returns false if any item's cached json differs from its actual content """
    if hasattr(obj, '__ijson__'):
        cached_json = obj.__ijson__()
        if cached_json is not None:
            assert obj == ujsoncached.loads(cached_json, use_cached=use_cached)

    if isinstance(obj, dict):
        for k, v in obj.items():
            assert_caches_properly_invalidated(v, use_cached)
    if isinstance(obj, list):
        for v in obj:
            assert_caches_properly_invalidated(v, use_cached)

    assert reserialize(obj, use_cached) == obj


def test_simple():
    assert_caches_properly_invalidated(reserialize(SAMPLE_OBJ))


def edit_dict_assignment(obj):
    obj['e']['x'] = 'NEW VALUE'
    assert obj['e']['x'] == 'NEW VALUE'
    return obj


def edit_dict_pop(obj):
    obj['e'].pop('asdasd')
    assert obj['e'] == {}
    obj['e'].pop('does-not-exist', None)
    with pytest.raises(KeyError):
        obj['e'].pop('does-not-exist2')


def edit_dict_popitem(obj):
    assert obj['c'].popitem() == ('s', 'bleehehhhhh')
    with pytest.raises(KeyError):
        assert obj['c'].popitem()
    assert obj['c'] == {}


def edit_dict_del(obj):
    del obj['e']['asdasd']
    assert obj['e'] == {}


def edit_dict_setdefault(obj):
    obj['e'].setdefault('x', 4)
    obj['e'].setdefault('y')
    assert obj['e'] == {"asdasd": "hi", 'x': 4, 'y': None}


def edit_dict_update1(obj):
    obj['e'].update([('new', 'value')])
    assert obj['e'] == {"asdasd": "hi", 'new': 'value'}


def edit_dict_update2(obj):
    obj['e'].update(new='value')
    assert obj['e'] == {"asdasd": "hi", 'new': 'value'}


def edit_dict_update3(obj):
    obj['e'].update({'new': 'value'})
    assert obj['e'] == {"asdasd": "hi", 'new': 'value'}


def edit_dict_clear(obj):
    obj['e'].clear()
    assert obj['e'] == {}


def edit_list_clear(obj):
    obj['alist'].clear()
    assert obj['alist'] == []


def edit_list_fullreplace(obj):
    obj['alist'][:] = [2]
    assert obj['alist'] == [2]


def edit_list_setitem(obj):
    obj['alist'][1] = ['val']
    assert obj['alist'] == ['5', 'val', '3']


def edit_list_append(obj):
    obj['alist'].append('x')
    assert obj['alist'][-1] == 'x'


def edit_list_insert(obj):
    obj['alist'].insert(2, 'x')
    assert obj['alist'] == ['5', '2', 'x', '3']


def edit_list_extend(obj):
    obj['alist'].extend(['a', 'b'])
    assert obj['alist'] == ['5', '2', '3', 'a', 'b']


def edit_list_pop1(obj):
    obj['alist'].pop()
    assert obj['alist'] == ['5', '2']


def edit_list_pop2(obj):
    obj['alist'].pop(0)
    assert obj['alist'] == ['2', '3']


def edit_list_remove(obj):
    with pytest.raises(KeyError):
        obj['alist'].remove('abc')
    obj['alist'].remove('2')
    assert obj['alist'] == ['5', '3']


def edit_list_reverse(obj):
    obj['alist'].reverse()
    assert obj['alist'] == ['3', '2', '5']


def edit_list_sort(obj):
    obj['alist'].sort()
    assert obj['alist'] == ['2', '3', '5']


@pytest.mark.parametrize('func_edit', [
    edit_dict_assignment,
    edit_dict_pop,
    edit_dict_popitem,
    edit_dict_setdefault,
    edit_dict_del,
    edit_dict_update1,
    edit_dict_update2,
    edit_dict_update3,
    edit_dict_clear,
    edit_list_clear,
    edit_list_fullreplace,
    edit_list_setitem,
    edit_list_append,
    edit_list_insert,
    edit_list_extend,
    edit_list_pop1,
    edit_list_pop2,
    edit_list_remove,
    edit_list_reverse,
    edit_list_sort
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

    for i in range(20000):
        one()
        x = reserialize(x)
        assert_caches_properly_invalidated(x)
        if i % 100 == 0:
            print(refcnt())


def simple():
    x = ujsoncached.dumps([1, 2])
    print('serialized')
    ujsoncached.loads(x)
    print('deserialized')


if __name__ == "__main__":
    # leaktest()
    simple()
    print(refcnt())
    print("exit")