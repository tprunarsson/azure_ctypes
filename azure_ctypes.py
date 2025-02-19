import azure.functions as func
import logging
import json
import ctypes

# Define the JsonResult struct in Python
#class JsonResult(ctypes.Structure):
#    _fields_ = [("json_result", ctypes.c_char_p)]

# Load the shared library
ctypes.CDLL("libstdc++.so.6", mode=ctypes.RTLD_GLOBAL) 
lib = ctypes.CDLL("./libjson_processor.so")

# Configure the process_json function
# lib.process_json.restype = ctypes.POINTER(JsonResult)  # Returns a pointer to JsonResult
lib.process_json.restype = ctypes.c_void_p
lib.process_json.argtypes = [ctypes.c_char_p]

# Configure the free_json_result function
#lib.free_json_result.argtypes = [ctypes.POINTER(JsonResult)]
lib.free_json_result.argtypes = [ctypes.c_void_p]
lib.free_json_result.restype = None

# Azure Function App
app = func.FunctionApp(http_auth_level=func.AuthLevel.FUNCTION)

@app.route(route="HttpExample")
def HttpExample(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')

    try:
        # Parse the input JSON from the HTTP request
        req_body = req.get_json()
        input_json = json.dumps(req_body)  # Serialize the JSON input

        # Pass the JSON to the C function
        result_ptr = lib.process_json(input_json.encode('utf-8'))

        # Extract the resulting JSON string from the C function
        if result_ptr:
            #result_json = result_ptr.contents.json_result.decode('utf-8')
            result_json = ctypes.string_at(result_ptr).decode("utf-8")

            # Free the memory in C
            lib.free_json_result(result_ptr)

            # Return the modified JSON as the HTTP response
            return func.HttpResponse(
                result_json,
                status_code=200,
                mimetype="application/json"
            )
        else:
            return func.HttpResponse(
                "Error: Failed to process JSON with the C function.",
                status_code=500
            )

    except Exception as e:
        logging.error(f"Error processing the request: {str(e)}")
        return func.HttpResponse(
            f"Error: {str(e)}",
            status_code=400
        )

