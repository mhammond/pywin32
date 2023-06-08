"""Contains RelaxedIntEnum

This module is used by modules built by @makepy@ using enums.

RelaxedIntEnum is an enum that does not throw on unknown values 
but dynamically adds them to the member-map.
"""

from enum import IntEnum


class RelaxedIntEnum(IntEnum):
    """Support for C/C++ style-enums"""

    @classmethod
    def _missing_(cls, value):
        if not isinstance(value, int):
            raise ValueError("%r is not a valid %s" % (value, cls.__name__))
        new_member = cls._create_pseudo_member_(value)
        return new_member

    @classmethod
    def _create_pseudo_member_(cls, value):
        pseudo_member = cls._value2member_map_.get(value, None)
        if pseudo_member is None:
            # construct singleton pseudo-member
            pseudo_member = int.__new__(cls, value)
            pseudo_member._name_ = None
            pseudo_member._value_ = value
            # use setdefault in case another thread already created a composite
            # with this value
            pseudo_member = cls._value2member_map_.setdefault(value, pseudo_member)
        return pseudo_member
