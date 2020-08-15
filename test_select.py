import ujson
import ujsonselect
import pytest
from pprint import pprint
from typing import List


EN_GB_LITERALS = ["AAA"]
ENTITY = {
    "id": "1234",
    "hotel": {"stars": 5, "is_nice": False, "street_name": "Something\nTwoLines", "total_rooms": None},
    "literals": {"canonical": {"en-GB": EN_GB_LITERALS, "broken": 123}},
    "legacy": {"individual": "ATL_1234"},
    "b2b": {"hotel": {"street_name": "SomethingLegal", "stars": None}},
}


class SelectProperties:
    """ Defines properties to retrieve when deserializing a document.
    If no property is defined (eg: `SelectProperties()`) then all properties will be deserialized.

    Examples:
        SelectProperties(['id', 'hotel.stars']) --> only id and hotel.stars are selected
        SelectProperties() --> all properties are selected
    """
    def __init__(self, select_properties: List[str] = None):
        self.tree = True
        if select_properties:
            select_tree = {}
            for prop in select_properties:
                prefix = ''
                node = select_tree
                parts = prop.split('.')
                for part in parts[:-1]:
                    if select_tree.get(part) is True:
                        break
                    node = node.setdefault(part, {})
                    prefix = f'{prefix}{part}.'

                if node is not True:
                    node[parts[-1]] = True
            self.tree = select_tree



@pytest.mark.parametrize(
    "select_properties,expected_entity",
    [
        ([], ENTITY),  # deserialize all properties
        (["id"], {"id": ENTITY["id"]}),
        (["hotel"], {"hotel": ENTITY["hotel"]}),
        (
            ["hotel.stars", "legacy", "nothing"],
            {"hotel": {"stars": ENTITY["hotel"]["stars"]}, "legacy": ENTITY["legacy"]},
        ),
        (["hotel.stars", "hotel", "hotel.what"], {"hotel": ENTITY["hotel"]}),
        (["hotel.stars.x"], {}),
        (["hotel.stars.x", "hotel.stars"], {"hotel": {"stars": ENTITY["hotel"]["stars"]}}),
        (
            ["literals.canonical.en-GB", "literals.canonical.nope"],
            {"literals": {"canonical": {"en-GB": EN_GB_LITERALS}}},
        ),
        (
            ["literals.canonical.en-GB", "literals.canonical.en-GB.1234"],
            {"literals": {"canonical": {"en-GB": EN_GB_LITERALS}}},
        ),
        (["literals.canonical.missing", "literals.canonical.en-GB.1234", "literals.canonical.broken.key"],
         {}),
        # nulls
        (["hotel", "b2b.hotel"], {"hotel": ENTITY["hotel"], "b2b": ENTITY["b2b"]}),
        (["hotel.stars", "b2b.hotel.stars"], {"hotel": {"stars": 5}, "b2b": {"hotel": {"stars": None}}}),
        (
            [
                "hotel.total_rooms",
                "hotel.keydoesnotexist",
                "hotel.total_rooms.bleh",
                "b2b.hotel.stars.bleh",
                "bleh.bleh.bleh",
            ],
            {"hotel": {"total_rooms": None}},
        ),
    ],
)
def test_serialization(select_properties, expected_entity):
    print("CASE")
    pprint(SelectProperties(select_properties).tree)
    raw = ujsonselect.dumps(ENTITY)
    print(raw)
    assert expected_entity == ujsonselect.loads(
        raw, select_properties=SelectProperties(select_properties).tree
    ), f"select_properties={select_properties}"


def test_deserialize_big():
    with open('./mybenchmarks/hotel_with_review_descriptions.json', 'r') as f:
        raw = f.read()
    obj = ujsonselect.loads(raw, select_properties=SelectProperties(['hotel.reviews', 'id']).tree)
    obj = ujsonselect.loads(raw)
    pprint(obj)