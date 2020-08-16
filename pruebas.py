import ujsonselect
from pprint import pprint
from mybenchmarks.benchmark_select_deserialize import files, get_file, SelectProperties

# select_properties = {
#     "hotel": {
#         "stars": True
#     }
# }
#
# obj = {
#     "id": "1234",
#     "hotel": {
#         "stars": 5,
#         "street_address": "blabla",
#         "reviews": [1, "", {}, [], [{}], "}[]}bleh\"", "\\\""]
#     }
# }


obj = ujsonselect.loads(get_file(files['descriptions']))
select_properties = SelectProperties(['does-not-exist']).tree
print("TREE", select_properties)

input("next?")
raw = ujsonselect.dumps(obj)
print('raw=', raw)

result = ujsonselect.loads(raw, select_properties=select_properties)
print("Final dict:")
pprint(result)

# result = ujsonselect.loads(raw)
# print("result", result)