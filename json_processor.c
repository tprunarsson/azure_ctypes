#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>

// Struct to store the result and indicate ownership of memory

//typedef struct {
//    const char *json_result;
//} JsonResult;

//JsonResult* process_json(const char *json_input) {
    char * process_json(const char *json_input) {
    json_error_t error;
    json_t *root = json_loads(json_input, 0, &error);

    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return NULL;
    }

    if (!json_is_array(root)) {
        fprintf(stderr, "Error: Expected a JSON array.\n");
        json_decref(root);
        return NULL;
    }

    // Loop through each item in the array
    size_t index;
    json_t *item;
    json_array_foreach(root, index, item) {
        if (!json_is_object(item)) {
            fprintf(stderr, "Error: Expected JSON objects in array.\n");
            continue;
        }

        // Print and modify specific fields
        const char *keys[] = {"cr536_height", "cr536_numberofglulams", "cr536_press", "cr536_row"};
        for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
            json_t *field = json_object_get(item, keys[i]);
            if (field && json_is_integer(field)) {
                printf("%s: %lld\n", keys[i], json_integer_value(field));

                // Modify the value (e.g., increment by 1)
                json_object_set_new(item, keys[i], json_integer(json_integer_value(field) + 1));
            }
        }
    }

    // Convert the updated JSON back to a string
    char *modified_json = json_dumps(root, JSON_INDENT(2));

    // Clean up
    json_decref(root);

    // Create a result struct
    //JsonResult *result = malloc(sizeof(JsonResult));
    //result->json_result = modified_json; // Memory to be freed by free_json_result

    //return result;
    return modified_json;
}

// Function to free memory allocated in the result
//void free_json_result(JsonResult *result) {
//    if (result) {
//        free((void *)result->json_result); // Free the string
//        free(result);                      // Free the struct
//    }
//}
void free_json_result(char *json_str) {
    if (json_str) {
        //printf("Freeing memory at %p\n", (void *)json_str);
        free(json_str);
    }
}


