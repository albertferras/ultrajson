import ujsonselect
from pprint import pprint

select_properties = {
    "hotel": {
        "stars": True
    }
}

obj = {
    "id": "1234",
    "hotel": {
        "stars": 5,
        "street_address": "blabla",
        "reviews": [1, "", {}, [], [{}], "}[]}bleh\"", "\\\""]
    }
}

raw = ujsonselect.dumps(obj)
print('raw=', raw)

result = ujsonselect.loads(raw, select_properties=select_properties)
pprint(result)

result = ujsonselect.loads(raw)