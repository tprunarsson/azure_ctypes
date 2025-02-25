import ctypes
import json

# Define the JsonResult struct in Python
#class JsonResult(ctypes.Structure):
#    _fields_ = [("json_result", ctypes.c_char_p)]

# Load the shared library
ctypes.CDLL("./libhighs.so.1", mode=ctypes.RTLD_GLOBAL) 
ctypes.CDLL("./libscip.so.10.0", mode=ctypes.RTLD_GLOBAL) 
lib = ctypes.CDLL("./libjson_processor.so")

# Configure the process_json function
#lib.process_json.restype = ctypes.POINTER(JsonResult)  # Returns a pointer to JsonResult
lib.process_json.restype = ctypes.c_void_p
lib.process_json.argtypes = [ctypes.c_char_p]

# Configure the free_json_result function
#lib.free_json_result.argtypes = [ctypes.POINTER(JsonResult)]
lib.free_json_result.argtypes = [ctypes.c_void_p]
lib.free_json_result.restype = None

# Input JSON (your provided example)
input_json = json.dumps([
    {
        "cr536_accountname": "Hamarsey ehf.",
        "cr536_color": 1,
        "cr536_column": 1,
        "cr536_glulamlength": 3901,
        "cr536_height": 225,
        "cr536_numberofglulams": 1,
        "cr536_press": 1,
        "cr536_row": 1,
        "cr536_salesline_id": "SP785781_1",
        "cr536_saleslinenumber": "1",
        "cr536_salesorder_id": "SP785781",
        "cr536_type": "SP788344",
        "cr536_width": 90
    },
    {
        "cr536_accountname": "Hamarsey ehf.",
        "cr536_color": 1,
        "cr536_column": 2,
        "cr536_glulamlength": 3901,
        "cr536_height": 225,
        "cr536_numberofglulams": 1,
        "cr536_press": 1,
        "cr536_row": 1,
        "cr536_salesline_id": "SP785781_1",
        "cr536_saleslinenumber": "1",
        "cr536_salesorder_id": "SP785781",
        "cr536_type": "SP788344",
        "cr536_width": 90
    }
])

# Call the C function
result_ptr = lib.process_json(input_json.encode('utf-8'))
 
# Access the resulting JSON
if result_ptr:
    #result_json = result_ptr.contents.json_result.decode('utf-8')  # Extract the JSON string
    result_json = ctypes.string_at(result_ptr).decode("utf-8")
    print("Modified JSON from C:")
    print(result_json)

    # Free the memory in C
    lib.free_json_result(result_ptr)

