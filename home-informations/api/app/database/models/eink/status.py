from enum import IntEnum


class Status(IntEnum):
    def __new__(cls, value, label):
        obj = int.__new__(cls, value)
        obj._value_ = value
        obj.label = label
        return obj

    @classmethod
    def validate_value(cls, value):
        valid_values = [status.value for status in Status]
        if value in valid_values:
            return value
        else:
            raise ValueError(f"Value {value} not in {valid_values}")

    @classmethod
    def get_label(cls, value):
        return {status.value : status.label for status in Status}[value]
   
    @classmethod
    def validate_label(cls, label):
        valid_labels = [status.label for status in Status]
        if label in valid_labels:
            return label
        else:
            raise ValueError(f"Label {label} not in {valid_labels}")

    @classmethod
    def get_value(cls, label):
        return {status.label : status.value for status in Status}[label]

    PASS = 0, "pass"
    WARN = 1, "warn"
    ERROR = 2, "error"
