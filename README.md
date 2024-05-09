# ESP32_NVS
ESP32 IDF Non-Volatile Storage Library

The ESP32 IDF Non-Volatile Storage (NVS) library is designed to store key-value pairs in flash.

2. ESP32 NVS
NVS operates on key-value pairs. Keys are ASCII strings, the maximum key length is currently 15 characters. Values can have one of the following types:

Integer types: uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t
Floating-point types: float, double
Zero-terminated string
Variable length binary data (blob)
NVS assigns each key-value pair to one of namespaces. Namespace names follow the same rules as key names, i.e., the maximum length is 15 characters. Furthermore, there can be no more than 254 different namespaces in one NVS partition.
